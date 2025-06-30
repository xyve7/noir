#pragma once

#include <stdint.h>

// Send EOI
void lapic_eoi();
// Get current LAPIC ID
uint32_t lapic_id();
// Enable the LAPIC
void lapic_enable();
// Enable the timer
void lapic_timer_enable();
// Initialize the LAPIC
void lapic_init();
