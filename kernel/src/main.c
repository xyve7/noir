#include <cpu/cpu.h>
#include <dev/keyboard.h>
#include <dev/serial.h>
#include <fs/console.h>
#include <fs/devfs.h>
#include <fs/framebuffer.h>
#include <fs/ramdisk.h>
#include <fs/tarfs.h>
#include <fs/vfs.h>
#include <kernel.h>
#include <lib/printf.h>
#include <lib/spinlock.h>
#include <lib/string.h>
#include <limine.h>
#include <limits.h>
#include <mm/heap.h>
#include <mm/pmm.h>
#include <mm/vmm.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/acpi.h>
#include <sys/except.h>
#include <sys/gdt.h>
#include <sys/idt.h>
#include <sys/ioapic.h>
#include <sys/irq.h>
#include <sys/lapic.h>
#include <sys/madt.h>
#include <sys/pit.h>
#include <sys/ps2.h>
#include <sys/smp.h>
#include <sys/syscall.h>
#include <task/sched.h>
#include <terminal/terminal.h>

__attribute__((used, section(".limine_requests"))) static volatile LIMINE_BASE_REVISION(3);

__attribute__((used, section(".limine_requests"))) volatile struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST,
    .revision = 0
};

__attribute__((used, section(".limine_requests"))) volatile struct limine_hhdm_request hhdm_request = {
    .id = LIMINE_HHDM_REQUEST,
    .revision = 0
};

__attribute__((used, section(".limine_requests"))) volatile struct limine_module_request module_request = {
    .id = LIMINE_MODULE_REQUEST,
    .revision = 0
};

__attribute__((used, section(".limine_requests_start"))) static volatile LIMINE_REQUESTS_START_MARKER;
__attribute__((used, section(".limine_requests_end"))) static volatile LIMINE_REQUESTS_END_MARKER;

struct limine_framebuffer *framebuffer;

void kmain(void) {
    if (LIMINE_BASE_REVISION_SUPPORTED == false) {
        hcf();
    }
    if (framebuffer_request.response == NULL ||
        framebuffer_request.response->framebuffer_count < 1) {
        hcf();
    }

    // Fetch the first framebuffer.
    // Set up the terminal
    framebuffer = framebuffer_request.response->framebuffers[0];
    terminal_init(framebuffer->address, framebuffer->width, framebuffer->height, framebuffer->pitch);

    serial_init();
    gdt_init();
    idt_init();
    except_init();
    irq_init();
    irq_register_handler(32, timer_handler);

    pmm_init();
    vmm_init();

    heap_init();

    acpi_init();
    madt_init();

    lapic_init();
    lapic_enable();
    ioapic_init();
    lapic_timer_enable();

    ps2_init();
    keyboard_init();

    vfs_init();
    tarfs_init();
    ramdisk_init("tarfs");

    devfs_init();
    console_init();

    syscall_init();

    cpu *c = heap_alloc(sizeof(cpu));
    memset(c, 0, sizeof(cpu));
    cpu_set(c);

    sched_init();

    process_new(nullptr, "/init");

    while (true) {
        asm("sti");
    }
}
