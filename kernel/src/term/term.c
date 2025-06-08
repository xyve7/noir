#include <lib/spinlock.h>
#include <lib/string.h>
#include <term/font.h>
#include <term/term.h>

spinlock term_lock = SPINLOCK_INIT;

void fb_draw(struct limine_framebuffer *fb, size_t x, size_t y, uint32_t color) {
    volatile uint32_t *b = fb->address;
    b[y * (fb->pitch / 4) + x] = color;
}

term_ctx term_new(struct limine_framebuffer *fb) {
    return (term_ctx){
        .fb = fb,
        .cols = fb->width / 8,
        .rows = fb->height / 16,
        .col = 0,
        .row = 0,
        .fg = TERM_WHITE,
        .bg = TERM_BLACK,
        .tab_size = 4,
        .wrapped = false
    };
}
void term_scroll(term_ctx *ctx) {
    void *fb = ctx->fb->address;
    // Gets the numbes of bytes in a line
    size_t offset = ctx->fb->pitch * 16;
    // Gets the remaining of the framebuffer
    size_t remaining = ctx->fb->pitch * (ctx->fb->height - 16);

    // Copies it one line up
    memmove(fb, fb + offset, remaining);
    // Nulls the line
    memset(fb + remaining, ctx->bg, offset);
}
void term_draw_char(term_ctx *ctx, size_t x, size_t y, char ch) {
    for (size_t x1 = 0; x1 < 8; x1++) {
        for (size_t y1 = 0; y1 < 16; y1++) {
            if (INTELV8[(size_t)ch][y1] & (0b10000000 >> x1)) {
                fb_draw(ctx->fb, x + x1, y + y1, ctx->fg);
            } else {
                fb_draw(ctx->fb, x + x1, y + y1, ctx->bg);
            }
        }
    }
}
void term_write_char(term_ctx *ctx, char ch) {
    spinlock_acquire(&term_lock);

    if (ch == '\n') {
        if (!(ctx->col == 0 && ctx->wrapped)) {
            ctx->col = 0;
            ctx->row++;
        }
    } else if (ch == '\t') {
        ctx->col += ctx->tab_size;
        uint8_t rem = ctx->col % ctx->tab_size;
        ctx->col -= rem;
    } else {
        term_draw_char(ctx, ctx->col * 8, ctx->row * 16, ch);
        ctx->col++;
    }
    if (ctx->col == ctx->cols) {
        ctx->col = 0;
        ctx->row++;
        ctx->wrapped = true;
    }
    if (ctx->row == ctx->rows) {
        term_scroll(ctx);
        ctx->row--;
        ctx->col = 0;
    }

    spinlock_release(&term_lock);
}
