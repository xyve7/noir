#ifndef SYSCALL_H
#define SYSCALL_H

#include <kernel/noir.h>
#include <stddef.h>
#include <stdint.h>

extern Error sys_open(const char *path, uint8_t flags, FD *out_fd);
extern Error sys_read(FD fd, void *buffer, size_t size);
extern Error sys_write(FD fd, const void *buffer, size_t size);
extern Error sys_close(FD fd);
extern Error sys_exit();
extern Error sys_version(Version *nv);
extern Error sys_get_pid(PID *pid);

#endif
