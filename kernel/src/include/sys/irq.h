#pragma once

#include <cpu/cpu.h>
#include <stdint.h>

// Initialize IRQs
void irq_init();
// Register IRQ handler
void irq_register_handler(uint8_t vector, void (*handler)(int_context *));
