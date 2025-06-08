#pragma once

#include <cpu/cpu.h>
#include <stdint.h>

void except_init();
void except_register_handler(uint8_t vector, void (*handler)(stack_frame *));
