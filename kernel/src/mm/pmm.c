#include <kernel.h>
#include <lib/string.h>
#include <mm/pmm.h>
__attribute__((used, section(".limine_requests"))) volatile struct limine_memmap_request memmap_request = {
    .id = LIMINE_MEMMAP_REQUEST,
    .revision = 0
};
// final usable address
uintptr_t lastaddr = 0;
// total entries in the bitmap or the number of pages
uint64_t totalpages = 0;
// bitmap
uint8_t *bitmap = nullptr;
// size in bytes
uint64_t bitmapsize = 0;
// size aligned by page size
uint64_t bitmapsize_al = 0;

uint64_t pmm_bitmap_size() {
    return bitmapsize_al;
}
uint8_t *pmm_bitmap() {
    return bitmap;
}

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
            lastaddr = entry->base + entry->length;
            break;
        }
    }
    // Now get the number of pages it'll be
    totalpages = lastaddr / PAGE_SIZE;
    // Get the bitmap size
    bitmapsize = ROUND(totalpages, 8) / 8;
    // Align to the page size
    bitmapsize_al = ROUND(bitmapsize, PAGE_SIZE);

    // Find a place for the bitmap
    for (uint64_t i = 0; i < resp->entry_count; i++) {
        struct limine_memmap_entry *entry = resp->entries[i];
        if (entry->type == LIMINE_MEMMAP_USABLE && entry->length >= bitmapsize_al) {
            bitmap = (uint8_t *)VIRT(entry->base);

            entry->base += bitmapsize_al;
            entry->length -= bitmapsize_al;

            memset(bitmap, 0xff, bitmapsize_al);
            break;
        }
    }

    // Set free
    for (uint64_t i = 0; i < resp->entry_count; i++) {
        struct limine_memmap_entry *entry = resp->entries[i];
        if (entry->type == LIMINE_MEMMAP_USABLE) {
            for (uint64_t i = 0; i < entry->length; i += PAGE_SIZE) {
                clear((entry->base + i) / PAGE_SIZE);
            }
        }
    }

    LOG("pmm init\n");
}

void *pmm_alloc(size_t count) {
    // for now, we will search from the start
    // yes this is slow, ill fix it later
    uint64_t found = 0;
    uint64_t start = 0;
    for (uint64_t i = 0; i < totalpages; i++) {
        // count up consecutive pages
        if (get(i)) {
            found = 0;
        } else {
            start = i;
            found++;
        }
        // we found a page
        if (found == count) {
            // mark them  used
            for (uint64_t j = 0; j < count; j++) {
                set(start + j);
            }
            // get the address
            void *address = (void *)(start * PAGE_SIZE);
            return address;
        }
    }
    PANIC("pmm is unable to allocate %lu pages\n", count);
    UNREACHABLE();
}
void *pmm_allocz(size_t count) {
    void *page = VIRT(pmm_alloc(count));
    memset(page, 0, count * PAGE_SIZE);
    return PHYS(page);
}
void pmm_free(void *address, size_t count) {
    // assumes the address is in the lower half, or the physical address
    uint64_t page = (uint64_t)address;
    page /= PAGE_SIZE;

    for (uint64_t i = 0; i < count; i++) {
        clear(page + i);
    }
}
