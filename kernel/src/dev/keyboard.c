#include "kernel.h"
#include "lib/rb.h"
#include "lib/spinlock.h"
#include <dev/keyboard.h>
#include <stdint.h>
#include <sys/irq.h>
#include <sys/ps2.h>

// Chatgpt generated this since im LAZY
// Index = scan code (Set 1), Value = ASCII or 0 for non-printables
const char scancode_set1_ascii[128] = {
    [0x00] = 0,
    [0x01] = 27, // Esc
    [0x02] = '1',
    [0x03] = '2',
    [0x04] = '3',
    [0x05] = '4',
    [0x06] = '5',
    [0x07] = '6',
    [0x08] = '7',
    [0x09] = '8',
    [0x0A] = '9',
    [0x0B] = '0',
    [0x0C] = '-',
    [0x0D] = '=',
    [0x0E] = '\b',
    [0x0F] = '\t', // Backspace, Tab
    [0x10] = 'q',
    [0x11] = 'w',
    [0x12] = 'e',
    [0x13] = 'r',
    [0x14] = 't',
    [0x15] = 'y',
    [0x16] = 'u',
    [0x17] = 'i',
    [0x18] = 'o',
    [0x19] = 'p',
    [0x1A] = '[',
    [0x1B] = ']',
    [0x1C] = '\n', // Enter
    [0x1D] = 0,    // Left Ctrl
    [0x1E] = 'a',
    [0x1F] = 's',
    [0x20] = 'd',
    [0x21] = 'f',
    [0x22] = 'g',
    [0x23] = 'h',
    [0x24] = 'j',
    [0x25] = 'k',
    [0x26] = 'l',
    [0x27] = ';',
    [0x28] = '\'',
    [0x29] = '`',
    [0x2A] = 0, // Left Shift
    [0x2B] = '\\',
    [0x2C] = 'z',
    [0x2D] = 'x',
    [0x2E] = 'c',
    [0x2F] = 'v',
    [0x30] = 'b',
    [0x31] = 'n',
    [0x32] = 'm',
    [0x33] = ',',
    [0x34] = '.',
    [0x35] = '/',
    [0x36] = 0, // Right Shift
    [0x37] = '*',
    [0x38] = 0,   // Alt
    [0x39] = ' ', // Space
    // Remaining keys like F1–F12, CapsLock, etc., not included
};
const char scancode_set1_ascii_shift[128] = {
    [0x02] = '!',
    [0x03] = '@',
    [0x04] = '#',
    [0x05] = '$',
    [0x06] = '%',
    [0x07] = '^',
    [0x08] = '&',
    [0x09] = '*',
    [0x0A] = '(',
    [0x0B] = ')',
    [0x0C] = '_',
    [0x0D] = '+',
    [0x10] = 'Q',
    [0x11] = 'W',
    [0x12] = 'E',
    [0x13] = 'R',
    [0x14] = 'T',
    [0x15] = 'Y',
    [0x16] = 'U',
    [0x17] = 'I',
    [0x18] = 'O',
    [0x19] = 'P',
    [0x1A] = '{',
    [0x1B] = '}',
    [0x1E] = 'A',
    [0x1F] = 'S',
    [0x20] = 'D',
    [0x21] = 'F',
    [0x22] = 'G',
    [0x23] = 'H',
    [0x24] = 'J',
    [0x25] = 'K',
    [0x26] = 'L',
    [0x27] = ':',
    [0x28] = '"',
    [0x29] = '~',
    [0x2B] = '|',
    [0x2C] = 'Z',
    [0x2D] = 'X',
    [0x2E] = 'C',
    [0x2F] = 'V',
    [0x30] = 'B',
    [0x31] = 'N',
    [0x32] = 'M',
    [0x33] = '<',
    [0x34] = '>',
    [0x35] = '?',
};

#define KBD_RELEASE (1 << 7)
#define KBD_CODE_MASK (0b1111111)

rb kbd_buffer;

bool shift = false;
void keyboard_handler(cpu_context *frame) {
    (void)frame;

    // We assume the first scancode set, will fix later.
    // FIXME: Don't assume the first scan code
    uint8_t scan = ps2_read();

    // We first check if it was pressed or release
    if (scan & KBD_RELEASE) {
        // Here we only really care about modifiers
        // since releasing them resets the state
        scan = scan & KBD_CODE_MASK;
        if (scan == 0x2A || scan == 0x2A) {
            shift = false;
        }
    } else {
        // Since it isn't a release, we don't mask
        if (scan == 0x2A || scan == 0x2A) {
            shift = true;
        } else {
            char ch;
            if (shift) {
                ch = scancode_set1_ascii_shift[scan];
            } else {
                ch = scancode_set1_ascii[scan];
            }
            rb_write(&kbd_buffer, (uint8_t)ch);
        }
    }
}
spinlock kbd_lock = SPINLOCK_INIT;
char keyboard_read() {
    while (rb_empty(&kbd_buffer))
        ;
    spinlock_acquire(&kbd_lock);
    char ch = (char)rb_read(&kbd_buffer);
    spinlock_release(&kbd_lock);
    return ch;
}

void keyboard_init() {
    kbd_buffer = rb_new();

    irq_register_handler(33, keyboard_handler);
    LOG("Keyboard Initialized");
}
