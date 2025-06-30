#include "cpu/cpu.h"
#include <kernel.h>
#include <lib/spinlock.h>
#include <mm/vmm.h>

__attribute__((used, section(".limine_requests"))) volatile struct limine_executable_address_request kexaddr_request = {
    .id = LIMINE_EXECUTABLE_ADDRESS_REQUEST,
    .revision = 0
};

extern uintptr_t KERNEL_TEXT_START, KERNEL_TEXT_END;
extern uintptr_t KERNEL_RODATA_START, KERNEL_RODATA_END;
extern uintptr_t KERNEL_DATA_START, KERNEL_DATA_END;

spinlock vmm_lock = SPINLOCK_INIT;

pagemap kernel_pm;
void vmm_init() {
    kernel_pm = (pagemap)pmm_allocz(1);

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
        vmm_map(&kernel_pm, i - virt + phys, i, VMM_PRESENT | VMM_XD | VMM_WRITE);
    }
    for (uintptr_t i = kernel_rodata_start; i < kernel_rodata_end; i += PAGE_SIZE) {
        vmm_map(&kernel_pm, i - virt + phys, i, VMM_PRESENT | VMM_XD);
    }
    for (uintptr_t i = kernel_text_start; i < kernel_text_end; i += PAGE_SIZE) {
        vmm_map(&kernel_pm, i - virt + phys, i, VMM_PRESENT);
    }

    // Since I modified the limine entry of the area the bitmap was allocated
    // I have to map it manually

    uint8_t *bitmap = pmm_bitmap();
    uint64_t bitmap_size = pmm_bitmap_size();
    for (uint64_t i = 0; i < bitmap_size; i += PAGE_SIZE) {
        uintptr_t addr = (uintptr_t)(bitmap + i);
        vmm_map(&kernel_pm, (uintptr_t)PHYS(addr), addr, VMM_PRESENT | VMM_WRITE);
    }

    struct limine_memmap_response *memmap = memmap_request.response;
    for (uint64_t i = 0; i < memmap->entry_count; i++) {
        struct limine_memmap_entry *entry = memmap->entries[i];
        for (uint64_t j = 0; j < entry->length; j += PAGE_SIZE) {
            uintptr_t page = entry->base + j;
            vmm_map(&kernel_pm, page, (uintptr_t)VIRT(page), VMM_PRESENT | VMM_WRITE);
        }
    }
    vmm_switch(&kernel_pm);
    LOG("VMM Initalized");
}
pagemap vmm_pagemap_new() {
    uintptr_t *kpm = (uintptr_t *)VIRT(kernel_pm);
    uintptr_t *pm = VIRT(pmm_allocz(1));
    for (size_t i = 256; i < 512; i++) {
        pm[i] = kpm[i];
    }
    return (pagemap)PHYS(pm);
}
void vmm_switch(pagemap *pm) {
    asm("mov %0, %%cr3" ::"r"(*pm));
}
void vmm_map(pagemap *pm, uintptr_t phys, uintptr_t virt, uint64_t flags) {
    bool state = int_get_state();
    int_disable();
    spinlock_acquire(&vmm_lock);

    // Get the offset of the levels
    uintptr_t pml1 = PML1(virt);
    uintptr_t pml2 = PML2(virt);
    uintptr_t pml3 = PML3(virt);
    uintptr_t pml4 = PML4(virt);

    // Top privlages
    // Higher pages determine the permissions of the region beneath them
    // Use something that can be fine tuned
    uint64_t higher_flags = VMM_PRESENT | VMM_WRITE;

    // Pointer to the pml4
    uintptr_t *p = (uintptr_t *)VIRT(*pm);
    if (!(p[pml4] & VMM_PRESENT)) {
        p[pml4] = (uintptr_t)pmm_allocz(1) | higher_flags;
    }

    p = (uintptr_t *)VIRT(PHYSADDR(p[pml4]));
    if (!(p[pml3] & VMM_PRESENT)) {
        p[pml3] = (uintptr_t)pmm_allocz(1) | higher_flags;
    }

    p = (uintptr_t *)VIRT(PHYSADDR(p[pml3]));
    if (!(p[pml2] & VMM_PRESENT)) {
        p[pml2] = (uintptr_t)pmm_allocz(1) | higher_flags;
    }

    p = (uintptr_t *)VIRT(PHYSADDR(p[pml2]));
    p[pml1] = phys | flags;

    spinlock_release(&vmm_lock);
    int_set_state(state);
}
void vmm_unmap(pagemap *pm, uintptr_t virt) {
    bool state = int_get_state();
    int_disable();
    spinlock_acquire(&vmm_lock);

    // Get the offset of the levels
    uintptr_t pml1 = PML1(virt);
    uintptr_t pml2 = PML2(virt);
    uintptr_t pml3 = PML3(virt);
    uintptr_t pml4 = PML4(virt);

    // Pointer to the pml4
    uintptr_t *p = (uintptr_t *)VIRT(*pm);
    if (!(p[pml4] & VMM_PRESENT)) {
        PANIC("unable to unmap page\n");
    }

    p = (uintptr_t *)VIRT(PHYSADDR(p[pml4]));
    if (!(p[pml3] & VMM_PRESENT)) {
        PANIC("unable to unmap page\n");
    }

    p = (uintptr_t *)VIRT(PHYSADDR(p[pml3]));
    if (!(p[pml2] & VMM_PRESENT)) {
        PANIC("unable to unmap page\n");
    }

    p = (uintptr_t *)VIRT(PHYSADDR(p[pml2]));
    p[pml1] = 0;

    spinlock_release(&vmm_lock);
    int_set_state(state);
}
