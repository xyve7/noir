#include <stdint.h>
#include <string.h>

void *memcpy(void *dest, const void *src, size_t n) {
    uint8_t *d = dest;
    const uint8_t *s = src;
    while (n--) {
        *d = *s;
    }
    return dest;
}
size_t strlen(const char *s) {
    size_t len = 0;
    while (*s) {
        s++;
        len++;
    }
    return len;
}
