#include <arch/x86_64/idt.h>
#include <kernel.h>
#include <stddef.h>
#include <stdint.h>

extern void *x86_64_isrs[256];

[[gnu::aligned(0x10)]] IDTEntry idt[256];

void x86_64_idt_init() {
    // We set every entry
    for (size_t i = 0; i < 256; i++) {
        uint64_t address = (uint64_t)x86_64_isrs[i];
        idt[i] = (IDTEntry){
            .offset1 = (address & 0xFFFF),
            .ss = 0x28,
            .ist = 0,
            .flags = 0x8E,
            .offset2 = (address >> 16) & 0xFFFF,
            .offset3 = (address >> 32) & 0xFFFFFFFF,
            .reserved = 0
        };
    }

    // Load the IDT
    IDTR idtr = {
        .limit = sizeof(idt) - 1,
        .offset = (uint64_t)idt
    };

    asm("lidt %0" : : "m"(idtr));
    LOG_INFO("Interrupts Initialized");
}
