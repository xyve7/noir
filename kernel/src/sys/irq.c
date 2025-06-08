#include <dev/serial.h>
#include <kernel.h>
#include <lib/printf.h>
#include <sys/irq.h>
#include <sys/lapic.h>

static void (*irq_handlers[244])(stack_frame *);
void irq_init() {
    LOG("irq initialized\n");
}

void irq_register_handler(uint8_t vector, void (*handler)(stack_frame *)) {
    if (vector < 32) {
        PANIC("vector value is too low: %hhu\n", vector);
    }
    irq_handlers[vector - 32] = handler;
}
void irq_handler(stack_frame *frame) {
    if (irq_handlers[frame->no - 32]) {
        irq_handlers[frame->no - 32](frame);
    } else {
        printf("unhandled irq: %lu\n", frame->no);
        serial_printf("unhandled irq: %lu\n", frame->no);
        hcf();
    }
    lapic_eoi();
}
