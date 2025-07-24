#pragma once

#include <stdint.h>

// State of the CPU which is pushed onto the stack upon an interrupt 
typedef struct {
    // Pushed manually
    uint64_t r15;
    uint64_t r14;
    uint64_t r13;
    uint64_t r12;
    uint64_t r11;
    uint64_t r10;
    uint64_t r9;
    uint64_t r8;
    uint64_t rbp;
    uint64_t rdi;
    uint64_t rsi;
    uint64_t rdx;
    uint64_t rcx;
    uint64_t rbx;
    uint64_t rax;
    // Pushed automatically
    uint64_t no;
    uint64_t err;
    uint64_t rip;
    uint64_t cs;
    uint64_t rflags;
    uint64_t rsp;
    uint64_t ss;
} int_context;

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    asm volatile("inb %w1, %b0"
                 : "=a"(ret)
                 : "Nd"(port)
                 : "memory");
    return ret;
}
static inline void outb(uint16_t port, uint8_t val) {
    asm volatile("outb %b0, %w1" : : "a"(val), "Nd"(port) : "memory");
}

static inline void wrmsr(uint64_t msr, uint64_t value) {
    uint32_t low = value & 0xFFFFFFFF;
    uint32_t high = value >> 32;
    asm volatile(
        "wrmsr"
        :
        : "c"(msr), "a"(low), "d"(high)
    );
}
static inline uint64_t rdmsr(uint64_t msr) {
    uint32_t low, high;
    asm volatile(
        "rdmsr"
        : "=a"(low), "=d"(high)
        : "c"(msr)
    );
    return ((uint64_t)high << 32) | low;
}
static inline void int_disable() {
    asm volatile("cli");
}
static inline void int_enable() {
    asm volatile("sti");
}
static inline void int_set_state(bool state) {
    if (state) {
        int_enable();
    } else {
        int_disable();
    }
}
static inline bool int_get_state() {
    uint64_t flags;
    asm volatile("pushfq\n\t"
                 "pop %0"
                 : "=g"(flags));
    return flags & (1 << 9);
}
