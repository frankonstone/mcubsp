#pragma once

#include <array>
#include <cstdint>
#include <expected>

#include <mcubsp/arch/barrier.hpp>
#include <mcubsp/core/mmio.hpp>
#include <mcubsp/driver/gpio.hpp>
#include <mcubsp/mcu/ra4m1/pins.hpp>

namespace mcubsp::backend::ra4m1 {

// RA4M1 native GPIO sequencing and ownership. Claim/release are intended for
// startup-time use before interrupts are enabled; data writes use the atomic
// POSR/PORR aliases and do not disturb adjacent pins.
template<typename Policy>
class gpio_backend {
public:
    explicit constexpr gpio_backend(Policy& policy) noexcept : policy_(policy) {}

    // @req REQ-0003 REQ-0009 REQ-0014 REQ-0021
    [[nodiscard]] std::expected<void, driver::gpio_error>
    claim(core::pin_id pin, driver::gpio_direction direction) noexcept {
        const auto mode = direction == driver::gpio_direction::input
            ? core::electrical_mode::input
            : core::electrical_mode::push_pull;
        if (!valid_pin(pin)
            || !core::package_contains(mcu::ra4m1::package_pins, pin, mode)) {
            return std::unexpected(driver::gpio_error::invalid_pin);
        }
        const auto mask = pin_mask(pin);
        if ((owned_[pin.port] & mask) != 0U) {
            return std::unexpected(driver::gpio_error::already_owned);
        }
        owned_[pin.port] = static_cast<std::uint16_t>(owned_[pin.port] | mask);
        return {};
    }

    void release(core::pin_id pin) noexcept {
        if (!valid_pin(pin)) {
            return;
        }
        const auto mask = pin_mask(pin);
        owned_[pin.port] = static_cast<std::uint16_t>(owned_[pin.port] & ~mask);
    }

    [[nodiscard]] std::expected<void, driver::gpio_error>
    configure(core::pin_id pin,
              driver::gpio_configuration configuration) noexcept {
        if (!is_owned(pin)) {
            return std::unexpected(driver::gpio_error::invalid_pin);
        }
        if (configuration.pull == driver::gpio_pull::down) {
            return std::unexpected(driver::gpio_error::unsupported_configuration);
        }
        const auto requested_mode = configuration.direction
                == driver::gpio_direction::input
            ? core::electrical_mode::input
            : (configuration.output_type == driver::gpio_output_type::open_drain
                   ? core::electrical_mode::open_drain
                   : core::electrical_mode::push_pull);
        if (!core::package_contains(mcu::ra4m1::package_pins, pin,
                                    requested_mode)) {
            return std::unexpected(driver::gpio_error::unsupported_configuration);
        }

        const auto pfs = mcu::ra4m1::pfs_address(pin);
        const auto current = policy_.template read<std::uint32_t>(pfs);
        if (!current) {
            return mmio_failure();
        }

        constexpr std::uint32_t controlled = 0x1F01'FC55U;
        auto next = static_cast<std::uint32_t>(*current & ~controlled);
        if (configuration.initial_level == driver::gpio_level::high) {
            next |= 1U;
        }
        if (configuration.direction == driver::gpio_direction::output) {
            next |= 1U << 2U;
        }
        if (configuration.pull == driver::gpio_pull::up) {
            next |= 1U << 4U;
        }
        if (configuration.output_type == driver::gpio_output_type::open_drain) {
            next |= 1U << 6U;
        }
        if (configuration.drive == driver::gpio_drive::middle) {
            // P408 uniquely implements DSCR1 (bit 11); all other package pins
            // use DSCR (bit 10).
            next |= (pin.port == 4U && pin.index == 8U) ? (1U << 11U)
                                                        : (1U << 10U);
        }

        // Establish the output latch before enabling the output driver.
        if (configuration.direction == driver::gpio_direction::output) {
            const auto initial = write_level(pin, configuration.initial_level);
            if (!initial) {
                return initial;
            }
        }

        constexpr std::uintptr_t pwpr = 0x4004'0D03U;
        if (!write<std::uint8_t>(pwpr, 0x00U)
            || !write<std::uint8_t>(pwpr, 0x40U)) {
            best_effort_relock();
            return mmio_failure();
        }
        arch::data_memory_barrier();
        const auto configured = policy_.template write<std::uint32_t>(pfs, next);
        arch::data_memory_barrier();
        const bool relocked = write<std::uint8_t>(pwpr, 0x00U)
            && write<std::uint8_t>(pwpr, 0x80U);
        if (!configured || !relocked) {
            return mmio_failure();
        }
        return {};
    }

    [[nodiscard]] std::expected<void, driver::gpio_error>
    set(core::pin_id pin) noexcept {
        return write_level(pin, driver::gpio_level::high);
    }

    [[nodiscard]] std::expected<void, driver::gpio_error>
    clear(core::pin_id pin) noexcept {
        return write_level(pin, driver::gpio_level::low);
    }

    [[nodiscard]] std::expected<driver::gpio_level, driver::gpio_error>
    read(core::pin_id pin) noexcept {
        if (!is_owned(pin)) {
            return std::unexpected(driver::gpio_error::invalid_pin);
        }
        const auto input = policy_.template read<std::uint32_t>(
            port_base(pin) + 0x04U);
        if (!input) {
            return std::unexpected(driver::gpio_error::mmio);
        }
        return ((*input & pin_mask(pin)) != 0U) ? driver::gpio_level::high
                                                : driver::gpio_level::low;
    }

private:
    [[nodiscard]] static constexpr bool valid_pin(core::pin_id pin) noexcept {
        return pin.port < 10U && pin.index < 16U;
    }

    [[nodiscard]] bool is_owned(core::pin_id pin) const noexcept {
        return valid_pin(pin) && (owned_[pin.port] & pin_mask(pin)) != 0U;
    }

    [[nodiscard]] static constexpr std::uint16_t
    pin_mask(core::pin_id pin) noexcept {
        return static_cast<std::uint16_t>(1U << pin.index);
    }

    [[nodiscard]] static constexpr std::uintptr_t
    port_base(core::pin_id pin) noexcept {
        return 0x4004'0000U + pin.port * 0x20U;
    }

    template<typename Value>
    [[nodiscard]] bool write(std::uintptr_t address, Value value) noexcept {
        return policy_.template write<Value>(address, value).has_value();
    }

    [[nodiscard]] std::expected<void, driver::gpio_error>
    write_level(core::pin_id pin, driver::gpio_level level) noexcept {
        if (!is_owned(pin)) {
            return std::unexpected(driver::gpio_error::invalid_pin);
        }
        const auto mask = static_cast<std::uint32_t>(pin_mask(pin));
        const auto command = level == driver::gpio_level::high
            ? mask
            : static_cast<std::uint32_t>(mask << 16U);
        const auto result = policy_.template write<std::uint32_t>(
            port_base(pin) + 0x08U, command);
        arch::data_memory_barrier();
        if (!result) {
            return mmio_failure();
        }
        return {};
    }

    void best_effort_relock() noexcept {
        constexpr std::uintptr_t pwpr = 0x4004'0D03U;
        static_cast<void>(policy_.template write<std::uint8_t>(pwpr, 0x00U));
        static_cast<void>(policy_.template write<std::uint8_t>(pwpr, 0x80U));
        arch::data_memory_barrier();
    }

    [[nodiscard]] static std::expected<void, driver::gpio_error>
    mmio_failure() noexcept {
        return std::unexpected(driver::gpio_error::mmio);
    }

    Policy& policy_;
    std::array<std::uint16_t, 10> owned_{};
};

} // namespace mcubsp::backend::ra4m1
