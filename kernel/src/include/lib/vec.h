#pragma once

#include <stddef.h>

typedef struct {
    void **data;
    size_t cap, len;
} vec;

vec vec_new();
vec vec_with_capacity(size_t cap);

void *vec_at(vec *self, size_t i);
bool vec_set(vec *self, size_t i, void *item);

// bool vec_insert(vec *self, size_t i, void *item);
// bool vec_remove(vec *self, size_t i);

void vec_push(vec *self, void *item);
void vec_pop(vec *self);

void vec_resize(vec *self, size_t new_cap);

bool vec_find(vec *self, void *item, size_t *i, bool (*eq)(const void *, const void *));

vec vec_clone(vec *self);

void vec_clear(vec *self);
void vec_free(vec *self);
