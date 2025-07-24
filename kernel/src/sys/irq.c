#include <dev/serial.h>
#include <kernel.h>
#include <lib/printf.h>
#include <sys/irq.h>
#include <sys/lapic.h>

static void (*irq_handlers[244])(int_context *);
void irq_init() {
    LOG("IRQ Initialized");
}

void irq_register_handler(uint8_t vector, void (*handler)(int_context *)) {
    if (vector < 32) {
        PANIC("vector value is too low: %hhu\n", vector);
    }
    irq_handlers[vector - 32] = handler;
    LOG("Registered IRQ Handler: handler=%p, vector=%hhu", handler, vector);
}
void irq_handler(int_context *frame) {
    if (irq_handlers[frame->no - 32]) {
        irq_handlers[frame->no - 32](frame);
    } else {
        printf("unhandled irq: %lu\n", frame->no);
        serial_printf("unhandled irq: %lu\n", frame->no);
    }
    lapic_eoi();
}
