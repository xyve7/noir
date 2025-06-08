#include <dev/console.h>
#include <lib/printf.h>
#include <sys/vfs.h>

status console_write(void *ignore, const void *buffer, size_t bytes, size_t *written_bytes) {
    const char *b = buffer;
    while (bytes > 0) {
        write_char(*b);
        b++;
        bytes--;
        (*written_bytes)++;
    }
    return OK;
}

void console_init() {
    vfs_create("/dev/console", VFS_DEV, nullptr, nullptr, console_write, nullptr, nullptr);
}
