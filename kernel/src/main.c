#include <arch/aarch64/cpu.h>
#include <arch/aarch64/except.h>
#include <arch/x86_64/gdt.h>
#include <arch/x86_64/idt.h>
#include <kprintf.h>
#include <limine.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <terminal/terminal.h>

__attribute__((used, section(".limine_requests"))) static volatile uint64_t limine_base_revision[] = LIMINE_BASE_REVISION(4);
__attribute__((used, section(".limine_requests_start"))) static volatile uint64_t limine_requests_start_marker[] = LIMINE_REQUESTS_START_MARKER;

__attribute__((used, section(".limine_requests"))) static volatile struct limine_hhdm_request hhdm_request = {
    .id = LIMINE_HHDM_REQUEST_ID,
    .revision = 0
};

__attribute__((used, section(".limine_requests_end"))) static volatile uint64_t limine_requests_end_marker[] = LIMINE_REQUESTS_END_MARKER;

void hcf(void) {
    for (;;) {
#if defined(__x86_64__)
        asm("hlt");
#elif defined(__aarch64__) || defined(__riscv)
        asm("wfi");
#elif defined(__loongarch64)
        asm("idle 0");
#endif
    }
}

void kmain(void) {
    if (LIMINE_BASE_REVISION_SUPPORTED(limine_base_revision) == false) {
        hcf();
    }

    terminal_init();

    // All the architecture specific stuff is done here
#if defined(__aarch64__)
    kprintf("Hello from ARM!\n");
    aarch64_except_init();
    asm volatile("brk #0");
#elif defined(__x86_64__)
    kprintf("Hello from x86_64!\n");
    x86_64_gdt_init();
    x86_64_idt_init();
    asm volatile("int $1");
#endif
    hcf();
}
