#pragma once

#include <stdint.h>

void lapic_eoi();
uint32_t lapic_id();
void lapic_core();
void lapic_oneshot(uint8_t vector);
void lapic_init();
