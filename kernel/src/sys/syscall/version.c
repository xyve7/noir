#include <fs/vfs.h>
#include <kernel.h>
#include <lib/string.h>
#include <mm/heap.h>
#include <stdint.h>
#include <sys/smp.h>
#include <sys/syscall.h>
#include <task/sched.h>

typedef struct {
    uint8_t patch, minor, major;
    char name[8];
    char revision[8];
    char timestamp[128];
    char build[32];
} noir_version;

error sys_version(syscall_context *state) {
    noir_version *nv = (void *)ARG0(state);

    nv->patch = kernel_patch;
    nv->minor = kernel_minor;
    nv->major = kernel_major;
    strcpy(nv->name, kernel_name);
    strcpy(nv->revision, kernel_revision);
    strcpy(nv->timestamp, kernel_timestamp);
    strcpy(nv->build, kernel_build);

    return OK;
}
