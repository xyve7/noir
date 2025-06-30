#pragma once

#include <stddef.h>

typedef struct alloc_header alloc_header;

// Header for allocation
typedef struct alloc_header {
    size_t size;
    alloc_header *next;
    alloc_header *prev;
} alloc_header;

// Initialize the heap
void heap_init();
// Clear the heap
void heap_clear();
// Test the heap
void heap_tests();
// Get the status
void heap_status();

// Allocate bytes
void *kmalloc(size_t size);
// krealloc bytes
void *krealloc(void *ptr, size_t size);
// Free bytes
void kfree(void *ptr);
