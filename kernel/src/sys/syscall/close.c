#include <fs/vfs.h>
#include <kernel.h>
#include <mm/heap.h>
#include <stdint.h>
#include <sys/smp.h>
#include <sys/syscall.h>
#include <task/sched.h>

error close_fd(process *current_process, uint64_t fd) {
    file *f = current_process->files[fd];
    error err = vfs_close(f->node);
    if (err != OK) {
        return err;
    }

    heap_free(current_process->files[fd]);
    current_process->files[fd] = nullptr;
    return OK;
}

error sys_close(syscall_state *state) {
    uint64_t fd = ARG0(state);

    cpu *current_cpu = cpu_get();
    process *current_process = current_cpu->current_thread->parent;

    return close_fd(current_process, fd);
}
