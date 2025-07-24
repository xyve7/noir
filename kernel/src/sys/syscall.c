#include <cpu/cpu.h>
#include <kernel.h>
#include <stdint.h>
#include <sys/syscall.h>

#define IA32_STAR 0xC0000081
#define IA32_LSTAR 0xC0000082
#define IA32_CSTAR 0xC0000083
#define IA32_SFMASK 0xC0000084
#define IA32_EFER 0xC0000080

extern void syscall_common();
typedef error (*syscall_entry)(syscall_context *state);

// These are defined elsewhere
extern error sys_open(syscall_context *state);
extern error sys_close(syscall_context *state);
extern error sys_read(syscall_context *state);
extern error sys_write(syscall_context *state);
extern error sys_exit(syscall_context *state);
extern error sys_version(syscall_context *state);

syscall_entry syscall_table[] = {
    [SYS_OPEN] = sys_open,
    [SYS_CLOSE] = sys_close,
    [SYS_READ] = sys_read,
    [SYS_WRITE] = sys_write,
    [SYS_EXIT] = sys_exit,
    [SYS_VERSION] = sys_version,
};

void syscall_handler(syscall_context *state) {
    syscall_table[state->rax](state);
}

void syscall_init() {
    uint64_t segments = 0;
    // This is what SYSRET will use.
    // I went off on a rant in gdt.c about how stupid this is.
    // Please, take a read.
    segments |= ((uint64_t)USER_CS << 48);
    segments |= ((uint64_t)KERNEL_CS << 32);

    // Enable the SYSCALL instruction
    wrmsr(IA32_EFER, rdmsr(IA32_EFER) | 1);
    wrmsr(IA32_STAR, segments);

    // The address to the assembly handler
    wrmsr(IA32_LSTAR, (uint64_t)syscall_common);

    LOG("SYSCALL Instruction Enabled");
}
