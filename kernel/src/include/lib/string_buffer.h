#pragma once

#include <stddef.h>

typedef struct {
    char *data;
    size_t len;
    size_t cap;
} string_buffer;

string_buffer string_buffer_new();
string_buffer string_buffer_with_capacity(size_t cap);

void string_buffer_push_str(string_buffer *self, const char *str);
void string_buffer_push_ch(string_buffer *self, char ch);

void string_buffer_resize(string_buffer *self, size_t new_cap);

string_buffer string_buffer_clone(string_buffer *self);

void string_buffer_clear(string_buffer *self);
void string_buffer_free(string_buffer *self);
