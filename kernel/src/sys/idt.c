#include <kernel.h>
#include <stddef.h>
#include <sys/idt.h>

// IDT struct used to load the IDT
typedef struct [[gnu::packed]] {
    uint16_t limit;
    uint64_t base;
} idtr;

// IDT entry
typedef struct [[gnu::packed]] {
    uint16_t isr0;
    uint16_t selector;
    uint8_t ist;
    uint8_t flags;
    uint16_t isr1;
    uint32_t isr2;
    uint32_t reserved;
} idt_entry;

static idt_entry idt[256];
extern void *isrs[256];

void idt_load() {
    idtr r;
    // This is one less than the size because this value is added to the base
    // to get the last valid byte.
    // Some places, like the OSDEV Wiki, like to refer to this as the "size"
    // but then in fine print be like "Oh it's actually one less than the size".
    // I don't get why they don't call it the limit.
    // This isn't really an issue, just a tiny nitpick.
    // Reference: Intel SDM 3.7.10.
    r.limit = sizeof(idt) - 1;
    r.base = (uint64_t)idt;

    // Load the IDT
    asm("lidt %0" : : "m"(r));
}

void idt_init() {
    // Set every entry
    for (size_t i = 0; i < 256; i++) {
        idt[i].isr0 = (uint64_t)isrs[i] & 0xffff;
        idt[i].selector = 0x08;
        idt[i].ist = 0;
        idt[i].flags = 0x8E; // Interrupt gate
        idt[i].isr1 = ((uint64_t)isrs[i] >> 16) & 0xffff;
        idt[i].isr2 = ((uint64_t)isrs[i] >> 32) & 0xffffffff;
        idt[i].reserved = 0;
    }

    idt_load();
    LOG("IDT Initialized");
}
