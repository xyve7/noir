#pragma once
#include <stdint.h>

// Spinlock
typedef int spinlock;

// Spinlock initial value
#define SPINLOCK_INIT (0)

// Acquire spinlock
static inline void spinlock_acquire(spinlock *lock) {
    while (__atomic_exchange_n(lock, 1, __ATOMIC_ACQUIRE)) {
        __builtin_ia32_pause();
    }
}

// Release spinlock
static inline void spinlock_release(spinlock *lock) {
    __atomic_store_n(lock, 0, __ATOMIC_RELEASE);
}
