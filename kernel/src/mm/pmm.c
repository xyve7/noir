#include <mm/vmm.h>
#include <kernel.h>
#include <lib/spinlock.h>
#include <lib/string.h>
#include <mm/pmm.h>

__attribute__((used, section(".limine_requests"))) volatile struct limine_memmap_request memmap_request = {
    .id = LIMINE_MEMMAP_REQUEST,
    .revision = 0
};

spinlock pmm_lock = SPINLOCK_INIT;

// final usable address
uintptr_t last_usable_address = 0;
// total entries in the bitmap or the number of pages
uint64_t total_pages = 0;
// bitmap
uint8_t *bitmap = nullptr;
// size aligned by page size
uint64_t bitmap_size_aligned = 0;

void set(uint64_t i) {
    bitmap[i / 8] |= (1 << (i % 8));
}
uint8_t get(uint64_t i) {
    return bitmap[i / 8] & (1 << (i % 8));
}
void clear(uint64_t i) {
    bitmap[i / 8] &= ~(1 << (i % 8));
}

void pmm_init() {
    struct limine_memmap_response *resp = memmap_request.response;
    // Get highest usable address
    for (uint64_t i = resp->entry_count - 1; i >= 0; i--) {
        struct limine_memmap_entry *entry = resp->entries[i];
        if (entry->type == LIMINE_MEMMAP_USABLE) {
            last_usable_address = entry->base + entry->length;
            break;
        }
    }
    // Now get the number of pages it'll be
    total_pages = last_usable_address / PAGE_SIZE;
    // Get the bitmap size
    uint64_t bitmap_size = ROUND(total_pages, 8) / 8;
    // Align to the page size
    bitmap_size_aligned = ROUND(bitmap_size, PAGE_SIZE);

    // Find a place for the bitmap
    for (uint64_t i = 0; i < resp->entry_count; i++) {
        struct limine_memmap_entry *entry = resp->entries[i];
        if (entry->type == LIMINE_MEMMAP_USABLE && entry->length >= bitmap_size_aligned) {
            bitmap = (uint8_t *)VIRT(entry->base);

            entry->base += bitmap_size_aligned;
            entry->length -= bitmap_size_aligned;

            memset(bitmap, 0xff, bitmap_size_aligned);
            break;
        }
    }

    // Set the free parts of the bitmap, free
    for (uint64_t i = 0; i < resp->entry_count; i++) {
        struct limine_memmap_entry *entry = resp->entries[i];
        if (entry->type == LIMINE_MEMMAP_USABLE) {
            for (uint64_t i = 0; i < entry->length; i += PAGE_SIZE) {
                clear((entry->base + i) / PAGE_SIZE);
            }
        }
    }

    LOG("PMM Initalized");
}

void *pmm_alloc(size_t count) {
    bool state = spinlock_acquire_irq_save(&pmm_lock);
    // for now, we will search from the start
    // yes this is slow, ill fix it later
    uint64_t found = 0;
    uint64_t start = 0;
    for (uint64_t i = 0; i < total_pages; i++) {
        // count up consecutive pages
        if (get(i)) {
            found = 0;
        } else {
            if (found == 0) {
                start = i;
            }
            found++;
        }
        // we found a page
        if (found == count) {
            // mark them used
            for (uint64_t j = 0; j < count; j++) {
                set(start + j);
            }
            // get the address
            void *address = (void *)(start * PAGE_SIZE);
            spinlock_release_irq_restore(&pmm_lock, state);
            return address;
        }
    }
    spinlock_release_irq_restore(&pmm_lock, state);

    PANIC("PMM is unable to allocate %lu pages\n", count);
    UNREACHABLE();
}
void *pmm_alloc_zeroed(size_t count) {
    void *page = VIRT(pmm_alloc(count));
    memset(page, 0, count * PAGE_SIZE);
    return PHYS(page);
}
void pmm_free(void *address, size_t count) {
    bool state = spinlock_acquire_irq_save(&pmm_lock);

    uint64_t page = (uint64_t)address / PAGE_SIZE;
    for (uint64_t i = 0; i < count; i++) {
        clear(page + i);
    }
    
    spinlock_release_irq_restore(&pmm_lock, state);
}

void pmm_map(uintptr_t *pt) {
    for (uint64_t i = 0; i < bitmap_size_aligned; i += PAGE_SIZE) {
        uintptr_t addr = (uintptr_t)(bitmap + i);
        vmm_map(pt, (uintptr_t)PHYS(addr), addr, VMM_PRESENT | VMM_WRITE);
    }
}
