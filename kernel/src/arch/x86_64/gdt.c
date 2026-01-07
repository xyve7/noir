#include <arch/x86_64/gdt.h>
#include <kernel.h>
#include <stddef.h>
#include <stdint.h>

GDTEntry gdt[5] = {
    {0,      0, 0, 0,    0,    0},
    {0xFFFF, 0, 0, 0x9A, 0xAF, 0},
    {0xFFFF, 0, 0, 0x92, 0xCF, 0},
    {0xFFFF, 0, 0, 0xF2, 0xCF, 0},
    {0xFFFF, 0, 0, 0xFA, 0xAF, 0},
};

void x86_64_gdt_init() {
    // Load the GDT
    GDTR gdtr = {
        .limit = sizeof(gdt) - 1,
        .offset = (uint64_t)gdt
    };

    asm("lgdt %0" : : "m"(gdtr));
    LOG_INFO("GDT Initialized");
}
