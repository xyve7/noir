#pragma once

#include <stdint.h>
#include <task/sched.h>

#define CPU_MAX 16

typedef struct {
    thread *current_thread;
    uint64_t kernel_stack;
    uint64_t user_stack;
} cpu;

void cpu_set(cpu *c);
cpu *cpu_get();
