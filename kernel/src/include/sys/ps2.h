#pragma once

#include <stdint.h>

uint8_t ps2_read();
void ps2_write(uint8_t byte);
uint8_t ps2_status();
void ps2_init();
