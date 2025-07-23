#pragma once
#include <cpu/cpu.h>
#include <stdint.h>

// Spinlock
typedef volatile int spinlock;

// Spinlock initial value
#define SPINLOCK_INIT (0)

// Acquire spinlock
static inline void spinlock_acquire(spinlock *lock) {
    while (__atomic_exchange_n(lock, 1, __ATOMIC_ACQUIRE)) {
        __builtin_ia32_pause();
    }
}

// Acquire spinlock and save IRQ
static inline bool spinlock_acquire_irq_save(spinlock *lock) {
    bool state = int_get_state();
    int_disable();
    spinlock_acquire(lock);
    return state;
}

// Release spinlock
static inline void spinlock_release(spinlock *lock) {
    __atomic_store_n(lock, 0, __ATOMIC_RELEASE);
}

// Release spinlock and restore IRQ
static inline void spinlock_release_irq_restore(spinlock *lock, bool state) {
    int_set_state(state);
    spinlock_release(lock);
}
