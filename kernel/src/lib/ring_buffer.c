#include <lib/ring_buffer.h>
#include <lib/string.h>

ring_buffer ring_buffer_new() {
    return (ring_buffer){{0}, 0, 0, 0};
}

bool ring_buffer_empty(ring_buffer *self) {
    return self->count == 0;
}
bool ring_buffer_full(ring_buffer *self) {
    return self->count == sizeof(self->data);
}
void ring_buffer_write(ring_buffer *self, uint8_t byte) {
    if (ring_buffer_full(self)) {
        return;
    }

    size_t cap = sizeof(self->data);

    self->data[self->write] = byte;
    self->write = (self->write + 1) % cap;

    self->count++;
}
uint8_t ring_buffer_read(ring_buffer *self) {
    if (ring_buffer_empty(self)) {
        return 0;
    }
    size_t cap = sizeof(self->data);

    uint8_t byte = self->data[self->read];
    self->read = (self->read + 1) % cap;

    self->count--;
    return byte;
}
void ring_buffer_clear(ring_buffer *self) {
    memset(self->data, 0, sizeof(self->data));
    self->write = 0;
    self->read = 0;
    self->count = 0;
}
