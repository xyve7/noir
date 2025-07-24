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

spinlock log_lock = SPINLOCK_INIT;
void log(int kind, const char *file, const char *func, uint32_t line, const char *restrict format, ...) {
    va_list serial, print;
    va_start(serial, format);
    va_start(print, format);

    bool state = spinlock_acquire_irq_save(&log_lock);

    switch (kind) {
    case 0:
        serial_printf("\033[36mINFO  \033[39m");
        printf("\033[36mINFO  \033[39m");
        break;
    case 1:
        serial_printf("\033[33mWARN  \033[39m");
        printf("\033[33mWARN  \033[39m");
        break;
    case 2:
        serial_printf("\033[35mDEBUG \033[39m");
        serial_printf("%s:%s:%u: ", file, func, line);

        printf("\033[35mDEBUG \033[39m");
        printf("%s:%s:%u: ", file, func, line);
        break;
    case 3:
        serial_printf("\033[31mPANIC \033[39m");
        printf("\033[31mPANIC \033[39m");
        break;
    }
    serial_vprintf(format, serial);
    serial_write('\n');

    vprintf(format, print);
    write_char('\n');

    spinlock_release_irq_restore(&log_lock, state);

    va_end(serial);
    va_end(print);
}
