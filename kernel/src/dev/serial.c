#include <cpu/cpu.h>
#include <dev/serial.h>
#include <kernel.h>
#include <lib/spinlock.h>
#include <stdint.h>

// Serial port address
#define COM1 0x3F8
// Line control register offset
#define COM_LINE_CONTROL_REGISTER 3
// Line status register offset
#define COM_LINE_STATUS_REGISTER 5
// Mask the line status register for read status
// Also known as DR  
#define COM_READ (1 << 0)
// Mask the line status register for write status
// Also known as THRE
#define COM_WRITE (1 << 5)

static inline uint8_t serial_status() {
    return inb(COM1 + COM_LINE_STATUS_REGISTER);
}
void serial_write(uint8_t byte) {
    // Loop until the data is ready to be read
    while (!(serial_status() & COM_READ)) {
        asm("pause");
    }
    outb(COM1, byte);
}
uint8_t serial_read() {
    // Loop until the data is ready to be sent
    while (!(serial_status() & COM_WRITE)) {
        asm("pause");
    }
    return inb(COM1);
}
void serial_write_ch(char ch) {
    serial_write((uint8_t)ch);
}
int serial_vprintf(const char *restrict format, va_list list) {
    return vfprintf(&serial_write_ch, format, list);
}

spinlock serial_lock = SPINLOCK_INIT;
int serial_printf(const char *restrict format, ...) {
    va_list list;
    va_start(list, format);

    bool state = spinlock_acquire_irq_save(&serial_lock);

    int ret = serial_vprintf(format, list);

    spinlock_release_irq_restore(&serial_lock, state);

    va_end(list);
    return ret;
}
void serial_init() {
    // No stop or data bits 
    // Full 8 bit characters
    outb(COM1 + COM_LINE_CONTROL_REGISTER, 0b11);
    LOG("Serial Initialized");
}