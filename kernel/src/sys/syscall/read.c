#include <fs/vfs.h>
#include <kernel.h>
#include <mm/heap.h>
#include <stdint.h>
#include <sys/smp.h>
#include <sys/syscall.h>
#include <task/sched.h>
error sys_read(syscall_state *state) {
    uint64_t fd = ARG0(state);
    void *buffer = (void *)ARG1(state);
    size_t size = ARG2(state);

    cpu *current_cpu = cpu_get();
    process *current_process = current_cpu->current_thread->parent;

    if (current_process->files[fd] == nullptr) {
        return BAD_FD;
    }

    file *f = current_process->files[fd];
    error err = vfs_read(f->node, buffer, f->offset, size);

    return err;
}
