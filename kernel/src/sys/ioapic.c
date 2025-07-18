#include "kernel.h"
#include "mm/vmm.h"
#include <lib/vec.h>
#include <mm/pmm.h>
#include <stdint.h>
#include <sys/ioapic.h>
#include <sys/madt.h>

#define IOREDTBL(n) (0x10 + n * 2)

volatile void *ioapic = nullptr;

void ioapic_write(volatile void *address, uint8_t index, uint32_t value) {
    // We set the IOREGSEL
    *((volatile uint8_t *)address) = index;
    // Now, we write the actual data
    *((volatile uint32_t *)(address + 0x10)) = value;
}
uint32_t ioapic_read(volatile void *address, uint8_t index) {
    // We set the IOREGSEL
    *((volatile uint8_t *)address) = index;
    // Now, we read the actual data
    return *((volatile uint32_t *)(address + 0x10));
}
void ioapic_init() {
    madt_ioapic *i = vec_at(&ioapics, 0);

    void *ioapic_phys = (void *)(uint64_t)i->address;
    void *ioapic_virt = VIRT(ioapic_phys);

    vmm_map(&kernel_pagemap, (uintptr_t)ioapic_phys, (uintptr_t)ioapic_virt, VMM_PRESENT | VMM_WRITE);
    ioapic = ioapic_virt;

    // Setup the interrupt
    ioapic_write(ioapic, IOREDTBL(1), 33);
    ioapic_write(ioapic, IOREDTBL(2), 34);

    LOG("IOAPIC Initalized");
}
