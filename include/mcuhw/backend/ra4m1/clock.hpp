#pragma once

#include <cstddef>
#include <cstdint>
#include <expected>

#include <mcuhw/arch/barrier.hpp>
#include <mcuhw/core/mmio.hpp>

namespace mcuhw::backend::ra4m1 {

enum class clock_error : std::uint8_t {
    incompatible_hoco_frequency,
    stabilization_timeout,
    transition_timeout,
    verification_failed,
    mmio,
};

// Establishes the UNO R4 Minima clock contract: 48 MHz HOCO, 48 MHz ICLK,
// 48 MHz PCLKA/C/D, 24 MHz PCLKB/FCLK, and one flash wait state. The board's
// option-setting memory selects the 48 MHz HOCO trim; this routine verifies it
// rather than rewriting option memory.
template<typename Policy>
class clock_backend {
public:
    explicit constexpr clock_backend(Policy& policy) noexcept : policy_(policy) {}

    // @req REQ-0003 REQ-0009 REQ-0031
    [[nodiscard]] std::expected<void, clock_error>
    configure_hoco_48mhz(std::size_t poll_limit = 100'000U) noexcept {
        constexpr std::uintptr_t base = 0x4001'E000U;
        constexpr std::uintptr_t prcr = base + 0x3FEU;

        const auto hoco_configuration =
            policy_.template read<std::uint8_t>(base + 0x37U);
        if (!hoco_configuration) {
            return mmio_failure();
        }
        if ((*hoco_configuration & 0x38U) != 0x20U) {
            return std::unexpected(clock_error::incompatible_hoco_frequency);
        }

        if (!write<std::uint16_t>(prcr, 0xA503U)) {
            return mmio_failure();
        }

        if (!write<std::uint8_t>(base + 0x36U, 0x00U)) {
            best_effort_relock(prcr);
            return mmio_failure();
        }
        const auto oscillator = poll_mask(base + 0x3CU, 0x01U, poll_limit);
        if (!oscillator) {
            best_effort_relock(prcr);
            return oscillator;
        }

        // MEMWAIT may only be set in high-speed mode. The bootloader normally
        // already selected this mode, but writing zero makes ownership explicit.
        if (!write<std::uint8_t>(base + 0xA0U, 0x00U)) {
            best_effort_relock(prcr);
            return mmio_failure();
        }
        const auto power_mode = poll_clear(base + 0xA0U, 0x10U, poll_limit);
        if (!power_mode) {
            best_effort_relock(prcr);
            return power_mode;
        }

        constexpr std::uint32_t divisions = 0x1001'0100U;
        if (!write<std::uint8_t>(base + 0x31U, 0x01U)
            || !write<std::uint32_t>(base + 0x20U, divisions)
            || !write<std::uint8_t>(base + 0x26U, 0x00U)) {
            best_effort_relock(prcr);
            return mmio_failure();
        }
        arch::data_synchronization_barrier();
        arch::instruction_barrier();

        const auto source = policy_.template read<std::uint8_t>(base + 0x26U);
        const auto divider = policy_.template read<std::uint32_t>(base + 0x20U);
        const auto wait = policy_.template read<std::uint8_t>(base + 0x31U);
        const bool relocked = write<std::uint16_t>(prcr, 0xA500U);
        arch::data_memory_barrier();
        if (!source || !divider || !wait || !relocked) {
            return mmio_failure();
        }
        if (*source != 0U || *divider != divisions || *wait != 1U) {
            return std::unexpected(clock_error::verification_failed);
        }
        return {};
    }

private:
    template<typename Value>
    [[nodiscard]] bool write(std::uintptr_t address, Value value) noexcept {
        return policy_.template write<Value>(address, value).has_value();
    }

    [[nodiscard]] std::expected<void, clock_error>
    poll_mask(std::uintptr_t address, std::uint8_t mask,
              std::size_t poll_limit) noexcept {
        for (std::size_t attempt = 0; attempt < poll_limit; ++attempt) {
            const auto value = policy_.template read<std::uint8_t>(address);
            if (!value) {
                return mmio_failure();
            }
            if ((*value & mask) == mask) {
                return {};
            }
        }
        return std::unexpected(clock_error::stabilization_timeout);
    }

    [[nodiscard]] std::expected<void, clock_error>
    poll_clear(std::uintptr_t address, std::uint8_t mask,
               std::size_t poll_limit) noexcept {
        for (std::size_t attempt = 0; attempt < poll_limit; ++attempt) {
            const auto value = policy_.template read<std::uint8_t>(address);
            if (!value) {
                return mmio_failure();
            }
            if ((*value & mask) == 0U) {
                return {};
            }
        }
        return std::unexpected(clock_error::transition_timeout);
    }

    void best_effort_relock(std::uintptr_t prcr) noexcept {
        static_cast<void>(policy_.template write<std::uint16_t>(prcr, 0xA500U));
        arch::data_memory_barrier();
    }

    [[nodiscard]] static std::expected<void, clock_error>
    mmio_failure() noexcept {
        return std::unexpected(clock_error::mmio);
    }

    Policy& policy_;
};

} // namespace mcuhw::backend::ra4m1
