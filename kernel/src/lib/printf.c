#include <lib/printf.h>
#include <lib/spinlock.h>
#include <stddef.h>
#include <stdint.h>

// Length modifiers
#define LEN_MOD_HH 1
#define LEN_MOD_H 2
#define LEN_MOD_L 3

// Different kinds of digits for both capital and lowercase hex
const char digits_low[] = "0123456789abcdef";
const char digits_upper[] = "0123456789ABCDEF";

spinlock printf_lock = SPINLOCK_INIT;

// Converts a long into a string
// This uses a long, so smaller types can be safely casted
// The other option is writing 4 different functions, which is silly
char *ltos(long val, int radix, const char *digits) {
    static char buffer[66];
    bool n = (val < 0);
    char *s = buffer + sizeof(buffer) - 1;
    *s = '\0';
    do {
        long r = (val % radix);
        *(--s) = digits[n ? -r : r];
        val /= radix;
    } while (val);
    if (n) {
        *(--s) = '-';
    }
    return s;
}
// Same as above, but unsigned
char *ultos(unsigned long val, int radix, const char *digits) {
    static char buffer[66];
    char *s = buffer + sizeof(buffer) - 1;
    *s = '\0';
    do {
        unsigned long r = (val % radix);
        *(--s) = digits[r];
        val /= radix;
    } while (val);
    return s;
}
// Write the string based on the char writing function
int write_string(void (*write)(char), const char *s) {
    int written = 0;
    if (s) {
        while (*s) {
            write(*s++);
            written++;
        }
    } else {
        write_string(write, "(null)");
    }
    return written;
}
// Get the next string argument
char *next_int(uint8_t len_mod, uint8_t radix, const char *digits, va_list list) {
    // Get the correct argument, according to the length modifier
    switch (len_mod) {
    case LEN_MOD_HH:
        return ltos((char)va_arg(list, int), radix, digits);
    case LEN_MOD_H:
        return ltos((short)va_arg(list, int), radix, digits);
    case LEN_MOD_L:
        return ltos(va_arg(list, long), radix, digits);
    default:
        return ltos(va_arg(list, int), radix, digits);
    }
}
char *next_uint(uint8_t len_mod, uint8_t radix, const char *digits, va_list list) {
    // Get the correct argument, according to the length modifier
    switch (len_mod) {
    case LEN_MOD_HH:
        return ultos((unsigned char)va_arg(list, unsigned int), radix, digits);
    case LEN_MOD_H:
        return ultos((unsigned short)va_arg(list, unsigned int), radix, digits);
    case LEN_MOD_L:
        return ultos(va_arg(list, unsigned long), radix, digits);
    default:
        return ultos(va_arg(list, unsigned int), radix, digits);
    }
}

int vfprintf(void (*write)(char), const char *restrict format, va_list list) {
    spinlock_acquire(&printf_lock);

    // Flag characters
    int written = 0;
    uint8_t len_mod = 0;
    while (*format) {
        // Check if the element is a %
        if (*format == '%') {
            format++;
            // Check length modifier
            switch (*format) {
            case 'h':
                format++;
                if (*format == 'h') {
                    len_mod = LEN_MOD_HH;
                    format++;
                } else {
                    len_mod = LEN_MOD_H;
                }
                break;
            case 'l':
                len_mod = LEN_MOD_L;
                format++;
                break;
            default:
                break;
            }

            // Specifier
            switch (*format++) {
            case 'd':
            case 'i':
                written += write_string(write, next_int(len_mod, 10, digits_low, list));
                break;
            case 'u':
                written += write_string(write, next_uint(len_mod, 10, digits_low, list));
                break;
            case 'o':
                written += write_string(write, next_uint(len_mod, 8, digits_low, list));
                break;
            case 'x':
                written += write_string(write, next_uint(len_mod, 16, digits_low, list));
                break;
            case 'X':
                written += write_string(write, next_uint(len_mod, 16, digits_upper, list));
                break;
            case 'b':
                written += write_string(write, next_uint(len_mod, 2, digits_upper, list));
                break;
            case 'p':
                written += write_string(write, "0x");
                written += write_string(write, ultos(va_arg(list, uintptr_t), 16, digits_low));
                break;
            case '%':
                write('%');
                written++;
                break;
            case 'c':
                write((char)va_arg(list, int));
                written++;
                break;
            case 's':
                written += write_string(write, va_arg(list, const char *));
                break;
            }
        } else {
            write(*format++);
            written++;
        }
    }

    spinlock_release(&printf_lock);
    return written;
}
int vprintf(const char *restrict format, va_list list) {
    return vfprintf(write_char, format, list);
}
int printf(const char *restrict format, ...) {
    va_list list;
    va_start(list, format);
    int ret = vprintf(format, list);
    va_end(list);
    return ret;
}
