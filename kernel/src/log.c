#include <kernel.h>
#include <kprintf.h>
#include <stdarg.h>
#include <stdint.h>

int mubsan_log(const char *format, ...) {
    va_list print;
    va_start(print, format);

    int written = kvprintf(format, print);

    va_end(print);
    return written;
}

void log(int kind, const char *file, const char *func, uint32_t line, const char *restrict format, ...) {
    va_list print;
    va_start(print, format);

#if defined(__x86_64__)
    kprintf("x86_64  ");
#elif defined(__aarch64__)
    kprintf("aarch64 ");
#else
    kprintf("unknown ");
#endif

    switch (kind) {
    case 0:
        kprintf("\033[36mINFO  \033[39m");
        break;
    case 1:
        kprintf("\033[33mWARN  \033[39m");
        break;
    case 2:
        kprintf("\033[35mDEBUG \033[39m");
        kprintf("%s:%s:%u: ", file, func, line);
        break;
    case 3:
        kprintf("\033[31mPANIC \033[39m");
        break;
    }

    kvprintf(format, print);
    write_char('\n');

    va_end(print);
}
