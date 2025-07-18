#pragma once

#include <cpu/cpu.h>
#include <sys/proc.h>

#define SCHED_MAX_PROCESSES 128

void sched_enqueue(process *proc);
void sched_schedule(cpu_context *state);
void sched_yield();
