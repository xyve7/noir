#pragma once

#include <limine.h>
#include <stdint.h>

// Kernel information
extern const char *kernel_name;
extern const char *kernel_revision;
extern const char *kernel_timestamp;
extern const char *kernel_build;

// Log to both serial and terminal
#define LOG(x, ...) log(0, __FILE__, __func__, __LINE__, x, ##__VA_ARGS__)
#define LOG_INFO(x, ...) log(0, __FILE__, __func__, __LINE__, x, ##__VA_ARGS__)
#define LOG_WARN(x, ...) log(1, __FILE__, __func__, __LINE__, x, ##__VA_ARGS__)
#define LOG_DEBUG(x, ...) log(2, __FILE__, __func__, __LINE__, x, ##__VA_ARGS__)
// Log and die
#define PANIC(x, ...) ({                                    \
    log(3, __FILE__, __func__, __LINE__, x, ##__VA_ARGS__); \
    hcf();                                                  \
})

// Unreachable ode
#define UNREACHABLE() __builtin_unreachable()
// Maximum
#define MAX(a, b) (a) > (b) ? (a) : (b)
// Minimum
#define MIN(a, b) (a) < (b) ? (a) : (b)
// Round value (n) by factor (f)
#define ROUND(n, f) (((n) + (f) - 1) / (f)) * (f)
// Disable optimization
#define UNOPTIMIZED __attribute__((optimize("O0")))

// CS and SS for kernel and usermode
#define KERNEL_CS 0x08
#define KERNEL_SS 0x10
#define USER_CS 0x18
#define USER_SS 0x20
#define USER_CS_DUP 0x28

__attribute__((used, section(".limine_requests"))) extern volatile struct limine_hhdm_request hhdm_request;
__attribute__((used, section(".limine_requests"))) extern volatile struct limine_memmap_request memmap_request;
__attribute__((used, section(".limine_requests"))) extern volatile struct limine_module_request module_request;
__attribute__((used, section(".limine_requests"))) extern volatile struct limine_framebuffer_request framebuffer_request;

// Framebuffer
extern struct limine_framebuffer *framebuffer;
// Errors returned to the user
typedef enum {
    // No action needed, successful
    OK,
    /* I/O */
    // Entry doesn't exist
    NO_ENTRY,
    // Entry already exists
    EXISTS,
    // File descriptor doesn't exist
    BAD_FD,
    // Entry is directory
    IS_DIR,
    // Entry is file
    IS_FILE,
    // Entry is device
    IS_DEV,
    // Read only
    READ_ONLY,
    // Write only
    WRITE_ONLY,
    // Unsupported operations
    BAD_OP,
    // Unsupported filesystem
    BAD_FS
} error;

void log(int kind, const char *file, const char *func, uint32_t line, const char *restrict format, ...);

// Halt and disable interrupts
void hcf();
