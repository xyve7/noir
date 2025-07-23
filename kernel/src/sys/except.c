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
        PANIC("Vector exceeds 32: %hhu\n", vector);
    }
    except_handlers[vector] = handler;
    LOG("Registered Exception Handler: handler=%p, vector=%hhu", handler, vector);
}
void except_handler(cpu_context *frame) {
    if (except_handlers[frame->no]) {
        except_handlers[frame->no](frame);
    } else {
        PANIC(
            "Exception: %s (Code: 0x%lx)\n"
            "Registers:\n"
            "r15=0x%lx r14=0x%lx r13=0x%lx r12=0x%lx\n"
            "r11=0x%lx r10=0x%lx r9=0x%lx  r8=0x%lx\n"
            "rbp=0x%lx rdi=0x%lx rsi=0x%lx rdx=0x%lx\n"
            "rcx=0x%lx rbx=0x%lx rax=0x%lx no=0x%lx\n"
            "err=0x%lx rip=0x%lx cs=0x%lx  rflags=0x%lx\n"
            "rsp=0x%lx ss=0x%lx\n",
            exceptions[frame->no], frame->err,
            frame->r15, frame->r14, frame->r13, frame->r12,
            frame->r11, frame->r10, frame->r9, frame->r8,
            frame->rbp, frame->rdi, frame->rsi, frame->rdx,
            frame->rcx, frame->rbx, frame->rax, frame->no,
            frame->err, frame->rip, frame->cs, frame->rflags,
            frame->rsp, frame->ss
        );
    }
}
