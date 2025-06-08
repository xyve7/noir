#include <cpu/cpu.h>
#include <dev/serial.h>
#include <kernel.h>
#include <lib/spinlock.h>
#include <stdint.h>

#define COM_LCR 3
#define COM_LSR 5

#define COM_DR 0b1
#define COM_THRE 0b100000

spinlock serial_lock = SPINLOCK_INIT;

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
    spinlock_acquire(&serial_lock);

    // Loop until the data is ready to be read
    while (!(serial_status() & COM_THRE))
        ;
    outb(COM1, byte);

    spinlock_release(&serial_lock);
}
uint8_t serial_read() {
    spinlock_acquire(&serial_lock);

    // Loop until the data is ready to be sent
    while (!(serial_status() & COM_DR))
        ;
    return inb(COM1);

    spinlock_release(&serial_lock);
}
void serial_write_ch(char ch) {
    serial_write((uint8_t)ch);
}
int serial_vprintf(const char *restrict format, va_list list) {
    int ret = vfprintf(&serial_write_ch, format, list);
    return ret;
}
int serial_printf(const char *restrict format, ...) {
    va_list list;
    va_start(list, format);
    int ret = serial_vprintf(format, list);
    va_end(list);
    return ret;
}
