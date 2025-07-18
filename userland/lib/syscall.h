#pragma once

#include <noir.h>

extern error sys_open(const char *path, uint8_t flags, uint64_t *out_fd);
extern error sys_read(uint64_t fd, void *buffer, size_t size);
extern error sys_write(uint64_t fd, const void *buffer, size_t size);
extern error sys_close(uint64_t fd);
extern error sys_exit();
