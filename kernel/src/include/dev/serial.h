#pragma once

#include <lib/printf.h>
#include <stdint.h>

#define COM1 0x3F8

// initialized COMq
void serial_init();

void serial_write(uint8_t byte);
uint8_t serial_read();

void serial_write_string(char *s);

// used for formated printing
int serial_vprintf(const char *restrict format, va_list list);
int serial_printf(const char *restrict format, ...);
