#include <lib/spinlock.h>
#include <mm/pmm.h>
#include <sys/proc.h>
proc procs[8];
proc *proc_current = nullptr;

size_t proc_cap = 8;
size_t proc_len = 0;
size_t proc_index = 0;

bool first = true;

spinlock proc_lock = SPINLOCK_INIT;

void proc_enqueue(void *start) {
    proc new_proc;
    new_proc.start = start;

    void *stack = pmm_allocz(1);
    new_proc.state.rip = (uint64_t)start;
    new_proc.state.rbp = (uint64_t)(VIRT(stack) + PAGE_SIZE - 1);
    new_proc.state.rsp = (uint64_t)(VIRT(stack));

    // Make sure interrupts are enabled
    new_proc.state.rflags = 0x282;
    new_proc.state.cs = 0x28;
    new_proc.state.ss = 0x30;

    // Add it to the queue
    procs[proc_len] = new_proc;
    proc_len++;
}
void proc_switch(stack_frame *frame) {
    spinlock_acquire(&proc_lock);
    if (first) {
        // Get proc
        proc_current = &procs[proc_index];
        *frame = proc_current->state;

        // Keep in bounds
        proc_index++;
        proc_index = proc_index % proc_len;

        first = false;
    } else {
        // Store state
        proc_current->state = *frame;

        // Other proc
        // Keep in bounds
        proc_index++;
        proc_index = proc_index % proc_len;

        proc_current = &procs[proc_index];
        *frame = proc_current->state;
    }
    spinlock_release(&proc_lock);
}
