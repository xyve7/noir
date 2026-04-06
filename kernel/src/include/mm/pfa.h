#ifndef PFA_H
#define PFA_H

#include <stddef.h>

#define PAGE_SIZE 4096

void pfa_init();
void *pfa_get_pages(size_t page_count);
void pfa_free_pages(void *first_page, size_t page_count);

#endif
