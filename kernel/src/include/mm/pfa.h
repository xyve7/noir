#ifndef PFA_H
#define PFA_H

#include <stddef.h>
#include <kernel.h>

#define PAGE_SIZE 4096
#define VIRT(x) ((x) + (hhdm_request.response->offset))
#define PHYS(x) ((x) - (hhdm_request.response->offset))

void pfa_init();
void *pfa_get_pages(size_t page_count);
void pfa_free_pages(void *first_page, size_t page_count);

#endif
