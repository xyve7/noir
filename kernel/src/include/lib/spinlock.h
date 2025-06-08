typedef int spinlock;

#define SPINLOCK_INIT (false)

static inline void spinlock_acquire(spinlock *lock) {
    while (__atomic_exchange_n(lock, 1, __ATOMIC_ACQUIRE)) {
        __builtin_ia32_pause();
    }
}

static inline void spinlock_release(spinlock *lock) {
    __atomic_store_n(lock, 0, __ATOMIC_RELEASE);
}
