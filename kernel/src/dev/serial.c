#include "kernel.h"
#include <cpu/cpu.h>
#include <dev/serial.h>
#include <stdint.h>

#define COM_LCR 3
#define COM_LSR 5

#define COM_DR 0b1
#define COM_THRE 0b100000

void serial_init() {
    // Set the line control to
    // DLAB: off
    // Stop Bits: 1
    // Parity Bits: NONE
    outb(COM1 + COM_LCR, 0b11);
    LOG("serial init\n");
}
uint8_t serial_status() {
    return inb(COM1 + COM_LSR);
}
void serial_write(uint8_t byte) {
    // Loop until the data is ready to be read
    while (!(serial_status() & COM_THRE))
        ;
    outb(COM1, byte);
}
uint8_t serial_read() {
    // Loop until the data is ready to be sent
    while (!(serial_status() & COM_DR))
        ;
    return inb(COM1);
}
void serial_write_string(char *s) {
    while (*s) {
        serial_write((uint8_t)*s);
        s++;
    }
}
void serial_write_ch(char ch) {
    serial_write((uint8_t)ch);
}
int serial_vprintf(const char *restrict format, va_list list) {
    return vfprintf(&serial_write_ch, format, list);
}
int serial_printf(const char *restrict format, ...) {
    va_list list;
    va_start(list, format);
    int ret = serial_vprintf(format, list);
    va_end(list);
    return ret;
}
