#include "dev/serial.h"
#include <kernel.h>
#include <lib/printf.h>
#include <sys/except.h>

const char *exceptions[] = {
    "Divide Error",
    "Debug Exception",
    "NMI Interrupt",
    "Breakpoint",
    "Overflow",
    "BOUND Range Exceeded",
    "Invalid Opcode ",
    "Device Not Available ",
    "Double Fault",
    "Coprocessor Segment Overrun ",
    "Invalid TSS",
    "Segment Not Present",
    "Stack-Segment Fault",
    "General Protection",
    "Page Fault",
    "Intel reserved",
    "x87 FPU Floating-Point Fault",
    "Alignment Check",
    "Machine Check",
    "SIMD Floating-Point Exception",
    "Virtualization Exception",
    "Control Protection Exception"
};

void except_handler(stack_frame *frame) {
    printf("exception: %s (code: %lx)\n", exceptions[frame->no], frame->err);
    serial_printf("exception: %s (code: %lx)\n", exceptions[frame->no], frame->err);
    printf("faulting address: %p\n", (void *)frame->rip);
    serial_printf("faulting address: %p\n", (void *)frame->rip);
    hcf();
}
