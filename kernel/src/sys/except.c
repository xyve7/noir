#include <dev/serial.h>
#include <kernel.h>
#include <lib/printf.h>
#include <sys/except.h>

static const char *exceptions[] = {
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

static void (*except_handlers[32])(cpu_context *);

void except_init() {
    LOG("Exceptions Initialized");
}

void except_register_handler(uint8_t vector, void (*handler)(cpu_context *)) {
    if (vector >= 32) {
        PANIC("vector value is too high: %hhu\n", vector);
    }
    except_handlers[vector] = handler;
    LOG("Registered Exception Handler: handler=%p, vector=%hhu", handler, vector);
}
void except_handler(cpu_context *frame) {
    if (except_handlers[frame->no]) {
        except_handlers[frame->no](frame);
    } else {
        printf("exception: %s (code: %lx)\n", exceptions[frame->no], frame->err);
        serial_printf("exception: %s (code: %lx)\n", exceptions[frame->no], frame->err);

        printf("faulting address: %p\n", (void *)frame->rip);
        serial_printf("faulting address: %p\n", (void *)frame->rip);
        hcf();
    }
}
