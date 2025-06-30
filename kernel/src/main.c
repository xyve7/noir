#include <cpu/cpu.h>
#include <dev/keyboard.h>
#include <dev/serial.h>
#include <fs/console.h>
#include <fs/devfs.h>
#include <fs/framebuffer.h>
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
#include <sys/idt.h>
#include <sys/ioapic.h>
#include <sys/irq.h>
#include <sys/lapic.h>
#include <sys/madt.h>
#include <sys/pit.h>
#include <sys/proc.h>
#include <sys/ps2.h>
#include <sys/syscall.h>
#include <terminal/terminal.h>

// Set the base revision to 3, this is recommended as this is the latest
// base revision described by the Limine boot protocol specification.
// See specification for further info.

__attribute__((
    used, section(".limine_requests")
)) static volatile LIMINE_BASE_REVISION(3);

// The Limine requests can be placed anywhere, but it is important that
// the compiler does not optimise them away, so, usually, they should
// be made volatile or equivalent, _and_ they should be accessed at least
// once or marked as used with the "used" attribute as done here.

__attribute__((
    used,
    section(
        ".limine_requests"
    )
)) static volatile struct limine_framebuffer_request framebuffer_request = {.id = LIMINE_FRAMEBUFFER_REQUEST, .revision = 0};

__attribute__((used, section(".limine_requests"))) volatile struct limine_hhdm_request hhdm_request = {
    .id = LIMINE_HHDM_REQUEST,
    .revision = 0
};

__attribute__((used, section(".limine_requests"))) volatile struct limine_module_request module_request = {
    .id = LIMINE_MODULE_REQUEST,
    .revision = 0
};
// Finally, define the start and end markers for the Limine requests.
// These can also be moved anywhere, to any .c file, as seen fit.

__attribute__((used, section(".limine_requests_start"))) static volatile LIMINE_REQUESTS_START_MARKER;

__attribute__((
    used,
    section(
        ".limine_requests_end"
    )
)) static volatile LIMINE_REQUESTS_END_MARKER;

// Halt and catch fire function.
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
int mubsan_log(const char *format, ...) {
    va_list serial, print;
    va_start(serial, format);
    va_start(print, format);

    int written = serial_vprintf(format, serial);
    vprintf(format, print);

    va_end(serial);
    va_end(print);
    return written;
}

const char *log_name[] = {
    "INFO  ", "WARN  ", "DEBUG ", "PANIC "
};

spinlock log_lock = SPINLOCK_INIT;
void log(int kind, const char *file, const char *func, uint32_t line, const char *restrict format, ...) {
    va_list serial, print;
    va_start(serial, format);
    va_start(print, format);

    bool state = int_get_state();
    int_disable();
    spinlock_acquire(&log_lock);

    serial_printf("\033[36m%s\033[39m", log_name[kind]);
    if (kind == 2) {
        serial_printf("%s:%s:%u: ", file, func, line);
    }
    serial_vprintf(format, serial);
    serial_write('\n');

    printf("\033[36m%s\033[39m", log_name[kind]);
    if (kind == 2) {
        printf("%s:%s:%u: ", file, func, line);
    }

    vprintf(format, print);
    write_char('\n');

    spinlock_release(&log_lock);
    int_set_state(state);

    va_end(serial);
    va_end(print);
}
// Set up the terminal so we can render text
void write_char(char ch) {
    terminal_write((uint8_t)ch);
}

bool proc_enable = false;
void timer_handler(cpu_context *frame) {
    if (proc_enable) {
        proc_switch(frame);
    }
}

typedef struct {
    vnode node;
    void *memory;
    size_t size;
} mem_node;

error mem_read(vnode *node, void *buffer, size_t offset, size_t size);

vnode_ops mem_ops = {
    .read = mem_read
};

error mem_read(vnode *node, void *buffer, size_t offset, size_t size) {
    mem_node *n = (mem_node *)node;
    memcpy(buffer, n->memory + offset, size);
    return OK;
}

void load_proc() {
    // Load the process
    const char *process = "/home/user/admin/hello";

    LOG("Loading Process: process=%s", process);

    vnode *prog;
    vfs_open(process, 0, &prog);

    vinfo info;
    vfs_info(process, &info);

    void *buffer = kmalloc(info.size);
    size_t buffer_size = info.size;

    vfs_read(prog, buffer, 0, buffer_size);

    proc_elf(buffer, buffer_size);

    LOG("Loaded Process");
    LOG("Scheduler Enabled, Enqueuing First Process...");

    proc_enable = true;
}
struct limine_framebuffer *framebuffer;
// The following will be our kernel's entry point.
// If renaming kmain() to something else, make sure to change the
// linker script accordingly.
void kmain(void) {
    // Ensure the bootloader actually understands our base revision (see spec).
    if (LIMINE_BASE_REVISION_SUPPORTED == false) {
        hcf();
    }

    // Ensure we got a framebuffer.
    if (framebuffer_request.response == NULL ||
        framebuffer_request.response->framebuffer_count < 1) {
        hcf();
    }

    // Fetch the first framebuffer.
    // Set up the terminal
    framebuffer = framebuffer_request.response->framebuffers[0];
    terminal_init(framebuffer->address, framebuffer->width, framebuffer->height, framebuffer->pitch);

    serial_init();
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

    void *tar = module_request.response->modules[0]->address;
    size_t size = module_request.response->modules[0]->size;

    mem_node *tar_buf_root = kmalloc(sizeof(mem_node));
    tar_buf_root->node.ops = mem_ops;
    tar_buf_root->memory = tar;
    tar_buf_root->size = size;

    vfs_init();
    tarfs_init();
    vfs_mount_raw((vnode *)tar_buf_root, "/", "tarfs");

    // Initialize devices
    devfs_init();
    console_init();
    framebuffer_init();

    syscall_init();
    proc_init();

    while (true) {
        asm("sti");
    }
}
