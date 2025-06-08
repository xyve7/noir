#include <cpu/cpu.h>
#include <kernel.h>
#include <lib/spinlock.h>
#include <lib/vec.h>
#include <mm/pmm.h>
#include <mm/vmm.h>
#include <stdint.h>
#include <sys/lapic.h>
#include <sys/madt.h>

#define IA32_APIC_BASE 0x1B

#define ID 0x020
#define SVR 0x0F0
#define EOI 0x0B0
#define TIMER 0x320
#define LINT0 0x350
#define LINT1 0x360
#define TIMER_INITCNT 0x380
#define TIMER_DIV 0x3E0

#define SMI 0x200
#define NMI 0x400

#define PERIODIC 0x20000

// Needs to be volatile, since it changes so
// frequently.
volatile void *lapic;

spinlock lapic_lock = SPINLOCK_INIT;

// The registers are 32 bit
// Aligned at 128 bit boundaries
void lapic_write(volatile void *address, uint16_t reg, uint32_t value) {
    spinlock_acquire(&lapic_lock);
    *((volatile uint32_t *)(address + reg)) = value;
    spinlock_release(&lapic_lock);
}
uint32_t lapic_read(volatile void *address, uint16_t reg) {
    spinlock_acquire(&lapic_lock);
    uint32_t v = *((volatile uint32_t *)(address + reg));
    spinlock_release(&lapic_lock);
    return v;
}
void lapic_eoi() {
    // Send the eoi
    lapic_write(lapic, EOI, 0x0);
}
uint32_t lapic_id() {
    return lapic_read(lapic, ID) >> 24;
}
void lapic_core() {
    // We map the NMI to, but we have to check which one it is
    // Generally its LINT1
    for (size_t i = 0; i < lapic_nmis.len; i++) {
        madt_lapic_nmi *lapic_nmi = vec_at(&lapic_nmis, i);
        // We check if the current lapic id is the same
        // as the proc_id for the lapic_nmi

        // If the proc_id is 0xFF, that means all processors
        // If it isn't we then check what the value is
        // and check if the current lapic_id matches
        if (lapic_nmi->proc_id == 0xFF || lapic_id() == lapic_nmi->proc_id) {
            switch (lapic_nmi->lint) {
            case 0:
                lapic_write(lapic, LINT0, NMI | 0x2);
                break;
            case 1:
                lapic_write(lapic, LINT1, NMI | 0x2);
                break;
            }
        }
    }

    // We enable the lapic
    lapic_write(lapic, SVR, 0x100 | 0xFF);
    // We setup the timer vector
    lapic_write(lapic, TIMER, PERIODIC | 32);
    // Set the timer
    lapic_write(lapic, TIMER_INITCNT, 10000000);
    lapic_write(lapic, TIMER_DIV, 16);
}
void lapic_init() {
    // We get the processors lapic and mask the address out
    void *lapic_phys = (void *)(rdmsr(IA32_APIC_BASE) & 0xfffff000);
    void *lapic_virt = VIRT(lapic_phys);

    // We load the LAPIC, and write the enable bit
    // This is to turn on the LAPIC for the processor
    // We map it
    vmm_map(&kernel_pm, (uintptr_t)lapic_phys, (uintptr_t)lapic_virt, VMM_PRESENT | VMM_WRITE);
    lapic = lapic_virt;

    lapic_core();
}
