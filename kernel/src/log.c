#include <dev/serial.h>
#include <kernel.h>
#include <lib/spinlock.h>

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

    bool state = spinlock_acquire_irq_save(&log_lock);

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

    spinlock_release_irq_restore(&log_lock, state);

    va_end(serial);
    va_end(print);
}
