#include <lib/string.h>
#include <lib/vec.h>
#include <mm/heap.h>

vec vec_new() {
    return vec_with_capacity(10);
}
vec vec_with_capacity(size_t cap) {
    vec self;
    if (cap == 0) {
        self.data = nullptr;
    } else {
        self.data = kmalloc(sizeof(void *) * cap);
    }
    self.cap = cap;
    self.len = 0;

    return self;
}

void *vec_at(vec *self, size_t i) {
    if (i < self->len) {
        return self->data[i];
    }
    return nullptr;
}
bool vec_set(vec *self, size_t i, void *item) {
    if (i < self->len) {
        self->data[i] = item;
        return true;
    }
    return false;
}

void vec_push(vec *self, void *item) {
    // We reached the capacity
    if (self->len >= self->cap) {
        vec_resize(self, self->cap * 2);
    }
    self->data[self->len] = item;
    self->len++;
}
void vec_pop(vec *self) {
    if (self->len > 0) {
        self->len--;
    }
}

void vec_resize(vec *self, size_t new_cap) {
    self->data = krealloc(self->data, sizeof(void *) * new_cap);
    self->cap = new_cap;
}

bool vec_find(vec *self, void *item, size_t *i, bool (*eq)(const void *, const void *)) {
    for (size_t j = 0; j < self->len; j++) {
        void *current = self->data[j];
        if (eq(item, current)) {
            if (i) {
                *i = j;
            }
            return true;
        }
    }
    return false;
}

vec vec_clone(vec *self) {
    vec new_self = *self;
    new_self.data = kmalloc(sizeof(void *) * self->cap);
    memcpy(new_self.data, self->data, sizeof(void *) * self->cap);

    return new_self;
}

void vec_clear(vec *self) {
    self->len = 0;
}
void vec_free(vec *self) {
    kfree(self->data);

    self->data = nullptr;
    self->len = 0;
    self->cap = 0;
}
