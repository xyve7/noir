#pragma once

#include <cpu/cpu.h>
#include <stdint.h>

void irq_init();
void irq_register_handler(uint8_t vector, void (*handler)(stack_frame *));
