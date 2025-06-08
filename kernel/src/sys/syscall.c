

#include <sys/irq.h>
#include <sys/syscall.h>

typedef status (*syscall_entry)(stack_frame *);

status syscall_write(stack_frame *frame) {
}

syscall_entry syscall_table[] = {
    [SYS_WRITE] = syscall_write
};

void syscall_handler(stack_frame *frame) {
}

void syscall_init() {
    irq_register_handler(0x80, syscall_handler);
}
