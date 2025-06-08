#pragma once

#include <stdint.h>
typedef struct [[gnu::packed]] {
    uint16_t limit;
    uint64_t base;
} idt_r;

typedef struct [[gnu::packed]] {
    uint16_t isr0;
    uint16_t sel;
    uint8_t ist;
    uint8_t flags;
    uint16_t isr1;
    uint32_t isr2;
    uint32_t resv;
} idt_ent;

void idt_load();
void idt_init();
