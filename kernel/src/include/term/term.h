#pragma once

#include <limine.h>
#include <stddef.h>
#include <stdint.h>

#define TERM_WHITE 0xFFFFFFFF
#define TERM_BLACK 0x00000000

typedef struct {
    struct limine_framebuffer *fb;
    // Obtained via the framebuffer size
    size_t cols, rows;
    // Current index
    size_t col, row;
    // Current colors
    uint32_t fg, bg;
    // Tab size
    uint8_t tab_size;
    // If text wrap had just occurred
    bool wrapped;
} term_ctx;

term_ctx term_new(struct limine_framebuffer *fb);
void term_set_color(term_ctx *ctx, uint32_t fg, uint32_t bg);
void term_write_char(term_ctx *ctx, char ch);
