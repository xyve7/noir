#ifndef KERNEL_H
#define KERNEL_H

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

void log(int kind, const char *file, const char *func, uint32_t line, const char *restrict format, ...);
void hcf();

#endif
