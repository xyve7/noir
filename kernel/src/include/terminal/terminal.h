#pragma once

#include <stdint.h>

void terminal_init(void *fb, uint64_t w, uint64_t h, uint64_t p);
void terminal_write(uint8_t ch);
