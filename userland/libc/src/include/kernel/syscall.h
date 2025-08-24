#ifndef SYSCALL_H
#define SYSCALL_H

#include <kernel/noir.h>
#include <stddef.h>
#include <stdint.h>

extern error sys_open(const char *path, uint8_t flags, uint64_t *out_fd);
extern error sys_read(uint64_t fd, void *buffer, size_t size);
extern error sys_write(uint64_t fd, const void *buffer, size_t size);
extern error sys_close(uint64_t fd);
extern error sys_exit();
extern error sys_version(noir_version *nv);
extern error sys_get_pid(uint64_t *pid);

#endif
