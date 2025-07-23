#pragma once

#include <stddef.h>

typedef struct {
    void **data;
    size_t cap, len;
} vector;

vector vector_new();
vector vector_with_capacity(size_t cap);

void *vector_at(vector *self, size_t i);
bool vector_set(vector *self, size_t i, void *item);

// bool vector_insert(vector *self, size_t i, void *item);
// bool vector_remove(vector *self, size_t i);

void vector_push(vector *self, void *item);
void vector_pop(vector *self);

void vector_resize(vector *self, size_t new_cap);

bool vector_find(vector *self, void *item, size_t *i, bool (*eq)(const void *, const void *));

vector vector_clone(vector *self);

void vector_clear(vector *self);
void vector_free(vector *self);
