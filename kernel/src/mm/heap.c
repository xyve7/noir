#include "kernel.h"
#include <mm/heap.h>
#include <mm/pmm.h>

void *base = nullptr;
void *end = nullptr;
void *current = nullptr;

// This value can be changed if we keep running out of memory
// Currently, I'm using a bitmap allocator, so the pages are guaranteed to be next to one another
size_t pages = 1;

alloc_header *used_list = nullptr;
alloc_header *free_list = nullptr;

void heap_init() {
    // Initialize the memory
    base = VIRT(pmm_alloc(pages));
    end = base + PAGE_SIZE * pages;
    current = base;
}
void *kmalloc(size_t size) {
    if (size == 0) {
        return nullptr;
    }
    // We check the free list first
    alloc_header *n = free_list;
    while (n) {
        if (n->size >= size) {
            // Remove it from the free list
            if (n->prev) {
                n->prev->next = n->next;
            } else {
                // Head of the list
                free_list = n->next;
            }

            // Add it to the used list
            n->next = used_list;
            used_list = n;

            // Return
            return (void *)(n + 1);
        }
        n = n->next;
    }
    // Get the block
    alloc_header *block = current;
    // Advance the pointer
    size_t total_size = sizeof(alloc_header) + size;
    size_t advance_bytes = ROUND(total_size, sizeof(alloc_header));
    current += advance_bytes;

    // Too much memory allocated
    if (current >= end) {
        PANIC("out of memory");
    }

    // The allocation was fine
    block->prev = nullptr;
    block->size = size;
    block->next = used_list;

    // Add to the list
    used_list = block;

    return (void *)(block + 1);
}
void kfree(void *ptr) {
    if (!ptr) {
        return;
    }
    alloc_header *block = (alloc_header *)ptr - 1;
    // Remove it from the used list
    if (block->prev) {
        block->prev->next = block->next;
    } else {
        // Head of the list
        used_list = block->next;
    }

    // Add it to the free list
    block->next = free_list;
    free_list = block;
}

// This function will wipe every allocation
// This should NOT be done unless you are 100% sure that it wont cause the entire
// system to go up in flames
//
// This only exists to wipe the heap tests out when we initialize the system
void heap_clear() {
    used_list = nullptr;
    free_list = nullptr;

    current = base;
    LOG("heap has been cleared\n");
}
void heap_status() {
    LOG("free: %lu bytes\n", end - current);
    LOG("used: %lu bytes\n", current - base);
    LOG("total: %lu bytes\n", end - base);
}
