#ifndef GDT_H
#define GDT_H

#include <stdint.h>

typedef struct [[gnu::packed]] {
    uint16_t limit1;
    uint16_t base1;
    uint8_t base2;
    uint8_t access;
    uint8_t limit2_flags;
    uint8_t base3;
} GDTEntry;

typedef struct [[gnu::packed]] {
    uint16_t limit;
    uint64_t offset;
} GDTR;

void x86_64_gdt_init();

#endif
