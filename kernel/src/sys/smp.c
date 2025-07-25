
#include <cpu/cpu.h>
#include <stdint.h>
#include <sys/lapic.h>
#include <sys/smp.h>

#define IA32_GS_BASE 0xC0000101
#define IA32_KERNEL_GS_BASE 0xC0000102

cpu *cpus[CPU_MAX];

void cpu_set(cpu *c) {
    cpus[lapic_id()] = c;
    wrmsr(IA32_KERNEL_GS_BASE, (uint64_t)c);
    wrmsr(IA32_GS_BASE, (uint64_t)c);
}
cpu *cpu_get() {
    return (cpu *)rdmsr(IA32_KERNEL_GS_BASE);
}
