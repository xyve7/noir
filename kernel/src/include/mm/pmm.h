#pragma once

#include <kernel.h>
#include <stddef.h>
#include <stdint.h>

// Virtual address
#define VIRT(x) (void *)(((uint64_t)(x)) + hhdm_request.response->offset)
// Physical address
#define PHYS(x) (void *)(((uint64_t)(x)) - hhdm_request.response->offset)

// Page size
#define PAGE_SIZE (4096)

// Initialize the physical memory manager
void pmm_init();
// Allocated pages
void *pmm_alloc(size_t count);
// Allocated pages, zeroed
void *pmm_allocz(size_t count);
// Free pages, address pointing to the beginning
void pmm_free(void *address, size_t count);
// Map the bitmap, called only from the vmm
void pmm_map(uintptr_t *pm);
