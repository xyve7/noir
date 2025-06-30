#pragma once

#include <cpu/cpu.h>
#include <stdint.h>

void pit_init();
void pit_handler(cpu_context *frame);
void pit_sleep(uint64_t ms);
uint64_t pit_tick();
