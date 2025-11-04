#ifndef STDIO_H
#define STDIO_H

#include <kernel/noir.h>
#include <stdarg.h>

typedef struct {
    FD fd;
} FILE;

#define SEEK_SET -1
#define SEEK_CUR 0
#define SEEK_END 1

#define NULL ((void *)0)

int printf(const char *restrict, ...);
int vprintf(const char *restrict, va_list args);
int fprintf(FILE *restrict, const char *restrict, va_list args);
int vfprintf(FILE *restrict, const char *restrict, va_list args);

#endif
