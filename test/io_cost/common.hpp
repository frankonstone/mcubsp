#pragma once

#include <array>
#include <chrono>
#include <cstdint>
#include <utility>

#include <mcuhw/core/direct_mmio.hpp>
#include <mcuhw/execution/bare_metal_policy.hpp>
#include <mcuhw/io.hpp>
#include <mcuhw/io/board/uno_r4_minima.hpp>

extern "C" [[noreturn]] void mcuhw_io_cost_trap() noexcept;

namespace cost {
namespace io = mcuhw::io;
using board_type = io::boards::uno_r4_minima;
using mmio_type = mcuhw::core::direct_mmio;

// Both sides use the same interrupt masking, DWT time, and bounded polling.
// The driver side supplies progress explicitly; io supplies it through its
// execution adapter. There is no scheduler or simulated peripheral involved.
void progress() noexcept;
struct hooks {
    [[nodiscard]] static std::uint32_t enter_critical() noexcept {
        std::uint32_t saved;
        __asm__ volatile("mrs %0, primask\n\tcpsid i" : "=r"(saved) :: "memory");
        return saved;
    }
    static void leave_critical(std::uint32_t saved) noexcept {
        __asm__ volatile("msr primask, %0" :: "r"(saved) : "memory");
    }
    [[nodiscard]] static mcuhw::execution::tick now() noexcept {
        return *reinterpret_cast<volatile std::uint32_t*>(0xE000'1004U);
    }
    [[nodiscard]] static bool in_isr() noexcept {
        std::uint32_t value;
        __asm__ volatile("mrs %0, ipsr" : "=r"(value));
        return value != 0U;
    }
    static void poll() noexcept { progress(); }
    static void notify() noexcept {}
    [[noreturn]] static void active_transfer_destroyed() noexcept {
        mcuhw_io_cost_trap();
    }
};
using execution_type = mcuhw::execution::bare_metal_policy<hooks>;
inline execution_type execution;
inline mmio_type mmio;
inline volatile std::uint32_t observation{};

inline void check(bool success) noexcept {
    if (!success) mcuhw_io_cost_trap();
}

// Kept out of line on both sides so asynchronous work cannot be optimized
// into an empty expression, or the complete operation into an unused result.
[[gnu::noinline]] inline void other_work() noexcept {
    observation = observation + 1U;
}

template<typename Transfer>
void finish_driver(Transfer& transfer, mcuhw::execution::deadline limit) noexcept {
    const auto waited = transfer.wait_until(limit);
    const auto result = transfer.take();
    observation = waited.has_value() && result.has_value();
}
template<typename Transfer>
void finish_driver(Transfer& transfer) noexcept {
    finish_driver(transfer, mcuhw::execution::deadline{
        static_cast<mcuhw::execution::tick>(execution.now() + 4'800'000U)});
}
} // namespace cost
