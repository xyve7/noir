#pragma once

#include <mm/vmm.h>
#include <stdint.h>

uintptr_t elf_load_exec(page_table *pt, void *buffer);
uintptr_t elf_load_module(void *buffer);
