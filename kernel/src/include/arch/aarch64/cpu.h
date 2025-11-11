#ifndef CPU_H
#define CPU_H

#include <stdint.h>
typedef struct {
    uint64_t x0, x1;
    uint64_t x2, x3;
    uint64_t x4, x5;
    uint64_t x6, x7;
    uint64_t x8, x9;
    uint64_t x10, x11;
    uint64_t x12, x13;
    uint64_t x14, x15;
    uint64_t x16, x17;
    uint64_t x18, x19;
    uint64_t x20, x21;
    uint64_t x22, x23;
    uint64_t x24, x25;
    uint64_t x26, x27;
    uint64_t x28, x29;
    uint64_t x30;
} AARCH64State;

static inline uint64_t aarch64_current_el_read() {
    uint64_t ret;
    asm volatile("mrs %0, CurrentEL" : "=r"(ret));
    return ret;
}
static inline uint64_t aarch64_sp_el1_read() {
    uint64_t ret;
    asm volatile("mrs %0, SP_EL1" : "=r"(ret));
    return ret;
}
static inline uint64_t aarch64_sp_el0_read() {
    uint64_t ret;
    asm volatile("mrs %0, SP_EL0" : "=r"(ret));
    return ret;
}
static inline void aarch64_spsel_write(uint64_t value) {
    asm volatile("msr SPSEL, %0" ::"r"(value) : "memory");
}

#endif
