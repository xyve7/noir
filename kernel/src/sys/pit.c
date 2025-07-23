#include "kernel.h"
#include <stdint.h>
#include <sys/pit.h>

// Copied this code because I don't care
// Im just using this to calibrate
//
// Calibrated it to fire every ms
void pit_init() {
    int divisor = 1193180 / 1193;
    outb(0x43, 0b110110);
    outb(0x40, divisor & 0xFF);
    outb(0x40, divisor >> 8);
    LOG("PIT Initialized");
}

volatile uint64_t count = 0;
void pit_handler(cpu_context *frame) {
    (void)frame;
    count++;
}
uint64_t pit_tick() {
    return count;
}
void pit_sleep(uint64_t ms) {
    uint64_t start = count;
    uint64_t until = start + ms;
    while (count < until) {
        asm ("pause");
    }
}
