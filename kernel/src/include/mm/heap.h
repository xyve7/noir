#pragma once

#include <stddef.h>

// Initialize the heap
void heap_init();
// Clear the heap
void heap_clear();
// Test the heap
void heap_tests();
// Get the status
void heap_status();

// Allocate bytes
void *heap_alloc(size_t size);
// heap_realloc bytes
void *heap_realloc(void *ptr, size_t size);
// Free bytes
void heap_free(void *ptr);
