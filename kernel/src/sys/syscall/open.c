#include <fs/vfs.h>
#include <kernel.h>
#include <mm/heap.h>
#include <stdint.h>
#include <sys/smp.h>
#include <sys/syscall.h>
#include <task/sched.h>

uint64_t next_fd(process *p) {
    for (uint64_t i = 0; i < MAX_PROCESSES; i++) {
        if (p->files[i] == nullptr) {
            return i;
        }
    }
    PANIC("Too many open files\n");
    UNREACHABLE();
}
error sys_open(syscall_context *state) {
    const char *path = (const char *)ARG0(state);
    vflags flags = ARG1(state);
    uint64_t *out_fd = (uint64_t *)ARG2(state);

    vnode *file_node;
    error err = vfs_open(path, flags, &file_node);
    if (err != OK) {
        return err;
    }

    cpu *current_cpu = cpu_get();
    process *current_process = current_cpu->current_thread->parent;
    uint64_t fd = next_fd(current_process);

    current_process->files[fd] = heap_alloc(sizeof(file));
    current_process->files[fd]->fd = fd;
    current_process->files[fd]->node = file_node;
    current_process->files[fd]->offset = 0;

    *out_fd = fd;

    return OK;
}
