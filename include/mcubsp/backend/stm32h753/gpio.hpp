#pragma once

#include <array>
#include <cstdint>
#include <expected>

#include <mcubsp/arch/barrier.hpp>
#include <mcubsp/core/mmio.hpp>
#include <mcubsp/driver/gpio.hpp>
#include <mcubsp/mcu/stm32h753/pins.hpp>
#include <mcubsp/mcu/stm32h753/system.hpp>

namespace mcubsp::backend::stm32h753 {

// STM32H753 native GPIO sequencing and startup-time ownership. GPIO clocks and
// configuration registers are updated before MODER enables an output. BSRR is
// used for atomic data changes that preserve every adjacent pin.
template<typename Policy,
         mcu::stm32h753::package_variant Package =
             mcu::stm32h753::package_variant::lqfp144>
class gpio_backend {
public:
    explicit constexpr gpio_backend(Policy& policy) noexcept : policy_(policy) {}

    // @req REQ-0003 REQ-0009 REQ-0014 REQ-0021 REQ-0033
    [[nodiscard]] std::expected<void, driver::gpio_error>
    claim(core::pin_id pin, driver::gpio_direction) noexcept {
        if (!valid_pin(pin)) {
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
        owned_[pin.port] = static_cast<std::uint16_t>(
            owned_[pin.port] & ~pin_mask(pin));
    }

    [[nodiscard]] std::expected<void, driver::gpio_error>
    configure(core::pin_id pin,
              driver::gpio_configuration configuration) noexcept {
        if (!is_owned(pin)) {
            return std::unexpected(driver::gpio_error::invalid_pin);
        }
        const auto clock = enable_port_clock(pin.port);
        if (!clock) {
            return clock;
        }

        const auto base = mcu::stm32h753::gpio_base(pin.port);
        const auto bit = static_cast<std::uint32_t>(pin_mask(pin));
        const auto pair_shift = static_cast<unsigned>(pin.index * 2U);
        const auto pair_mask = static_cast<std::uint32_t>(0x3U << pair_shift);

        // Establish the latch before MODER connects the output driver.
        if (configuration.direction == driver::gpio_direction::output) {
            const auto initial = write_level(pin, configuration.initial_level);
            if (!initial) {
                return initial;
            }
        }

        const auto output_type = policy_.template read<std::uint32_t>(base + 0x04U);
        const auto speed = policy_.template read<std::uint32_t>(base + 0x08U);
        const auto pull = policy_.template read<std::uint32_t>(base + 0x0CU);
        const auto mode = policy_.template read<std::uint32_t>(base + 0x00U);
        if (!output_type || !speed || !pull || !mode) {
            return mmio_failure();
        }

        const auto next_output_type = configuration.output_type
                == driver::gpio_output_type::open_drain
            ? static_cast<std::uint32_t>(*output_type | bit)
            : static_cast<std::uint32_t>(*output_type & ~bit);
        const auto speed_encoding = configuration.drive == driver::gpio_drive::middle
            ? 0x1U
            : 0x0U;
        const auto next_speed = static_cast<std::uint32_t>(
            (*speed & ~pair_mask) | (speed_encoding << pair_shift));
        const auto pull_encoding = configuration.pull == driver::gpio_pull::up
            ? 0x1U
            : (configuration.pull == driver::gpio_pull::down ? 0x2U : 0x0U);
        const auto next_pull = static_cast<std::uint32_t>(
            (*pull & ~pair_mask) | (pull_encoding << pair_shift));
        const auto mode_encoding = configuration.direction
                == driver::gpio_direction::output
            ? 0x1U
            : 0x0U;
        const auto next_mode = static_cast<std::uint32_t>(
            (*mode & ~pair_mask) | (mode_encoding << pair_shift));

        if (!write(base + 0x04U, next_output_type)
            || !write(base + 0x08U, next_speed)
            || !write(base + 0x0CU, next_pull)) {
            return mmio_failure();
        }
        arch::data_memory_barrier();
        if (!write(base + 0x00U, next_mode)) {
            return mmio_failure();
        }
        arch::data_synchronization_barrier();
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
        const auto input = policy_.template read<std::uint16_t>(
            mcu::stm32h753::gpio_base(pin.port) + 0x10U);
        if (!input) {
            return std::unexpected(driver::gpio_error::mmio);
        }
        return ((*input & pin_mask(pin)) != 0U) ? driver::gpio_level::high
                                                : driver::gpio_level::low;
    }

private:
    [[nodiscard]] static constexpr bool valid_pin(core::pin_id pin) noexcept {
        return pin.port < 11U && pin.index < 16U
            && mcu::stm32h753::package_contains(
                Package, pin, mcu::stm32h753::package_memberships);
    }

    [[nodiscard]] bool is_owned(core::pin_id pin) const noexcept {
        return valid_pin(pin) && (owned_[pin.port] & pin_mask(pin)) != 0U;
    }

    [[nodiscard]] static constexpr std::uint16_t
    pin_mask(core::pin_id pin) noexcept {
        return static_cast<std::uint16_t>(1U << pin.index);
    }

    [[nodiscard]] std::expected<void, driver::gpio_error>
    enable_port_clock(std::uint16_t port) noexcept {
        constexpr std::uintptr_t ahb4enr = mcu::stm32h753::rcc_base + 0xE0U;
        const auto current = policy_.template read<std::uint32_t>(ahb4enr);
        if (!current) {
            return mmio_failure();
        }
        const auto bit = static_cast<std::uint32_t>(1U << port);
        if ((*current & bit) == 0U && !write(ahb4enr, *current | bit)) {
            return mmio_failure();
        }
        arch::data_memory_barrier();
        return {};
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
        if (!write(mcu::stm32h753::gpio_base(pin.port) + 0x18U, command)) {
            return mmio_failure();
        }
        arch::data_memory_barrier();
        return {};
    }

    [[nodiscard]] bool write(std::uintptr_t address,
                             std::uint32_t value) noexcept {
        return policy_.template write<std::uint32_t>(address, value).has_value();
    }

    [[nodiscard]] static std::expected<void, driver::gpio_error>
    mmio_failure() noexcept {
        return std::unexpected(driver::gpio_error::mmio);
    }

    Policy& policy_;
    std::array<std::uint16_t, 11> owned_{};
};

} // namespace mcubsp::backend::stm32h753
