#pragma once

#include <lib/printf.h>
#include <stdint.h>

#define COM1 0x3F8

// Initialize serial
void serial_init();

// Write a byte to the serial port
void serial_write(uint8_t byte);
// Read a byte to the serial port
uint8_t serial_read();

// Formatted printing
int serial_vprintf(const char *restrict format, va_list list);
int serial_printf(const char *restrict format, ...);
