#pragma once

#include <limine.h>
#include <stdint.h>

#define LOG(x, ...) log(__FILE__, __func__, __LINE__, x, ##__VA_ARGS__)
#define PANIC(x, ...) ({                                 \
    log(__FILE__, __func__, __LINE__, x, ##__VA_ARGS__); \
    hcf();                                               \
})
#define UNREACHABLE() __builtin_unreachable()
#define MAX(a, b) (a) > (b) ? (a) : (b)
#define MIN(a, b) (a) < (b) ? (a) : (b)
#define ROUND(n, f) (((n) + (f) - 1) / (f)) * (f)

__attribute__((used, section(".limine_requests"))) extern volatile struct limine_hhdm_request hhdm_request;
__attribute__((used, section(".limine_requests"))) extern volatile struct limine_memmap_request memmap_request;

void log(const char *file, const char *func, uint32_t line, const char *restrict format, ...);
void hcf(void);
