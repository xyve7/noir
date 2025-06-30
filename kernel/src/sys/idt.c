#include "kernel.h"
#include <stddef.h>
#include <sys/idt.h>

static idt_ent idt[256];
extern void *isrs[256];

void idt_load() {
    idt_r idtr;
    // This is one less than the size because this value is added to the base
    // to get the last valid byte.
    // Some places, like the OSDEV Wiki, like to refer to this as the "size"
    // but then in fine print be like "Oh it's actually one less than the size".
    // I don't get why they don't call it the limit.
    // This isn't really an issue, just a tiny nitpick.
    // Reference: Intel SDM 3.7.10.
    idtr.limit = sizeof(idt) - 1;
    idtr.base = (uint64_t)idt;

    // Load the IDT
    asm("lidt %0" : : "m"(idtr));
}

void idt_init() {
    // Set every entry
    for (size_t i = 0; i < 256; i++) {
        idt[i].isr0 = (uint64_t)isrs[i] & 0xffff;
        idt[i].sel = 0x28;
        idt[i].ist = 0;      // I haven't set up a TSS yet.
        idt[i].flags = 0x8E; // Interrupt gate
        idt[i].isr1 = ((uint64_t)isrs[i] >> 16) & 0xffff;
        idt[i].isr2 = ((uint64_t)isrs[i] >> 32) & 0xffffffff;
        idt[i].resv = 0;
    }

    // We want syscalls to be interruptable
    idt[0x80].flags = 0x8F;
    // I set the entries as an interrupt gate not a trap gate.
    // This is because trap gates don't disable interrupts when
    // the handler is called.

    idt_load();

    LOG("IDT Initialized");
}
