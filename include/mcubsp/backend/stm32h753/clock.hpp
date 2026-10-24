#pragma once

#include <cstdint>
#include <expected>

#include <mcubsp/core/mmio.hpp>
#include <mcubsp/mcu/stm32h753/system.hpp>

namespace mcubsp::backend::stm32h753 {

enum class clock_error : std::uint8_t {
    hsi_not_ready,
    unexpected_system_source,
    unexpected_divider,
    mmio,
};

// Verifies the conservative reset clock used by the first NUCLEO-H753ZI
// bring-up image. PLL and voltage-scale transitions are intentionally deferred
// until startup, GPIO and debugger control are independently proven.
template<typename Policy>
class clock_backend {
public:
    explicit constexpr clock_backend(Policy& policy) noexcept : policy_(policy) {}

    // @req REQ-0003 REQ-0009 REQ-0031 REQ-0033
    [[nodiscard]] std::expected<void, clock_error>
    verify_hsi_64mhz() noexcept {
        using namespace mcu::stm32h753;
        const auto control = policy_.template read<std::uint32_t>(rcc_base);
        const auto configuration =
            policy_.template read<std::uint32_t>(rcc_base + 0x10U);
        const auto domain =
            policy_.template read<std::uint32_t>(rcc_base + 0x18U);
        if (!control || !configuration || !domain) {
            return std::unexpected(clock_error::mmio);
        }
        constexpr std::uint32_t hsi_on_and_ready = (1U << 0U) | (1U << 2U);
        if ((*control & hsi_on_and_ready) != hsi_on_and_ready) {
            return std::unexpected(clock_error::hsi_not_ready);
        }
        if ((*configuration & (0x7U << 3U)) != 0U) {
            return std::unexpected(clock_error::unexpected_system_source);
        }
        constexpr std::uint32_t hsi_divider = 0x3U << 3U;
        if ((*control & hsi_divider) != 0U) {
            return std::unexpected(clock_error::unexpected_divider);
        }
        constexpr std::uint32_t d1cpre_and_hpre =
            (0xFU << 8U) | (0xFU << 0U);
        if ((*domain & d1cpre_and_hpre) != 0U) {
            return std::unexpected(clock_error::unexpected_divider);
        }
        return {};
    }

private:
    Policy& policy_;
};

} // namespace mcubsp::backend::stm32h753
