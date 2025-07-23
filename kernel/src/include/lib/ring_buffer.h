#pragma once

#include <stddef.h>
#include <stdint.h>

typedef struct {
    uint8_t data[8];
    size_t read;
    size_t write;
    size_t count;
} ring_buffer;

ring_buffer ring_buffer_new();
void ring_buffer_write(ring_buffer *self, uint8_t byte);
uint8_t ring_buffer_read(ring_buffer *self);
bool ring_buffer_empty(ring_buffer *self);
bool ring_buffer_full(ring_buffer *self);
void ring_buffer_clear(ring_buffer *self);
