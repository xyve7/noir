#ifndef KERNEL_H
#define KERNEL_H

#include <limine.h>
#include <stdint.h>

// Log to both serial and terminal
#define LOG_INFO(x, ...) log(0, __FILE__, __func__, __LINE__, x, ##__VA_ARGS__)
#define LOG_WARN(x, ...) log(1, __FILE__, __func__, __LINE__, x, ##__VA_ARGS__)
#define LOG_DEBUG(x, ...) log(2, __FILE__, __func__, __LINE__, x, ##__VA_ARGS__)
// Log and die
#define PANIC(x, ...) ({                                    \
    log(3, __FILE__, __func__, __LINE__, x, ##__VA_ARGS__); \
    hcf();                                                  \
})

#define ASSERT(expr, format, ...) ({                                 \
    if (!(expr)) {                                                   \
        log(3, __FILE__, __func__, __LINE__, format, ##__VA_ARGS__); \
        hcf();                                                       \
    }                                                                \
})

// Unreachable ode
#define UNREACHABLE() __builtin_unreachable()
// Maximum
#define MAX(a, b) (a) > (b) ? (a) : (b)
// Minimum
#define MIN(a, b) (a) < (b) ? (a) : (b)
// Align up (n) by factor (f)
#define ALIGN_UP(n, f) ((((n) + (f) - 1) / (f)) * (f))
// Align down (n) by factor (f)
#define ALIGN_DOWN(n, f) ((n) / (f) * (f))
// Disable optimization
#define UNOPTIMIZED __attribute__((optimize("O0")))

// Kernel Information
#define NAME "noir"
#define VERSION_MAJOR 0
#define VERSION_MINOR 0
#define VERSION_PATCH 0
#define PRERELEASE_LABEL "dev"
// This is defined at compile time
#ifndef BUILD
#define BUILD "0000000"
#endif

__attribute__((used, section(".limine_requests"))) static volatile struct limine_hhdm_request hhdm_request;

void log(int kind, const char *file, const char *func, uint32_t line, const char *restrict format, ...);
void hcf();

#endif
