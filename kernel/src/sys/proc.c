#include "cpu/cpu.h"
#include "kernel.h"
#include "lib/vec.h"
#include "mm/vmm.h"
#include <elf.h>
#include <lib/printf.h>
#include <lib/spinlock.h>
#include <lib/string.h>
#include <mm/heap.h>
#include <mm/pmm.h>
#include <stdint.h>
#include <sys/proc.h>
#include <sys/smp.h>

spinlock proc_lock = SPINLOCK_INIT;
proc *procs;
size_t proc_cnt = 0;
proc *idle;
void proc_idle() {
    while (true) {
        asm("sti");
    }
}
proc *proc_new(void *start) {
    proc *enqueued = kmalloc(sizeof(proc));
    enqueued->start = start;
    enqueued->pm = vmm_pagemap_new();

    void *stack = pmm_allocz(1);
    vmm_map(&enqueued->pm, (uintptr_t)stack, (uintptr_t)VIRT(stack), VMM_PRESENT | VMM_WRITE);

    void *stack_top = VIRT(stack) + PAGE_SIZE;
    enqueued->context.rip = (uint64_t)start;
    enqueued->context.rbp = (uint64_t)stack_top;
    enqueued->context.rsp = (uint64_t)stack_top;

    // Make sure interrupts are enabled
    enqueued->context.rflags = 0x202;
    enqueued->context.cs = 0x28;
    enqueued->context.ss = 0x30;

    // Create open file vector
    enqueued->files = vec_new();

    return enqueued;
}
void proc_init() {
    idle = proc_new(proc_idle);
    idle->state = IDLE;
    LOG("Scheduler Initialized");
}
void proc_enqueue(void *start) {
    proc *enqueued = proc_new(start);
    // Set the process state
    enqueued->state = ENQUEUED;
    // Add the process
    enqueued->next = procs ? procs : nullptr;
    procs = enqueued;

    proc_cnt++;
}
void UNOPTIMIZED proc_elf(void *buffer, size_t size) {
    // Read ehdr
    Elf64_Ehdr *e = buffer;

    // Check if its 64 bit
    if (e->e_ident[EI_CLASS] != 2) {
        PANIC("elf is not 64 bit");
    }

    // Check endianness
    if (e->e_ident[EI_DATA] != 1) {
        PANIC("elf is not little endian");
    }

    // Check if its an executable
    if (e->e_type != ET_EXEC) {
        PANIC("elf is not executable");
    }

    // Check if x86
    if (e->e_machine != 0x3e) {
        PANIC("elf is not x86-64");
    }

    void *start = (void *)e->e_entry;

    proc *enqueued = proc_new(start);
    // Set the process state
    enqueued->state = ENQUEUED;

    // Read phdr
    Elf64_Phdr *p = buffer + e->e_phoff;

    for (size_t i = 0; i < e->e_phnum; i++) {
        uint64_t flags = 0;
        if (p[i].p_type != PT_LOAD) {
            continue;
        }

        // We convert the ELF flags to VMM flags
        if ((p[i].p_flags & PF_X) == 0) {
            flags |= VMM_XD;
        }
        if (p[i].p_flags & PF_W) {
            flags |= VMM_WRITE;
        }
        flags |= VMM_PRESENT;

        // We need to map it to the kernel first
        // This is so we can actually write to it
        // Technically its already mapped but, you know what I mean

        // We copy the page
        void *page = VIRT(pmm_allocz(1));
        memcpy(page, buffer + p[i].p_offset, p[i].p_memsz);

        // We map the page
        page = PHYS(page);
        void *virt_page = (void *)p[i].p_vaddr;
        vmm_map(&enqueued->pm, (uintptr_t)page, (uintptr_t)virt_page, flags);
        vmm_map(&kernel_pm, (uintptr_t)page, (uintptr_t)virt_page, flags);
    }

    // Add the process
    enqueued->next = procs ? procs : nullptr;
    procs = enqueued;

    proc_cnt++;
}
proc *proc_next(proc *current) {
    if (current == nullptr || current == idle) {
        current = procs;
    }
    proc *start = current;

    do {
        if (current == nullptr) {
            current = procs;
            continue;
        }
        if (current->state == ENQUEUED) {
            current->state = RUNNING;
            return current;
        }
        current = current->next;
    } while (current != start);
    return idle;
}
void proc_switch(cpu_context *frame) {
    spinlock_acquire(&proc_lock);

    // This is the process that we entered with
    proc *entry_proc = cpu_get()->current_proc;
    // We have a current process and it isnt idle
    if (entry_proc && entry_proc != idle) {
        // Save current state
        cpu_get()->current_proc->context = *frame;
        // Don't enqueue the task if its already existed
        if (entry_proc->state != EXITED) {
            entry_proc->state = ENQUEUED;
        }
    }
    // Next process
    cpu_get()->current_proc = proc_next(entry_proc);

    // printf("cpu: %u, cur: %p, ctx: %p\n", lapic_id(), cpu_get()->current_proc->start, (void *)frame->rip);

    // Load the context
    *frame = cpu_get()->current_proc->context;

    // Switch address space
    vmm_switch(&cpu_get()->current_proc->pm);
    spinlock_release(&proc_lock);
}
void proc_exit(proc *p) {
    p->state = EXITED;

    // FIXME: Close all file descriptors
}
