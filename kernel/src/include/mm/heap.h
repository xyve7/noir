#pragma once

#include <stddef.h>

typedef struct alloc_header alloc_header;

typedef struct alloc_header {
    size_t size;
    alloc_header *next;
    alloc_header *prev;
} alloc_header;

void heap_init();
void heap_clear();
void heap_status();
void *kmalloc(size_t size);
void kfree(void *ptr);
