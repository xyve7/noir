#include <fs/vfs.h>
#include <kernel.h>
#include <stdint.h>
#include <sys/smp.h>
#include <sys/syscall.h>
#include <task/sched.h>

error sys_get_pid(syscall_context *state) {
    uint64_t *out_pid = (void *)ARG0(state);

    cpu *current_cpu = cpu_get();
    process *current_process = current_cpu->current_thread->parent;
    *out_pid = current_process->pid;

    return OK;
}
