#include <kprintf.h>
#include <stddef.h>
#include <mm/pfa.h>
#include <stdint.h>
#include <stdbool.h>
#include <libk/string.h>

// We will use a bitmap to keep track of page 

uint8_t *bitmap = NULL;
size_t memory_size = 0;
size_t total_pages = 0;
size_t bitmap_size = 0;

static inline void set(size_t i) {
	bitmap[i >> 3] |= (1 << (i & 0b111));
}
static inline bool get(size_t i) {
	return bitmap[i >> 3] & (1 << (i & 0b111));
}
static inline void clear(size_t i) {
	bitmap[i >> 3] &= ~(1 << (i & 0b111));
}
static inline void toggle(size_t i) {
	bitmap[i >> 3] ^= ~(1 << (i & 0b111));
}

__attribute__((used, section(".limine_requests"))) static volatile struct limine_memmap_request memmap_request = {
    .id = LIMINE_MEMMAP_REQUEST_ID,
    .revision = 0
};
__attribute__((used, section(".limine_requests"))) static volatile struct limine_hhdm_request hhdm_request = {
    .id = LIMINE_HHDM_REQUEST_ID,
    .revision = 0
};

void pfa_init() {
	// 2 guarantees are made by the Limine Protocol which makes our lives really easy
	// - The entries are guaranteed to be sorted by base address, lowest to highest.
	// - Usable and bootloader reclaimable entries are guaranteed to be 4096 byte aligned for both base and length.
	
	// We need to find the highest address, plus the length, which is then the total memory the bitmap cares about 
	if (memmap_request.response == NULL) {
		PANIC("Bootloader returned no memory map");
	}
	
	uint64_t count = memmap_request.response->entry_count;
	struct limine_memmap_entry **entries = memmap_request.response->entries;
	struct limine_memmap_entry *last_usable = NULL;
	for (uint64_t i = count - 1; i >= 0; i--) {
		if (entries[i]->type == LIMINE_MEMMAP_USABLE) {
			last_usable = entries[i];
			break;
		}
		if (i == 0) {
			break;
		}
	}

	// This is how much total memory the bitmap cares about
	// Past the last usable entry, it doesn't care
	memory_size = last_usable->base + last_usable->length;

	// This is simple since if the base and length are divisible by 4096, so is their sum 
	total_pages = memory_size / PAGE_SIZE;

	// Each bit can track 8 pages, if we have 9 pages lets say, it needs 2 bytes 
	// Dividing it by 9/8 would give us 1, so we have to round up to 8 then divide by 8, giving us 2 bytes
	bitmap_size = ALIGN_UP(total_pages, 8) / 8;
	
	// We gotta make room for the bitmap in the memory map, but the issue is we cant just subtract the size 
	// Its not page aligned, and its gonna screw up the rest of the pfa if its not aligned 
	size_t bitmap_size_aligned = ALIGN_UP(bitmap_size, PAGE_SIZE);

	for (uint64_t i = 0; i < count; i++) {
		if (entries[i]->type != LIMINE_MEMMAP_USABLE || entries[i]->length < bitmap_size_aligned) {
			continue;
		}
		bitmap = (void*)VIRT(entries[i]->base);

		entries[i]->base += bitmap_size_aligned;
		entries[i]->length -= bitmap_size_aligned;
		break;
	}
	if (bitmap == NULL) {
		PANIC("No suitable location for bitmap found");
	}

	// We assume everything is used then clear up the entries that aren't used
	memset(bitmap, 0xFF, bitmap_size_aligned);

	for (uint64_t i = 0; i < count; i++) {
		if (entries[i]->type != LIMINE_MEMMAP_USABLE) {
			continue;
		}
		for (uint64_t j = 0; j < entries[i]->length; j += PAGE_SIZE) {
			size_t base = entries[i]->base + j;
			clear(base / PAGE_SIZE);
		}
	}
	LOG_INFO("PFA Initialized");
}
void *pfa_get_pages(size_t page_count) {
	for (size_t i = 0; i < total_pages; i++) {
		if (get(i)) {
			continue;
		}
		// We hit a free page, make sure the number of consecutive pages 
		// is the same as page_count
		size_t cons = 1;
		for (size_t j = 1; j < page_count; j++) {
			if (!get(i + j)) {
				cons++;
			}
		}
		// No allocation found, continue
		if (cons != page_count) {
			continue;
		}
		
		void *base = (void*)(i * PAGE_SIZE);
		for (size_t j = 0; j < page_count; j++) {
			set(i + j);
		}
		return base;
	}
	PANIC("Out of memory");
	UNREACHABLE();
}
void pfa_free_pages(void *first_page, size_t page_count) {
	if ((uint64_t)first_page >= hhdm_request.response->offset) {
		PANIC("Attempting to free virtual address, use PHYS()");
	}
	size_t i = (size_t)first_page / PAGE_SIZE;
	for (size_t j = 0; j < page_count; j++) {
		clear(i + j);
	}
}
