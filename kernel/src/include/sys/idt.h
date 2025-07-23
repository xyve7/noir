#pragma once

#include <stdint.h>


// Load the IDT
void idt_load();
// Create the entries
void idt_init();
