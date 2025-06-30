#pragma once

#include <cpu/cpu.h>
#include <fs/vfs.h>
#include <lib/vec.h>
#include <mm/vmm.h>
#include <stdint.h>

// File structure used to
// keep track of open files
typedef struct {
    // File descriptor
    uint64_t fd;

    // File state
    // Internal VFS node
    vnode *node;
    uint64_t offset;

} proc_file;

// Process state
typedef enum {
    ENQUEUED,
    RUNNING,
    IDLE,
    EXITED
} proc_state;

// Process structure
typedef struct proc {
    void *start;
    vec files;
    proc_state state;
    cpu_context context;
    pagemap pm;
    struct proc *next;
} proc;

// Initialize processing
void proc_init();
// Enqueues a new task
void proc_enqueue(void *start);
void proc_elf(void *buffer, size_t size);
// Switches the task given the current context
void proc_switch(cpu_context *frame);
// Exit
void proc_exit(proc *p);
