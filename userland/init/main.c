#include <noir.h>
#include <string.h>
#include <syscall.h>

void _start() {
    uint64_t fd;
    sys_open("/device/console", 0, &fd);

    sys_write(fd, "From: /init\n", 12);
    sys_write(fd, "Hello Noir!\n", 12);
    sys_write(fd, "Reporting Version: ", 19);

    noir_version ver;
    sys_version(&ver);

    sys_write(fd, ver.name, strlen(ver.name));
    sys_write(fd, " ", 1);

    char c;
    c = '0' + ver.major;
    sys_write(fd, &c, 1);
    sys_write(fd, ".", 1);
    c = '0' + ver.minor;
    sys_write(fd, &c, 1);
    sys_write(fd, ".", 1);
    c = '0' + ver.patch;
    sys_write(fd, &c, 1);

    sys_write(fd, "-", 1);
    sys_write(fd, ver.revision, strlen(ver.revision));
    sys_write(fd, "+", 1);
    sys_write(fd, ver.build, strlen(ver.build));
    sys_write(fd, " ", 1);
    sys_write(fd, ver.timestamp, strlen(ver.timestamp));

    sys_exit();
}
