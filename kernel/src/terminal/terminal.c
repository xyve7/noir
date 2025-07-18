#include <lib/string.h>
#include <stddef.h>
#include <stdint.h>
#include <terminal/font.h>
#include <terminal/terminal.h>

// Colors
#define BLACK 0xFF000000
#define RED 0xFFFF0000
#define GREEN 0xFF00FF00
#define YELLOW 0xFFFFFF00
#define BLUE 0xFF0000FF
#define MAGENTA 0xFFFF00FF
#define CYAN 0xFF00FFFF
#define WHITE 0xFFFFFFFF

#define BRIGHT_BLACK 0xFF555555
#define BRIGHT_RED 0xFFFF5555
#define BRIGHT_GREEN 0xFF55FF55
#define BRIGHT_YELLOW 0xFFFFFF55
#define BRIGHT_BLUE 0xFF5555FF
#define BRIGHT_MAGENTA 0xFFFF55FF
#define BRIGHT_CYAN 0xFF55FFFF
#define BRIGHT_WHITE 0xFFFFFFFF

// Framebuffer data
static volatile uint32_t *restrict framebuffer;
static uint64_t width;
static uint64_t height;
static uint64_t pitch;

// Terminal constants
static const uint64_t char_width = 8;
static const uint64_t char_height = 16;

// Terminal state that doesn't change
static int64_t cols;
static int64_t rows;

// Terminal state
static int64_t col;
static int64_t row;
static uint64_t tab_size = 4;

static uint32_t default_bg = BLACK;
static uint32_t default_fg = WHITE;

static uint32_t bg = BLACK;
static uint32_t fg = WHITE;

static bool wrapped = false;
static bool ansi = false;
static bool csi = false;
static char ansi_buffer[16] = {0};
static uint64_t buffer_i = 0;

