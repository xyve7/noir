#pragma once

#include <stdint.h>

// Syscalls exposed to the user
typedef enum : uint64_t {
    SYS_OPEN = 0,
    SYS_WRITE = 1,
    SYS_READ = 2,
    SYS_CLOSE = 3,
    SYS_EXIT = 4,
    SYS_MAP = 5
} syscalls;

// Initialize system calls
// Install the syscall handler on 0x80
void syscall_init();
