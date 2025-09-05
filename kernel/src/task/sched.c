#include <cpu/cpu.h>
#include <elf.h>
#include <fs/vfs.h>
#include <kernel.h>
#include <lib/string.h>
#include <mm/heap.h>
#include <mm/pmm.h>
#include <mm/vmm.h>
#include <stdint.h>
#include <sys/gdt.h>
#include <sys/smp.h>
#include <task/loader.h>
#include <task/sched.h>

process *process_table[MAX_PROCESSES];
thread *thread_queue[MAX_THREADS];
thread *thread_idle = nullptr;

uint64_t thread_count;
uint64_t current_thread;

constexpr uintptr_t user_stack_base = 0x00007F0000000000;
uintptr_t user_stack_current = user_stack_base;

[[gnu::noreturn]] void context_restore(thread_context *ctx);
[[gnu::returns_twice]] uint64_t context_save(thread_context *ctx);

uintptr_t new_user_stack(page_table *process_pm) {
    void *user_stack = pmm_alloc_zeroed(1);
    uintptr_t user_mapping = user_stack_current;
    vmm_map(process_pm, (uintptr_t)user_stack, user_mapping, VMM_PRESENT | VMM_WRITE | VMM_USER);
    user_stack_current += PAGE_SIZE;
    return user_mapping + PAGE_SIZE;
}
uintptr_t new_kernel_stack() {
    void *kernel_stack = pmm_alloc_zeroed(4);
    return (uintptr_t)VIRT(kernel_stack) + (PAGE_SIZE * 4);
}

uint64_t next_pid() {
    for (uint64_t i = 0; i < MAX_PROCESSES; i++) {
        if (process_table[i] == nullptr) {
            return i;
        }
    }
    PANIC("Too many processes\n");
    UNREACHABLE();
}

uint64_t next_tid(process *p) {
    for (uint64_t i = 0; i < MAX_THREADS_PER_PROCESS; i++) {
        if (p->threads[i] == nullptr) {
            return i;
        }
    }
    PANIC("Too many threads\n");
    UNREACHABLE();
}

void enqueue(thread *t) {
    for (uint64_t i = 0; i < MAX_THREADS; i++) {
        if (thread_queue[i] == nullptr) {
            thread_queue[i] = t;
            thread_count++;
            return;
        }
    }
    PANIC("Too many threads\n");
    UNREACHABLE();
}

thread *thread_new(process *parent, uintptr_t entry) {
    thread *t = heap_alloc(sizeof(thread));
    t->parent = parent;
    t->state = READY;
    t->tid = next_tid(parent);
    t->entry = entry;
    t->kernel_stack = new_kernel_stack();
    t->user_stack = new_user_stack(&parent->pt);
    t->context = (thread_context *)(t->kernel_stack - sizeof(thread_context));

    t->context->rip = entry;
    t->context->rsp = t->user_stack;

    enqueue(t);
    return t;
}

process *process_new(process *parent, const char *path) {
    // We create the new page_table and map the elf to it
    page_table new_process_page_table = vmm_new();

    vinfo elf_info;
    vfs_info(path, &elf_info);

    void *buffer = heap_alloc(elf_info.size);
    vnode *elf;
    vfs_open(path, VFS_READ, &elf);
    vfs_read(elf, buffer, 0, elf_info.size);
    vfs_close(elf);

    uintptr_t entry = elf_load_exec(&new_process_page_table, buffer);

    process *p = heap_alloc(sizeof(process));
    p->parent = parent;
    p->pid = next_pid();
    p->pt = new_process_page_table;
    p->threads[0] = thread_new(p, entry);

    process_table[p->pid] = p;
    return p;
}
void process_spawn_thread(process *parent, uintptr_t entry) {
    uint64_t id = next_tid(parent);
    parent->threads[id] = thread_new(parent, entry);
}
thread *next_thread() {
    for (uint64_t i = 0; i < 32; i++) {
        uint64_t id = (current_thread + i) % 32;
        if (thread_queue[id] && thread_queue[id]->state == READY) {
            thread *t = thread_queue[id];
            current_thread = id + 1;
            return t;
        }
    }
    return thread_idle;
}
extern void usermode_enter(uint64_t rsp, uint64_t rip);
// Perform a context switch
void sched_switch(bool usermode) {
    thread *t = next_thread();

    cpu *current_cpu = cpu_get();
    current_cpu->current_thread = t;
    current_cpu->kernel_stack = t->kernel_stack;
    current_cpu->user_stack = t->user_stack;

    if (t == thread_idle) {
        vmm_switch(&kernel_page_table);
    } else {
        vmm_switch(&t->parent->pt);
    }
    gdt_set_rsp0(t->kernel_stack);
    if (usermode) {
        usermode_enter(t->context->rsp, t->context->rip);
    } else {
        context_restore(t->context);
    }
}
// Save the current state and switch
void sched_yield() {
    // The current cpu state is passed
    // This is inside the current processes kernel stack
    cpu *current_cpu = cpu_get();
    if (current_cpu->current_thread) {
        // We save the state
        current_cpu->kernel_stack = current_cpu->current_thread->kernel_stack;
        current_cpu->user_stack = current_cpu->current_thread->user_stack;
        if (context_save(current_cpu->current_thread->context) == 1) {
            // We returned from a context_restore, return
            return;
        }
        sched_switch(false);
    } else {
        sched_switch(true);
    }
}
void sched_idle() {
    while (1) {
        asm("pause");
    }
}
void sched_init() {
    thread *t = heap_alloc(sizeof(thread));
    t->parent = nullptr;
    t->state = READY;
    t->tid = 0;
    t->entry = (uintptr_t)sched_idle;
    t->kernel_stack = new_kernel_stack();
    t->context = (thread_context *)(t->kernel_stack - sizeof(thread_context));

    t->context->rip = t->entry;
    t->context->rsp = t->kernel_stack;

    thread_idle = t;
}

void timer_handler(int_context *state) {
    sched_yield();
}
