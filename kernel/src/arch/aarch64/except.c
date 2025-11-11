#include <arch/aarch64/cpu.h>
#include <arch/aarch64/except.h>
#include <kernel.h>
#include <stdint.h>

extern uint32_t aarch64_vector_table[];

void aarch64_except_init() {
    asm volatile(
        "msr VBAR_EL1, %0\n"
        :
        : "r"((uint64_t)aarch64_vector_table)
        : "memory"
    );
    LOG_INFO("Exceptions Initialized");
}
