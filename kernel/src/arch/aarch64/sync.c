#include <arch/aarch64/sync.h>
#include <kernel.h>

void aarch64_sync_handler(AARCH64State *state) {
    PANIC(
        "Exception has occurred!"
    );
}
