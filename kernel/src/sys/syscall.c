

#include "cpu/cpu.h"
#include "kernel.h"
#include "lib/spinlock.h"
#include "lib/vec.h"
#include "mm/heap.h"
#include "sys/proc.h"
#include "sys/smp.h"
#include <fs/vfs.h>
#include <lib/string.h>
#include <stdint.h>
#include <sys/irq.h>
#include <sys/syscall.h>

typedef void (*syscall_entry)(cpu_context *);

void syscall_write(cpu_context *frame) {
    uint64_t fd = frame->rdi;
    const void *buffer = (const void *)frame->rsi;
    size_t bytes = frame->rdx;

    proc_file *file = vec_at(&cpu_get()->current_proc->files, fd);
    if (file == nullptr) {
        frame->rax = NO_ENTRY;
        return;
    }

    error ret = vfs_write(file->node, buffer, file->offset, bytes);
    frame->rax = ret;
}
void syscall_read(cpu_context *frame) {
    uint64_t fd = frame->rdi;
    void *buffer = (void *)frame->rsi;
    size_t bytes = frame->rdx;

    proc_file *file = vec_at(&cpu_get()->current_proc->files, fd);
    if (file == nullptr) {
        frame->rax = NO_ENTRY;
        return;
    }

    error ret = vfs_read(file->node, buffer, file->offset, bytes);
    frame->rax = ret;
}
void syscall_close(cpu_context *frame) {
    uint64_t fd = frame->rdi;

    proc_file *file = vec_at(&cpu_get()->current_proc->files, fd);
    if (file == nullptr) {
        frame->rax = NO_ENTRY;
        return;
    }

    error ret = vfs_close(file->node);
    frame->rax = ret;
}
void syscall_open(cpu_context *frame) {
    const char *path = (const char *)frame->rdi;
    uint64_t *fd = (uint64_t *)frame->rsi;

    vnode *node;
    error ret = vfs_open(path, 0, &node);

    proc_file *pc = kmalloc(sizeof(proc_file));
    pc->node = node;
    vec_push(&cpu_get()->current_proc->files, pc);

    *fd = cpu_get()->current_proc->files.len - 1;

    frame->rax = ret;
}
void syscall_map(cpu_context *frame) {
    uint64_t fd = frame->rdi;
    size_t offset = frame->rsi;
    size_t size = frame->rdx;
    void **where = (void **)frame->rcx;

    proc_file *file = vec_at(&cpu_get()->current_proc->files, fd);
    if (file == nullptr) {
        frame->rax = NO_ENTRY;
        return;
    }

    error ret = vfs_map(file->node, offset, size, where);
    frame->rax = ret;
}
void syscall_exit(cpu_context *frame) {
    proc_exit(cpu_get()->current_proc);
    proc_switch(frame);
    frame->rax = OK;
}

syscall_entry syscall_table[] = {
    [SYS_OPEN] = syscall_open,
    [SYS_WRITE] = syscall_write,
    [SYS_READ] = syscall_read,
    [SYS_CLOSE] = syscall_close,
    [SYS_EXIT] = syscall_exit,
    [SYS_MAP] = syscall_map
};

spinlock syscall_lock = SPINLOCK_INIT;
void syscall_handler(cpu_context *frame) {
    spinlock_acquire(&syscall_lock);
    syscall_table[frame->rax](frame);
    spinlock_release(&syscall_lock);
}

void syscall_init() {
    irq_register_handler(0x80, syscall_handler);
    LOG("Syscalls Initialized");
}
