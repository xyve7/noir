#include <cpu/cpu.h>
#include <lib/spinlock.h>
#include <lib/string.h>
#include <lib/vec.h>
#include <limine.h>
#include <mm/heap.h>
#include <mm/vmm.h>
#include <sys/idt.h>
#include <sys/lapic.h>
#include <sys/smp.h>

#define MAX_CPUS 64

__attribute__((used, section(".limine_requests"))) static volatile struct limine_mp_request mp_request = {
    .id = LIMINE_MP_REQUEST,
    .revision = 0
};

static cpu cpus[MAX_CPUS];
static size_t cpu_count;

spinlock cpu_lock = SPINLOCK_INIT;
cpu *cpu_get() {
    return &cpus[lapic_id()];
}
void cpu_set(cpu *c) {
    bool state = int_get_state();
    int_disable();
    spinlock_acquire(&cpu_lock);
    cpus[lapic_id()] = *c;
    spinlock_release(&cpu_lock);
    int_set_state(state);
}

void cpu_start(struct limine_mp_info *info) {
    idt_load();
    vmm_switch(&kernel_pm);
    lapic_enable();

    LOG("CPU Online: cpu=%u", info->lapic_id);

    while (true) {
        asm("sti");
    }
}

void smp_init() {
    // Initialize the vector based on the cpu count
    cpu_count = mp_request.response->cpu_count;

    // Add all the cpus
    for (size_t i = 0; i < cpu_count; i++) {
        if (i == mp_request.response->bsp_lapic_id) {
            continue;
        }
        mp_request.response->cpus[i]->goto_address = cpu_start;
    }

    LOG("SMP Initalized");
}
