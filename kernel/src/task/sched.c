#include "cpu/cpu.h"
#include "fs/vfs.h"
#include "mm/pmm.h"
#include "mm/vmm.h"
#include "sys/gdt.h"
#include "sys/smp.h"
#include <elf.h>
#include <kernel.h>
#include <lib/string.h>
#include <mm/heap.h>
#include <stdint.h>
#include <task/sched.h>

process *process_table[MAX_PROCESSES];
thread *thread_queue[MAX_THREADS];
thread *thread_idle = nullptr;

uint64_t thread_count;
uint64_t current_thread;

constexpr uintptr_t user_stack_base = 0x00007F0000000000;
uintptr_t user_stack_current = user_stack_base;

[[gnu::noreturn]] void context_switch(uint64_t rsp);

uintptr_t new_user_stack(pagemap *process_pm) {
    void *user_stack = pmm_allocz(1);
    uintptr_t user_mapping = user_stack_current;
    vmm_map(process_pm, (uintptr_t)user_stack, user_mapping, VMM_PRESENT | VMM_WRITE | VMM_USER);
    user_stack_current += PAGE_SIZE;
    return user_mapping + PAGE_SIZE;
}
uintptr_t new_kernel_stack() {
    void *kernel_stack = pmm_allocz(4);
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
    thread *t = kmalloc(sizeof(thread));
    t->parent = parent;
    t->state = READY;
    t->tid = next_tid(parent);
    t->entry = entry;
    t->kernel_stack = new_kernel_stack();
    t->user_stack = new_user_stack(&parent->pm);
    t->context = (cpu_context *)(t->kernel_stack - sizeof(cpu_context));

    t->context->rip = entry;
    t->context->rsp = t->user_stack;
    t->context->rflags = 0x202;
    t->context->cs = USER_CS | 3;
    t->context->ss = USER_SS | 3;

    enqueue(t);
    return t;
}
uintptr_t load_elf(pagemap *pm, void *buffer) {
    // Read ehdr
    elf64_ehdr *e = buffer;

    if (!elf_is_elf(e)) {
        PANIC("Invalid ELF Magic");
    }
    if (!elf_is_valid(e)) {
        PANIC("Unsupported ELF");
    }

    uintptr_t entry = e->entry;

    // Read phdr
    elf64_phdr *p = buffer + e->phoff;

    for (size_t i = 0; i < e->phnum; i++) {
        uint64_t flags = 0;
        if (p[i].type != ELF_PTYPE_LOAD) {
            continue;
        }

        // We convert the ELF flags to VMM flags
        if ((p[i].flags & ELF_PFLAGS_EXEC) == 0) {
            flags |= VMM_XD;
        }
        if (p[i].flags & ELF_PFLAGS_WRITE) {
            flags |= VMM_WRITE;
        }
        flags |= VMM_PRESENT;
        flags |= VMM_USER;

        // We need to map it to the kernel first
        // This is so we can actually write to it
        // Technically its already mapped but, you know what I mean

        // We copy the page
        void *page = VIRT(pmm_allocz(1));
        memcpy(page, buffer + p[i].offset, p[i].memsz);

        // We map the page
        page = PHYS(page);
        void *virt_page = (void *)p[i].vaddr;

        vmm_map(pm, (uintptr_t)page, (uintptr_t)virt_page, flags);
    }
    return entry;
}

process *process_new(process *parent, const char *path) {
    // We create the new pagemap and map the elf to it
    pagemap new_process_pagemap = vmm_new();

    vinfo elf_info;
    vfs_info(path, &elf_info);

    void *buffer = kmalloc(elf_info.size);
    vnode *elf;
    vfs_open(path, VFS_READ, &elf);
    vfs_read(elf, buffer, 0, elf_info.size);
    vfs_close(elf);

    uintptr_t entry = load_elf(&new_process_pagemap, buffer);

    process *p = kmalloc(sizeof(process));
    p->parent = parent;
    p->pid = next_pid();
    p->pm = new_process_pagemap;
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
void sched_switch() {
    thread *t = next_thread();

    cpu *current_cpu = cpu_get();
    current_cpu->current_thread = t;
    current_cpu->kernel_stack = t->kernel_stack;
    current_cpu->user_stack = t->user_stack;

    if (t == thread_idle) {
        vmm_switch(&kernel_pagemap);
    } else {
        vmm_switch(&t->parent->pm);
    }
    gdt_set_rsp0(t->kernel_stack);
    context_switch((uint64_t)t->context);
}
// Save the current state and switch
void schedule(cpu_context *state) {
    // The current cpu state is passed
    // This is inside the current processes kernel stack

    cpu *current_cpu = cpu_get();
    if (current_cpu->current_thread) {
        // We save the state
        current_cpu->current_thread->context = state;
        current_cpu->kernel_stack = current_cpu->current_thread->kernel_stack;
        current_cpu->user_stack = current_cpu->current_thread->user_stack;
    }
    sched_switch();
}
void sched_idle() {
    while (1) {
        asm("pause");
    }
}
void sched_init() {
    thread *t = kmalloc(sizeof(thread));
    t->parent = nullptr;
    t->state = READY;
    t->tid = 0;
    t->entry = (uintptr_t)sched_idle;
    t->kernel_stack = new_kernel_stack();
    t->context = (cpu_context *)(t->kernel_stack - sizeof(cpu_context));

    t->context->rip = t->entry;
    t->context->rsp = t->kernel_stack;
    t->context->rflags = 0x202;
    t->context->cs = KERNEL_CS;
    t->context->ss = KERNEL_SS;

    thread_idle = t;
}
