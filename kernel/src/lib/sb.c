#include <lib/sb.h>
#include <lib/string.h>
#include <mm/heap.h>

sb sb_new() {
    return sb_with_capacity(10);
}
sb sb_with_capacity(size_t cap) {
    sb self;
    if (cap == 0) {
        self.data = nullptr;
    } else {
        self.data = kmalloc(cap);
        memset(self.data, 0, cap);
    }
    self.cap = cap;
    self.len = 0;

    return self;
}

void sb_push_str(sb *self, const char *str) {
    if (str == nullptr) {
        return;
    }
    size_t len = strlen(str);
    // We reached the capacity
    if (self->len + len + 1 >= self->cap) {
        sb_resize(self, (self->cap + len) * 2);
    }
    memcpy(self->data + self->len, str, len);
    self->len += len;
    self->data[self->len] = '\0';
}
void sb_push_ch(sb *self, char ch) {
    // We reached the capacity
    if (self->len + 1 >= self->cap) {
        sb_resize(self, self->cap * 2);
    }
    self->data[self->len] = ch;
    self->len++;
    self->data[self->len] = '\0';
}

void sb_resize(sb *self, size_t new_cap) {
    size_t diff = new_cap - self->len;

    self->data = krealloc(self->data, new_cap);
    memset(self->data + self->len, 0, diff);
    self->cap = new_cap;
}

sb sb_clone(sb *self) {
    sb new_self = *self;
    new_self.data = kmalloc(self->cap);
    memcpy(new_self.data, self->data, self->cap);

    return new_self;
}

void sb_clear(sb *self) {
    memset(self->data, 0, self->cap);
    self->len = 0;
}
void sb_free(sb *self) {
    kfree(self->data);

    self->data = nullptr;
    self->len = 0;
    self->cap = 0;
}
