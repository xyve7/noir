#include <fs/vfs.h>
#include <kernel.h>
#include <mm/heap.h>
#include <stdint.h>
#include <sys/smp.h>
#include <sys/syscall.h>
#include <task/sched.h>

extern error close_fd(process *current_process, uint64_t fd);

error sys_exit(syscall_state *state) {
    uint64_t ret = ARG0(state);

    cpu *current_cpu = cpu_get();
    process *current_process = current_cpu->current_thread->parent;

    for (uint64_t fd = 0; fd < MAX_FILES_PER_PROCESS; fd++) {
        if (current_process->files[fd]) {
            // We ignore the error message
            close_fd(current_process, fd);
        }
    }

    current_cpu->current_thread->state = EXITED;
    schedule(nullptr);
    state->rax = ret;
    return OK;
}
