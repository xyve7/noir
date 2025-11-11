#ifndef KPRINTF_H
#define KPRINTF_H

#include <stdarg.h>

// Defined elsewhere
extern void write_char(char ch);

// Helper functions for printf
int kvfprintf(void (*write)(char), const char *restrict format, va_list list);
int kvprintf(const char *restrict format, va_list list);

// Formatter print to the framebuffer
int kprintf(const char *restrict format, ...);

#endif
