#pragma once

// The PIT is only used when calibrating the LAPIC timer

#include <cpu/cpu.h>
#include <stdint.h>

void pit_init();
void pit_handler(int_context *frame);
void pit_sleep(uint64_t ms);
uint64_t pit_tick();
