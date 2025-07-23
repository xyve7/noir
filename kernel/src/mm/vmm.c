#include <cpu/cpu.h>
#include <kernel.h>
#include <lib/spinlock.h>
#include <lib/string.h>
#include <mm/vmm.h>
#include <stdint.h>


// Macros to help get the different level indexes
#define PML1(virt) ((virt >> 12) & 0x1ff)
#define PML2(virt) ((virt >> 21) & 0x1ff)
#define PML3(virt) ((virt >> 30) & 0x1ff)
#define PML4(virt) ((virt >> 39) & 0x1ff)

#define PHYSADDR(virt) (virt & 0x0007fffffffff000)

__attribute__((used, section(".limine_requests"))) volatile struct limine_executable_address_request kexaddr_request = {
    .id = LIMINE_EXECUTABLE_ADDRESS_REQUEST,
    .revision = 0
};

extern uintptr_t KERNEL_TEXT_START, KERNEL_TEXT_END;
extern uintptr_t KERNEL_RODATA_START, KERNEL_RODATA_END;
extern uintptr_t KERNEL_DATA_START, KERNEL_DATA_END;

// Default flags for the directories and tables
// Attempt to be the most permissive
const uint64_t default_flags = VMM_PRESENT | VMM_WRITE | VMM_USER;

spinlock vmm_lock = SPINLOCK_INIT;
page_table kernel_page_table;

void vmm_init() {
    kernel_page_table = (page_table)pmm_alloc_zeroed(1);

    uintptr_t kernel_text_start, kernel_text_end;
    uintptr_t kernel_rodata_start, kernel_rodata_end;
    uintptr_t kernel_data_start, kernel_data_end;

    kernel_text_start = (uintptr_t)&KERNEL_TEXT_START;
    kernel_text_end = ROUND((uintptr_t)&KERNEL_TEXT_END, PAGE_SIZE);
    kernel_rodata_start = (uintptr_t)&KERNEL_RODATA_START;
    kernel_rodata_end = ROUND((uintptr_t)&KERNEL_RODATA_END, PAGE_SIZE);
    kernel_data_start = (uintptr_t)&KERNEL_DATA_START;
    kernel_data_end = ROUND((uintptr_t)&KERNEL_DATA_END, PAGE_SIZE);

    uintptr_t phys = kexaddr_request.response->physical_base;
    uintptr_t virt = kexaddr_request.response->virtual_base;

    for (uintptr_t i = kernel_data_start; i < kernel_data_end; i += PAGE_SIZE) {
        vmm_map(&kernel_page_table, i - virt + phys, i, VMM_PRESENT | VMM_XD | VMM_WRITE);
    }
    for (uintptr_t i = kernel_rodata_start; i < kernel_rodata_end; i += PAGE_SIZE) {
        vmm_map(&kernel_page_table, i - virt + phys, i, VMM_PRESENT | VMM_XD);
    }
    for (uintptr_t i = kernel_text_start; i < kernel_text_end; i += PAGE_SIZE) {
        vmm_map(&kernel_page_table, i - virt + phys, i, VMM_PRESENT);
    }

    // Since I modified the limine entry of the area the bitmap was allocated
    // I have to map it manually
    pmm_map(&kernel_page_table);

    struct limine_memmap_response *memmap = memmap_request.response;
    for (uint64_t i = 0; i < memmap->entry_count; i++) {
        struct limine_memmap_entry *entry = memmap->entries[i];
        for (uint64_t j = 0; j < entry->length; j += PAGE_SIZE) {
            uintptr_t page = entry->base + j;
            vmm_map(&kernel_page_table, page, (uintptr_t)VIRT(page), VMM_PRESENT | VMM_WRITE);
        }
    }
    vmm_switch(&kernel_page_table);
    LOG("VMM Initalized");
}
page_table vmm_new() {
    uintptr_t *kernel_pt = (uintptr_t *)VIRT(kernel_page_table);
    uintptr_t *new_pt = VIRT(pmm_alloc_zeroed(1));

    // We make sure to copy the higher half
    memcpy(new_pt + 256, kernel_pt + 256, 256 * sizeof(uintptr_t));
    return (page_table)PHYS(new_pt);
}
void vmm_switch(page_table *pt) {
    asm("mov %0, %%cr3" ::"r"(*pt));
}
// The allocate parameter is set when we unmap a page
// We don't have a condition where this shouldn't fail
// So we just panic for now
uintptr_t *next_level(uintptr_t *prev, uintptr_t pml, bool allocate) {
    if (!(prev[pml] & VMM_PRESENT)) {
        // There is no entry, and we aren't supposed to allocate
        // So we panic
        if (!allocate) {
            PANIC("Unable to obtain next PML");
        }

        prev[pml] = (uintptr_t)pmm_alloc_zeroed(1) | default_flags;
    }
    return (uintptr_t *)VIRT(PHYSADDR(prev[pml]));
}
void vmm_map(page_table *pt, uintptr_t phys, uintptr_t virt, uint64_t flags) {
    bool state = int_get_state();
    int_disable();
    spinlock_acquire(&vmm_lock);

    uintptr_t pml1 = PML1(virt);
    uintptr_t pml2 = PML2(virt);
    uintptr_t pml3 = PML3(virt);
    uintptr_t pml4 = PML4(virt);

    uintptr_t *p = (uintptr_t *)VIRT(*pt);

    p = next_level(p, pml4, true);
    p = next_level(p, pml3, true);
    p = next_level(p, pml2, true);

    p[pml1] = phys | flags;

    spinlock_release(&vmm_lock);
    int_set_state(state);
}
void vmm_unmap(page_table *pt, uintptr_t virt) {
    bool state = int_get_state();
    int_disable();
    spinlock_acquire(&vmm_lock);

    uintptr_t pml1 = PML1(virt);
    uintptr_t pml2 = PML2(virt);
    uintptr_t pml3 = PML3(virt);
    uintptr_t pml4 = PML4(virt);

    uintptr_t *p = (uintptr_t *)VIRT(*pt);

    p = next_level(p, pml4, false);
    p = next_level(p, pml3, false);
    p = next_level(p, pml2, false);

    p[pml1] = 0;

    spinlock_release(&vmm_lock);
    int_set_state(state);
}
