#pragma once

#include <kernel.h>
#include <stddef.h>
#include <stdint.h>

#define VIRT(x) (void *)(((uint64_t)(x)) + hhdm_request.response->offset)
#define PHYS(x) (void *)(((uint64_t)(x)) - hhdm_request.response->offset)

#define PAGE_SIZE (4096)

void pmm_init();
void *pmm_alloc(size_t count);
void *pmm_allocz(size_t count);
void pmm_free(void *address, size_t count);
uint64_t pmm_bitmap_size();
uint8_t *pmm_bitmap();
