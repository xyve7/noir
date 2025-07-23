#include "kernel.h"
#include <cpu/cpu.h>
#include <sys/ps2.h>

#define PS2_DATA 0x60
#define PS2_STATUS 0x64
#define PS2_COMMAND 0x64

uint8_t ps2_read() {
    // Wait until ps2 status is free
    while (!(ps2_status() & 0b1)) {
        asm ("pause");
    }

    // Read data
    return inb(PS2_DATA);
}
void ps2_write(uint8_t byte) {
    // Wait until ps2 status is free
    while (ps2_status() & 0b10) {
        asm ("pause");
    }

    // Read data
    outb(PS2_DATA, byte);
}
uint8_t ps2_status() {
    return inb(PS2_STATUS);
}

void ps2_init() {
    // We read in data, sometimes there is garbage data
    // This causes the interrupt to never fire, since the initial data was never read.
    inb(PS2_DATA);
    
    LOG("PS/2 Initialized");
}
