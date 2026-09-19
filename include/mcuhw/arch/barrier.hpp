#pragma once

#include <atomic>

namespace mcuhw::arch {

// @req REQ-0009
inline void data_memory_barrier() noexcept {
#if defined(__arm__) || defined(__thumb__)
    __asm__ volatile("dmb sy" ::: "memory");
#elif defined(__riscv)
    __asm__ volatile("fence iorw, iorw" ::: "memory");
#else
    std::atomic_thread_fence(std::memory_order_seq_cst);
#endif
}

// @req REQ-0009
inline void data_synchronization_barrier() noexcept {
#if defined(__arm__) || defined(__thumb__)
    __asm__ volatile("dsb sy" ::: "memory");
#elif defined(__riscv)
    __asm__ volatile("fence iorw, iorw" ::: "memory");
#else
    std::atomic_thread_fence(std::memory_order_seq_cst);
#endif
}

// @req REQ-0009
inline void instruction_barrier() noexcept {
#if defined(__arm__) || defined(__thumb__)
    __asm__ volatile("isb sy" ::: "memory");
#elif defined(__riscv)
    __asm__ volatile("fence.i" ::: "memory");
#else
    std::atomic_signal_fence(std::memory_order_seq_cst);
#endif
}

} // namespace mcuhw::arch
