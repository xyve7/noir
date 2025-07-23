#pragma once

#include <mm/pmm.h>

typedef uintptr_t page_table;
extern page_table kernel_page_table;

#define VMM_PRESENT (1ul << 0)
#define VMM_WRITE (1ul << 1)
#define VMM_USER (1ul << 2)
#define VMM_XD (1ul << 63)

void vmm_init();
void vmm_switch(page_table *pt);
void vmm_map(page_table *pt, uintptr_t phys, uintptr_t virt, uint64_t flags);
void vmm_unmap(page_table *pt, uintptr_t virt);

page_table vmm_new();
void vmm_destroy(page_table *pt);