// This optimization makes character drawing fast
static inline void draw_char(uint64_t x, uint64_t y, uint8_t ch) {
    size_t index = (size_t)ch;
    const uint8_t *glyph = INTELV8[index];

    size_t pixels = (pitch / 4);
    volatile uint32_t *fb_row = &framebuffer[y * pixels + x];

    for (uint64_t row = 0; row < char_height; row++) {
        volatile uint32_t *line = fb_row;
        uint8_t g = glyph[row];

        *line++ = ((g >> 7) & 1) ? fg : bg;
        *line++ = ((g >> 6) & 1) ? fg : bg;
        *line++ = ((g >> 5) & 1) ? fg : bg;
        *line++ = ((g >> 4) & 1) ? fg : bg;
        *line++ = ((g >> 3) & 1) ? fg : bg;
        *line++ = ((g >> 2) & 1) ? fg : bg;
        *line++ = ((g >> 1) & 1) ? fg : bg;
        *line++ = (g & 1) ? fg : bg;

        fb_row += pixels;
    }
}
static inline void scroll() {
    void *fb = (void *)framebuffer;
    uint64_t bytes_in_row = pitch * char_height;
    uint64_t remaining_bytes = pitch * (char_height * (rows - 1));

    // We move everything below the first row all the way up
    // Essentially, overwriting the first row
    memmove(fb, fb + bytes_in_row, remaining_bytes);

    // Now we have garbage on the bottom row, wipe it out
    uint32_t *last_row = (fb + remaining_bytes);
    size_t pixels_in_row = bytes_in_row >> 2;
    for (size_t i = 0; i < pixels_in_row; i++) {
        last_row[i] = bg;
    }
}
static inline int get_ansi_arg() {
    // Null terminate string
    ansi_buffer[buffer_i] = 0;
    int result = 0;
    for (uint64_t i = 0; i < buffer_i; i++) {
        result = result * 10 + (ansi_buffer[i] - '0');
    }
    buffer_i = 0;
    return result;
}
uint32_t color_table[] = {
    BLACK,
    RED,
    GREEN,
    YELLOW,
    BLUE,
    MAGENTA,
    CYAN,
    WHITE,
};
uint32_t bright_color_table[] = {
    BRIGHT_BLACK,
    BRIGHT_RED,
    BRIGHT_GREEN,
    BRIGHT_YELLOW,
    BRIGHT_BLUE,
    BRIGHT_MAGENTA,
    BRIGHT_CYAN,
    BRIGHT_WHITE
};
static inline void handle_ansi(uint8_t ch) {
    // The CSI byte has been read
    if (csi) {
        switch (ch) {
        case 'H': {
            col = 0;
            row = 0;
            break;
        }
        case 'A': {
            int lines = get_ansi_arg();
            row -= lines;
            if (row < 0)
                row = 0;
            break;
        }
        case 'B': {
            int lines = get_ansi_arg();
            row += lines;
            break;
        }
        case 'C': {
            int cols = get_ansi_arg();
            col += cols;
            break;
        }
        case 'D': {
            int cols = get_ansi_arg();
            col -= cols;
            break;
        }
        case 'm': {
            int color = get_ansi_arg();
            // We get the first number
            int first = color / 10;
            int second = color % 10;

            switch (first) {
            case 3: {
                if (second == 9) {
                    fg = default_fg;
                } else {
                    fg = color_table[second];
                }
                break;
            }
            case 4: {
                if (second == 9) {
                    bg = default_bg;
                } else {
                    bg = color_table[second];
                }
                break;
            }
            case 9: {
                fg = bright_color_table[second];
                break;
            }
            case 10: {
                bg = bright_color_table[second];
                break;
            }
            }
            break;
        }
        default: {
            ansi_buffer[buffer_i] = ch;
            buffer_i++;
            return;
        }
        }
        // We make sure we are still in bounds
        if (row < 0) {
            row = 0;
        }
        if (row >= rows) {
            row = rows - 1;
        }
        if (col < 0) {
            col = 0;
        }
        if (col >= cols) {
            col = cols - 1;
        }

        // Disable CSI
        csi = false;
        ansi = false;
    } else {
        // CSI hasn't been read
        // Read it
        if (ch == '[') {
            csi = true;
        }
    }
}

// Initialized the terminal
void terminal_init(void *fb, uint64_t w, uint64_t h, uint64_t p) {
    framebuffer = fb;
    width = w;
    height = h;
    pitch = p;

    cols = width / char_width;
    rows = height / char_height;

    col = 0;
    row = 0;
}
void terminal_write(uint8_t ch) {
    // We check for ANSI
    if (ansi) {
        handle_ansi(ch);
        goto ensure;
    }

    switch (ch) {
    case 27: {
        // We hit an ANSI escape
        ansi = true;
        break;
    }
    case '\n': {
        // We ignore the newline if:
        // We are at the first column, and we just wrapped
        if (col == 0 && wrapped) {
            goto ensure;
        }
        col = 0;
        row++;
        wrapped = false;
        break;
    }
    case '\t': {
        col += tab_size;
        // Tabs can only be on tab_size divisible columns
        // So we make sure of that
        col -= (col % tab_size);
        break;
    }
    case '\b': {
        // If we are the first character, (0, 0), we ignore
        if (col == 0 && row == 0) {
            goto ensure;
        }
        // We are at the start
        if (col == 0) {
            col = cols;
            row--;
        }
        col--;
        // Draw an empty char
        draw_char(col * char_width, row * char_height, 0);
        break;
    }
    default: {
        // Every other character
        draw_char(col * char_width, row * char_height, ch);
        col++;
        break;
    }
    }

ensure:
    // Last col, go to start
    // Increment row
    // Since this was trigger from reaching the end (not a newline)
    // We set wrapped to true
    if (col == cols) {
        col = 0;
        row++;
        wrapped = true;
    }
    // Last row, move col back to the start
    // and decrement the row so we stay in bounds
    if (row == rows) {
        col = 0;
        row--;
        scroll();
    }
}
