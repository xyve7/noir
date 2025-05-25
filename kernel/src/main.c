#include "mm/heap.h"
#include <dev/serial.h>
#include <kernel.h>
#include <lib/printf.h>
#include <lib/string.h>
#include <limine.h>
#include <limits.h>
#include <mm/pmm.h>
#include <mm/vmm.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/acpi.h>
#include <sys/idt.h>
#include <term/term.h>
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
)) static volatile struct limine_framebuffer_request
    framebuffer_request = {.id = LIMINE_FRAMEBUFFER_REQUEST, .revision = 0};

__attribute__((used, section(".limine_requests"))) volatile struct limine_hhdm_request hhdm_request = {
    .id = LIMINE_HHDM_REQUEST,
    .response = 0
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
void log(const char *file, const char *func, uint32_t line, const char *restrict format, ...) {
    va_list serial, print;
    va_start(serial, format);
    va_start(print, format);

    serial_printf("%s:%s:%u: ", file, func, line);
    serial_vprintf(format, serial);

    printf("%s:%s:%u: ", file, func, line);

    vprintf(format, print);

    va_end(serial);
    va_end(print);
}
// Set up the terminal so we can render text
term_ctx term;
void write_char(char ch) { term_write_char(&term, ch); }

void heap_tests(void) {
    LOG("Heap tests starting...\n");

    // Test 1: Simple allocation/free
    void *ptr = kmalloc(64);
    if (!ptr) {
        LOG("FAIL: kmalloc(64) returned NULL\n");
    } else {
        LOG("PASS: kmalloc(64) returned %p\n", ptr);
        kfree(ptr);
        LOG("Freed memory at %p\n", ptr);
    }

    // Test 2: Multiple allocations/free
    void *ptrs[5];
    int i;
    int all_allocated = 1;
    for (i = 0; i < 5; i++) {
        ptrs[i] = kmalloc(32 * (i + 1));
        if (!ptrs[i]) {
            LOG("FAIL: kmalloc(%d) returned NULL\n", 32 * (i + 1));
            all_allocated = 0;
            break;
        } else {
            LOG("Allocated ptr[%d] = %p\n", i, ptrs[i]);
        }
    }

    if (all_allocated) {
        for (i = 0; i < 5; i++) {
            kfree(ptrs[i]);
            LOG("Freed ptr[%d] = %p\n", i, ptrs[i]);
        }
        LOG("PASS: Multiple allocations and frees succeeded\n");
    } else {
        // Free previously allocated ptrs in case of failure
        for (int j = 0; j < i; j++) {
            kfree(ptrs[j]);
            LOG("Freed ptr[%d] = %p\n", j, ptrs[j]);
        }
    }

    // Test 3: Allocation size zero
    ptr = kmalloc(0);
    if (ptr != NULL) {
        LOG("Warning: kmalloc(0) should usually return NULL or minimum size. Got %p\n", ptr);
        kfree(ptr);
    } else {
        LOG("PASS: kmalloc(0) returned NULL\n");
    }

    // Test 4: Free NULL pointer
    kfree(NULL); // should safely do nothing or handle gracefully
    LOG("PASS: kfree(NULL) did not crash\n");
}

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
    struct limine_framebuffer *framebuffer =
        framebuffer_request.response->framebuffers[0];
    term = term_new(framebuffer);

    serial_init();
    idt_init();
    pmm_init();
    vmm_init();
    acpi_init();
    acpi_list_tables();
    heap_init();
    heap_tests();
    heap_status();

    // We're done, just hang...
    hcf();
}
