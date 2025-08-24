#include <kernel/noir.h>
#include <kernel/syscall.h>
#include <string.h>

void _start() {
    uint64_t fd;
    sys_open("/device/console", 0, &fd);

    sys_exit();
}
