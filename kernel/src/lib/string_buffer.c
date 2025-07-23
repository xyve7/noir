#include <lib/string.h>
#include <lib/string_buffer.h>
#include <mm/heap.h>

string_buffer string_buffer_new() {
    return string_buffer_with_capacity(10);
}
string_buffer string_buffer_with_capacity(size_t cap) {
    string_buffer self;
    if (cap == 0) {
        self.data = nullptr;
    } else {
        self.data = heap_alloc(cap);
        memset(self.data, 0, cap);
    }
    self.cap = cap;
    self.len = 0;

    return self;
}

void string_buffer_push_str(string_buffer *self, const char *str) {
    if (str == nullptr) {
        return;
    }
    size_t len = strlen(str);
    // We reached the capacity
    if (self->len + len + 1 >= self->cap) {
        string_buffer_resize(self, (self->cap + len) * 2);
    }
    memcpy(self->data + self->len, str, len);
    self->len += len;
    self->data[self->len] = '\0';
}
void string_buffer_push_ch(string_buffer *self, char ch) {
    // We reached the capacity
    if (self->len + 1 >= self->cap) {
        string_buffer_resize(self, self->cap * 2);
    }
    self->data[self->len] = ch;
    self->len++;
    self->data[self->len] = '\0';
}

void string_buffer_resize(string_buffer *self, size_t new_cap) {
    size_t diff = new_cap - self->len;

    self->data = heap_realloc(self->data, new_cap);
    memset(self->data + self->len, 0, diff);
    self->cap = new_cap;
}

string_buffer string_buffer_clone(string_buffer *self) {
    string_buffer new_self = *self;
    new_self.data = heap_alloc(self->cap);
    memcpy(new_self.data, self->data, self->cap);

    return new_self;
}

void string_buffer_clear(string_buffer *self) {
    memset(self->data, 0, self->cap);
    self->len = 0;
}
void string_buffer_free(string_buffer *self) {
    heap_free(self->data);

    self->data = nullptr;
    self->len = 0;
    self->cap = 0;
}
