#include <lib/string.h>
#include <lib/vector.h>
#include <mm/heap.h>

vector vector_new() {
    return vector_with_capacity(10);
}
vector vector_with_capacity(size_t cap) {
    vector self;
    if (cap == 0) {
        self.data = nullptr;
    } else {
        self.data = heap_alloc(sizeof(void *) * cap);
    }
    self.cap = cap;
    self.len = 0;

    return self;
}

void *vector_at(vector *self, size_t i) {
    if (i < self->len) {
        return self->data[i];
    }
    return nullptr;
}
bool vector_set(vector *self, size_t i, void *item) {
    if (i < self->len) {
        self->data[i] = item;
        return true;
    }
    return false;
}

void vector_push(vector *self, void *item) {
    // We reached the capacity
    if (self->len >= self->cap) {
        vector_resize(self, self->cap * 2);
    }
    self->data[self->len] = item;
    self->len++;
}
void vector_pop(vector *self) {
    if (self->len > 0) {
        self->len--;
    }
}

void vector_resize(vector *self, size_t new_cap) {
    self->data = heap_realloc(self->data, sizeof(void *) * new_cap);
    self->cap = new_cap;
}

bool vector_find(vector *self, void *item, size_t *i, bool (*eq)(const void *, const void *)) {
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

vector vector_clone(vector *self) {
    vector new_self = *self;
    new_self.data = heap_alloc(sizeof(void *) * self->cap);
    memcpy(new_self.data, self->data, sizeof(void *) * self->cap);

    return new_self;
}

void vector_clear(vector *self) {
    self->len = 0;
}
void vector_free(vector *self) {
    heap_free(self->data);

    self->data = nullptr;
    self->len = 0;
    self->cap = 0;
}
