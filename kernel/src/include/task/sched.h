#pragma once

#include <cpu/cpu.h>
#include <fs/vfs.h>
#include <mm/vmm.h>
#include <stdint.h>

#define MAX_CHILDREN_PER_PROCESS 32
#define MAX_THREADS_PER_PROCESS 32
#define MAX_FILES_PER_PROCESS 32

#define MAX_PROCESSES 128
#define MAX_THREADS 1024

typedef struct process process;

typedef enum {
    READY,
    RUNNING,
    SLEEPING,
    EXITED,
} thread_state;

typedef struct {
    uint64_t fd;
    vnode *node;
    size_t offset;
} file;

typedef struct {
    process *parent;
    thread_state state;
    uint64_t tid;
    uintptr_t entry;
    uintptr_t kernel_stack;
    uintptr_t user_stack;
    cpu_context *context;
} thread;
typedef struct process {
    process *parent;
    uint64_t pid;
    page_table pt;
    file *files[MAX_FILES_PER_PROCESS];
    process *children[MAX_CHILDREN_PER_PROCESS];
    thread *threads[MAX_THREADS_PER_PROCESS];
} process;

process *process_new(process *parent, const char *path);
void process_spawn_thread(process *parent, uintptr_t entry);
void schedule(cpu_context *state);
void sched_init();
void timer_handler(cpu_context *state);
