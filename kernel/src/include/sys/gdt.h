#pragma once

#include <stdint.h>

void gdt_init();
void gdt_set_rsp0(uint64_t rsp0);
uint64_t gdt_tss_rsp();
