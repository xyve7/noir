#pragma once

#include <stdint.h>

// IDT struct used to load the IDT
typedef struct [[gnu::packed]] {
    uint16_t limit;
    uint64_t base;
} idtr;

// IDT entry
typedef struct [[gnu::packed]] {
    uint16_t isr0;
    uint16_t sel;
    uint8_t ist;
    uint8_t flags;
    uint16_t isr1;
    uint32_t isr2;
    uint32_t resv;
} idt_ent;

// Load the IDT
void idt_load();
// Create the entries
void idt_init();
