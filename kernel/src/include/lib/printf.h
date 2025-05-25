#pragma once

#include <stdarg.h>
#include <term/term.h>

// terminal context
extern term_ctx term;
// this is defined elsewhere,
extern void write_char(char ch);

// helper print function
int vfprintf(void (*write)(char), const char *restrict format, va_list list);
int vprintf(const char *restrict format, va_list list);

// main function
int printf(const char *restrict format, ...);
