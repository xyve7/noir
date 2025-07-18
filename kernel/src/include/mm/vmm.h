#pragma once

#include <mm/pmm.h>

typedef uintptr_t pagemap;
extern pagemap kernel_pagemap;

#define VMM_PRESENT (1ul << 0)
#define VMM_WRITE (1ul << 1)
#define VMM_USER (1ul << 2)
#define VMM_XD (1ul << 63)

#define PML1(virt) ((virt >> 12) & 0x1ff)
#define PML2(virt) ((virt >> 21) & 0x1ff)
#define PML3(virt) ((virt >> 30) & 0x1ff)
#define PML4(virt) ((virt >> 39) & 0x1ff)

#define PHYSADDR(virt) (virt & 0x0007fffffffff000)

void vmm_init();
void vmm_switch(pagemap *pm);
void vmm_map(pagemap *pm, uintptr_t phys, uintptr_t virt, uint64_t flags);
void vmm_unmap(pagemap *pm, uintptr_t virt);

pagemap vmm_new();
void vmm_destroy(pagemap *pm);
