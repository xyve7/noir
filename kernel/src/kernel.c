#include <kernel.h>

// Define kernel information
const uint8_t kernel_patch = 0;
const uint8_t kernel_minor = 1;
const uint8_t kernel_major = 0;

const char *kernel_name = "noir";
const char *kernel_revision = "dev";
const char *kernel_timestamp = __DATE__ " "__TIME__;
#ifdef KERNEL_BUILD
const char *kernel_build = KERNEL_BUILD;
#else
const char *kernel_build = "unknown";
#endif

// Halt and catch fire function.
void hcf(void) {
    for (;;) {
#if defined(__x86_64__)
        asm("hlt");
#elif defined(__aarch64__) || defined(__riscv)
        asm("wfi");
#elif defined(__loongarch64)
        asm("idle 0");
#endif
    }
}
