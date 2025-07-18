#include <noir.h>
#include <syscall.h>

void _start() {
    uint64_t fd;
    sys_open("/device/console", 0, &fd);

    sys_write(fd, "Hello Noir!", 11);

    sys_exit();
}
