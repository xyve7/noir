#include <lib/rb.h>
#include <lib/string.h>

rb rb_new() {
    return (rb){{0}, 0, 0, 0};
}

bool rb_empty(rb *self) {
    return self->count == 0;
}
bool rb_full(rb *self) {
    return self->count == sizeof(self->data);
}
void rb_write(rb *self, uint8_t byte) {
    if (rb_full(self)) {
        return;
    }

    size_t cap = sizeof(self->data);

    self->data[self->write] = byte;
    self->write = (self->write + 1) % cap;

    self->count++;
}
uint8_t rb_read(rb *self) {
    if (rb_empty(self)) {
        return 0;
    }
    size_t cap = sizeof(self->data);

    uint8_t byte = self->data[self->read];
    self->read = (self->read + 1) % cap;

    self->count--;
    return byte;
}
void rb_clear(rb *self) {
    memset(self->data, 0, sizeof(self->data));
    self->write = 0;
    self->read = 0;
    self->count = 0;
}
