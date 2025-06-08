#include <cpu/cpu.h>

typedef struct {
    void *start;
    stack_frame state;
} proc;

void proc_enqueue(void *start);
void proc_switch(stack_frame *frame);
