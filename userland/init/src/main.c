#include <kernel/noir.h>
#include <kernel/syscall.h>
#include <string.h>

void version(uint64_t console_fd) {
    noir_version ver;
    sys_version(&ver);

    sys_write(console_fd, ver.name, strlen(ver.name));
    sys_write(console_fd, " ", 1);

    char c;
    c = '0' + ver.major;
    sys_write(console_fd, &c, 1);
    sys_write(console_fd, ".", 1);
    c = '0' + ver.minor;
    sys_write(console_fd, &c, 1);
    sys_write(console_fd, ".", 1);
    c = '0' + ver.patch;
    sys_write(console_fd, &c, 1);

    sys_write(console_fd, "-", 1);
    sys_write(console_fd, ver.revision, strlen(ver.revision));
    sys_write(console_fd, "+", 1);
    sys_write(console_fd, ver.build, strlen(ver.build));
    sys_write(console_fd, " ", 1);
    sys_write(console_fd, ver.timestamp, strlen(ver.timestamp));
}

void _start() {
    uint64_t console_fd;
    sys_open("/device/console", 0, &console_fd);

    version(console_fd);

    sys_close(console_fd);
    sys_exit();
}
