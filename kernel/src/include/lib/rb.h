#pragma once

#include <stddef.h>
#include <stdint.h>

typedef struct {
    uint8_t data[8];
    size_t read;
    size_t write;
    size_t count;
} rb;

rb rb_new();
void rb_write(rb *self, uint8_t byte);
uint8_t rb_read(rb *self);
bool rb_empty(rb *self);
bool rb_full(rb *self);
void rb_clear(rb *self);
