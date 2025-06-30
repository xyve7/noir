#pragma once

#include <stdarg.h>

// Defined elsewhere
extern void write_char(char ch);

// Helper functions for printf
int vfprintf(void (*write)(char), const char *restrict format, va_list list);
int vprintf(const char *restrict format, va_list list);

// Formatter print to the framebuffer
int printf(const char *restrict format, ...);
