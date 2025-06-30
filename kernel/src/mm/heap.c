#include "cpu/cpu.h"
#include <kernel.h>
#include <lib/spinlock.h>
#include <lib/string.h>
#include <mm/heap.h>
#include <mm/pmm.h>

void *base = nullptr;
void *end = nullptr;
void *current = nullptr;

// This value can be changed if we keep running out of memory
// Currently, I'm using a bitmap allocator, so the pages are guaranteed to be next to one another
size_t pages = 16;

alloc_header *used_list = nullptr;
alloc_header *free_list = nullptr;

spinlock heap_lock = SPINLOCK_INIT;

void heap_init() {
    // Initialize the memory
    base = VIRT(pmm_alloc(pages));
    end = base + PAGE_SIZE * pages;
    current = base;

    LOG("Heap Initialized");
}
void *kmalloc(size_t size) {
    if (size == 0) {
        return nullptr;
    }

    bool state = int_get_state();
    int_disable();
    spinlock_acquire(&heap_lock);

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

            spinlock_release(&heap_lock);
            int_set_state(state);

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
        spinlock_release(&heap_lock);
        int_set_state(state);
        PANIC("out of memory: %ld\n", current - end);
    }

    // The allocation was fine
    block->prev = nullptr;
    block->size = size;
    block->next = used_list;

    // Add to the list
    used_list = block;

    spinlock_release(&heap_lock);
    int_set_state(state);

    return (void *)(block + 1);
}
void kfree(void *ptr) {
    if (!ptr) {
        return;
    }

    bool state = int_get_state();
    int_disable();
    spinlock_acquire(&heap_lock);

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

    spinlock_release(&heap_lock);
    int_set_state(state);
}
void *krealloc(void *ptr, size_t size) {
    if (ptr == nullptr) {
        return kmalloc(size);
    }

    // We grab the header
    alloc_header *block = (alloc_header *)ptr - 1;
    // Just a sanity check, if the actual size of the header is big enough, we dont bother
    // reallocating
    if (block->size >= size) {
        return ptr;
    }
    // It isn't lets, reallocate
    // This also assumes that the new size is bigger
    void *p = kmalloc(size);
    memcpy(p, ptr, block->size);

    // Free the original pointer
    kfree(ptr);

    return p;
}
// ChatGPT wrote this, too lazy to write tests
void heap_tests() {
    LOG("Heap tests starting...\n");

    // Test 1: Simple allocation/free
    void *ptr = kmalloc(64);
    if (!ptr) {
        LOG("FAIL: kmalloc(64) returned NULL\n");
    } else {
        LOG("PASS: kmalloc(64) returned %p\n", ptr);
        kfree(ptr);
        LOG("Freed memory at %p\n", ptr);
    }

    // Test 2: Multiple allocations/free
    void *ptrs[5];
    int i;
    int all_allocated = 1;
    for (i = 0; i < 5; i++) {
        ptrs[i] = kmalloc(32 * (i + 1));
        if (!ptrs[i]) {
            LOG("FAIL: kmalloc(%d) returned NULL\n", 32 * (i + 1));
            all_allocated = 0;
            break;
        } else {
            LOG("Allocated ptr[%d] = %p\n", i, ptrs[i]);
        }
    }

    if (all_allocated) {
        for (i = 0; i < 5; i++) {
            kfree(ptrs[i]);
            LOG("Freed ptr[%d] = %p\n", i, ptrs[i]);
        }
        LOG("PASS: Multiple allocations and frees succeeded\n");
    } else {
        // Free previously allocated ptrs in case of failure
        for (int j = 0; j < i; j++) {
            kfree(ptrs[j]);
            LOG("Freed ptr[%d] = %p\n", j, ptrs[j]);
        }
    }

    // Test 3: Allocation size zero
    ptr = kmalloc(0);
    if (ptr != NULL) {
        LOG("Warning: kmalloc(0) should usually return NULL or minimum size. Got %p\n", ptr);
        kfree(ptr);
    } else {
        LOG("PASS: kmalloc(0) returned NULL\n");
    }

    // Test 4: Free NULL pointer
    kfree(NULL); // should safely do nothing or handle gracefully
    LOG("PASS: kfree(NULL) did not crash\n");
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
