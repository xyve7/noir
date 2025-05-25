#pragma once

#include <mm/pmm.h>

typedef uintptr_t pagemap;

void vmm_init();
void vmm_switch(pagemap *pm);
void vmm_map(pagemap *pm, uintptr_t phys, uintptr_t virt, uint64_t flags);
void vmm_unmap(pagemap *pm, uintptr_t virt);
