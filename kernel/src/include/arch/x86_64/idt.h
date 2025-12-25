#ifndef IDT_H
#define IDT_H

#include <stdint.h>

typedef struct [[gnu::packed]] {
    uint16_t offset1;
    uint16_t ss;
    uint8_t ist;
    uint8_t flags;
    uint16_t offset2;
    uint32_t offset3;
    uint32_t reserved;
} IDTEntry;

typedef struct [[gnu::packed]] {
    uint16_t limit;
    uint64_t offset;
} IDTR;

void x86_64_idt_init();

#endif
