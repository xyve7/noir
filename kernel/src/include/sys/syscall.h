#pragma once

#include <stdint.h>

typedef struct {
	uint64_t r11;
	uint64_t r10;
	uint64_t r9;
	uint64_t r8;
	uint64_t rdi;
	uint64_t rsi;
	uint64_t rdx;
	uint64_t rcx;
	uint64_t rax;
} syscall_context;

typedef enum : uint64_t {
    SYS_OPEN = 0,
    SYS_CLOSE = 1,
    SYS_READ = 2,
    SYS_WRITE = 3,
    SYS_EXIT = 4
} syscall_no;

// Make fun of me or whatever, but I have horrible memory.
// I cannot for the life of me remember which register has which argument.
// This is just here so I don't have to have the website open everytime.
// rdi, rsi, rdx, r10, r8, r9
#define ARG0(state) ((state)->rdi)
#define ARG1(state) ((state)->rsi)
#define ARG2(state) ((state)->rdx)
#define ARG3(state) ((state)->r10)
#define ARG4(state) ((state)->r8)
#define ARG5(state) ((state)->r9)

// Enable the SYSCALL instruction
void syscall_init();
