#pragma once

#include <stddef.h>

typedef struct {
    char *data;
    size_t len;
    size_t cap;
} sb;

sb sb_new();
sb sb_with_capacity(size_t cap);

void sb_push_str(sb *self, const char *str);
void sb_push_ch(sb *self, char ch);

void sb_resize(sb *self, size_t new_cap);

sb sb_clone(sb *self);

void sb_clear(sb *self);
void sb_free(sb *self);
