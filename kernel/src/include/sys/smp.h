#pragma once

#include <stdint.h>
#include <sys/proc.h>

// Data structure to hold current CPU data
typedef struct {
    proc *current_proc;
} cpu;

// Get the current CPU
cpu *cpu_get();
// Set the current CPU
void cpu_set(cpu *);
// Initialize SMP
void smp_init();
