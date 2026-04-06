#include <arch/aarch64/cpu.h>
#include <arch/aarch64/except.h>
#include <kernel.h>
#include <stdint.h>

extern uint32_t aarch64_vector_table[];
void aarch64_except_init() {
	// I'm writing this so I don't have to relearn this (my memory loves failing me)
	// We modify the VBAR_EL1 since we are CURRENTLY only handling exceptions from EL1 to EL1 using SP_EL1
	// The issue is that SPSel is 0, meaning that sp points to SP_EL0
	// We modify SPSel to 1, now it points to SP_EL1 which has some garbage value
	// We copy the stack pointer so it doesn't crash and burn due to a bogus memory address
	
	// The order of what I do (setting the VBAR and then setting the EL1 stack) is important
	// Switching it doesn't work, why? I don't know, I'll figure it out one day
    asm volatile(
        "msr VBAR_EL1, %0\n"
        "mov x0, sp\n"
        "msr SPSel, #1\n"
        "mov sp, x0\n"
        :
        : "r"((uint64_t)aarch64_vector_table)
        : "memory"
    );
    LOG_INFO("Exceptions Initialized");
}
