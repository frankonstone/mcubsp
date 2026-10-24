#pragma once

#include <cstdint>

#include <mcubsp/execution/bare_metal_policy.hpp>

namespace mcubsp::io::detail {

// These clocks are for the fixed-frequency, single-core board startup profiles.
// A runtime that changes the CPU clock or enters a sleep state that stops the
// cycle counter must supply its own execution policy and monotonic clock.
template<std::uint32_t Frequency, bool UnlockDwt = false>
struct cortex_m_cycle_hooks {
    static constexpr std::uint32_t ticks_per_second = Frequency;

    static void begin() noexcept {
#if defined(__arm__) || defined(__thumb__)
        auto& demcr = *reinterpret_cast<volatile std::uint32_t*>(0xE000'EDFCU);
        auto& control = *reinterpret_cast<volatile std::uint32_t*>(0xE000'1000U);
        demcr = demcr | (1U << 24U);
        if constexpr (UnlockDwt) {
            *reinterpret_cast<volatile std::uint32_t*>(0xE000'1FB0U) =
                0xC5AC'CE55U;
        }
        // Do not reset CYCCNT: another application service may already use it.
        control = control | 1U;
        __asm__ volatile("dsb\n\tisb" ::: "memory");
#else
        static_assert(Frequency == 0U,
                      "Supply an execution policy when using this board on a host");
#endif
    }

    static std::uint32_t enter_critical() noexcept {
#if defined(__arm__) || defined(__thumb__)
        std::uint32_t previous;
        __asm__ volatile("mrs %0, primask\n\tcpsid i"
                         : "=r"(previous) :: "memory");
        return previous;
#else
        static_assert(Frequency == 0U, "Cortex-M execution requires an Arm target");
        return 0U;
#endif
    }

    static void leave_critical(std::uint32_t previous) noexcept {
#if defined(__arm__) || defined(__thumb__)
        __asm__ volatile("msr primask, %0" :: "r"(previous) : "memory");
#else
        static_cast<void>(previous);
        static_assert(Frequency == 0U, "Cortex-M execution requires an Arm target");
#endif
    }

    static execution::tick now() noexcept {
#if defined(__arm__) || defined(__thumb__)
        return *reinterpret_cast<volatile const std::uint32_t*>(0xE000'1004U);
#else
        static_assert(Frequency == 0U, "Cortex-M execution requires an Arm target");
        return 0U;
#endif
    }

    static bool in_isr() noexcept {
#if defined(__arm__) || defined(__thumb__)
        std::uint32_t exception;
        __asm__ volatile("mrs %0, ipsr" : "=r"(exception));
        return exception != 0U;
#else
        static_assert(Frequency == 0U, "Cortex-M execution requires an Arm target");
        return false;
#endif
    }

    static void poll() noexcept {
#if defined(__arm__) || defined(__thumb__)
        __asm__ volatile("nop");
#else
        static_assert(Frequency == 0U, "Cortex-M execution requires an Arm target");
#endif
    }

    static void notify() noexcept {
#if defined(__arm__) || defined(__thumb__)
        __asm__ volatile("sev" ::: "memory");
#else
        static_assert(Frequency == 0U, "Cortex-M execution requires an Arm target");
#endif
    }

    [[noreturn]] static void active_transfer_destroyed() noexcept {
        __builtin_trap();
    }
};

// ESP32-P4 CLIC machine interrupt level occupies bits 31:29 of mintstatus.
// Pre-v3 silicon uses CSR 0x346; production v3 uses the standard CSR 0xFB1.
// See ESP-IDF components/soc/esp32p4/include/soc/interrupt_reg.h and
// components/riscv/include/riscv/csr_clic.h (v6.1).
template<std::uint32_t Frequency, std::uint16_t MintstatusCsr>
struct esp32p4_cycle_hooks {
    static constexpr std::uint32_t ticks_per_second = Frequency;

    static void begin() noexcept {
#if !defined(__riscv)
        static_assert(Frequency == 0U,
                      "Supply an execution policy when using this board on a host");
#endif
    }

    static std::uint32_t enter_critical() noexcept {
#if defined(__riscv)
#if defined(__riscv_zcmp) && __riscv_zcmp
        // P4 requires an additional CLIC-threshold workaround when Zcmp is
        // enabled. Its SDK-specific startup binding must supply that policy.
        static_assert(Frequency == 0U,
                      "ESP32-P4 Zcmp builds require a custom execution policy");
#endif
        std::uint32_t previous;
        __asm__ volatile("csrrci %0, mstatus, 8"
                         : "=r"(previous) :: "memory");
        return previous;
#else
        static_assert(Frequency == 0U, "ESP32-P4 execution requires a RISC-V target");
        return 0U;
#endif
    }

    static void leave_critical(std::uint32_t previous) noexcept {
#if defined(__riscv)
        if ((previous & 8U) != 0U) {
            __asm__ volatile("csrsi mstatus, 8" ::: "memory");
        }
#else
        static_cast<void>(previous);
        static_assert(Frequency == 0U, "ESP32-P4 execution requires a RISC-V target");
#endif
    }

    static execution::tick now() noexcept {
#if defined(__riscv)
        std::uint32_t value;
        __asm__ volatile("rdcycle %0" : "=r"(value));
        return value;
#else
        static_assert(Frequency == 0U, "ESP32-P4 execution requires a RISC-V target");
        return 0U;
#endif
    }

    static bool in_isr() noexcept {
#if defined(__riscv)
        std::uint32_t value;
        __asm__ volatile("csrr %0, %1"
                         : "=r"(value) : "i"(MintstatusCsr));
        return (value >> 29U) != 0U;
#else
        static_assert(Frequency == 0U, "ESP32-P4 execution requires a RISC-V target");
        return false;
#endif
    }

    static void poll() noexcept {
#if defined(__riscv)
        __asm__ volatile("nop");
#else
        static_assert(Frequency == 0U, "ESP32-P4 execution requires a RISC-V target");
#endif
    }

    static void notify() noexcept {
#if defined(__riscv)
        __asm__ volatile("" ::: "memory");
#else
        static_assert(Frequency == 0U, "ESP32-P4 execution requires a RISC-V target");
#endif
    }

    [[noreturn]] static void active_transfer_destroyed() noexcept {
        __builtin_trap();
    }
};

template<typename Hooks>
struct target_execution : execution::bare_metal_policy<Hooks> {
    static constexpr std::uint32_t ticks_per_second = Hooks::ticks_per_second;
    void begin() noexcept { Hooks::begin(); }
};

} // namespace mcubsp::io::detail
