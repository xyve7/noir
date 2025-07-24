#pragma once

#include <cpu/cpu.h>
#include <stdint.h>

// Initialize exceptions
void except_init();
// Register exception handler
void except_register_handler(uint8_t vector, void (*handler)(int_context *));
