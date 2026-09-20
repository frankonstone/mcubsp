#pragma once

#include <array>
#include <cstdint>
#include <expected>

#include <mcuhw/arch/barrier.hpp>
#include <mcuhw/core/mmio.hpp>
#include <mcuhw/driver/gpio.hpp>
#include <mcuhw/mcu/esp32p4/pins.hpp>

namespace mcuhw::backend::esp32p4 {

// HP-domain GPIO backend for one startup owner (HP CPU0 in the initial board
// port). Ownership is fixed-storage and intentionally not multicore-safe.
// Atomic W1TS/W1TC aliases prevent adjacent GPIO state from being disturbed.
template<typename Policy>
class gpio_backend {
public:
    explicit constexpr gpio_backend(Policy& policy) noexcept : policy_(policy) {}

    // @req REQ-0003 REQ-0004 REQ-0009 REQ-0014 REQ-0034
    [[nodiscard]] std::expected<void, driver::gpio_error>
    claim(core::pin_id pin, driver::gpio_direction direction) noexcept {
        const auto mode = direction == driver::gpio_direction::input
            ? core::electrical_mode::input
            : core::electrical_mode::push_pull;
        if (!valid_pin(pin)
            || !core::package_contains(mcu::esp32p4::package_pins, pin, mode)) {
            return std::unexpected(driver::gpio_error::invalid_pin);
        }
        const auto mask = ownership_mask(pin);
        if ((owned_[ownership_word(pin)] & mask) != 0U) {
            return std::unexpected(driver::gpio_error::already_owned);
        }
        owned_[ownership_word(pin)] |= mask;
        return {};
    }

    void release(core::pin_id pin) noexcept {
        if (!valid_pin(pin)) {
            return;
        }
        owned_[ownership_word(pin)] &= ~ownership_mask(pin);
    }

    [[nodiscard]] std::expected<void, driver::gpio_error>
    configure(core::pin_id pin,
              driver::gpio_configuration configuration) noexcept {
        if (!is_owned(pin)) {
            return std::unexpected(driver::gpio_error::invalid_pin);
        }
        // GPIO24-27 are attached to the two native USB PHYs. Selecting GPIO
        // requires disabling the corresponding PHY first; that ownership
        // transition belongs to the USB/board integration layer.
        if (pin.index >= 24U && pin.index <= 27U) {
            return std::unexpected(
                driver::gpio_error::unsupported_configuration);
        }
        const auto requested_mode = configuration.direction
                == driver::gpio_direction::input
            ? core::electrical_mode::input
            : (configuration.output_type == driver::gpio_output_type::open_drain
                   ? core::electrical_mode::open_drain
                   : core::electrical_mode::push_pull);
        if (!core::package_contains(mcu::esp32p4::package_pins, pin,
                                    requested_mode)) {
            return std::unexpected(driver::gpio_error::unsupported_configuration);
        }

        if (configuration.direction == driver::gpio_direction::output) {
            const auto initial = write_level(pin, configuration.initial_level);
            if (!initial) {
                return initial;
            }
        }

        const auto pad = pad_address(pin);
        const auto pin_control = pin_control_address(pin);
        const auto output_select = output_select_address(pin);
        const auto current_pad = read(pad);
        const auto current_pin_control = read(pin_control);
        const auto current_output_select = read(output_select);
        if (!current_pad || !current_pin_control || !current_output_select) {
            return mmio_failure();
        }

        constexpr std::uint32_t pad_controlled =
            (1U << 7U) | (1U << 8U) | (1U << 9U)
            | (0x3U << 10U) | (0x7U << 12U);
        auto next_pad = static_cast<std::uint32_t>(
            *current_pad & ~pad_controlled);
        if (configuration.pull == driver::gpio_pull::down) {
            next_pad |= 1U << 7U;
        } else if (configuration.pull == driver::gpio_pull::up) {
            next_pad |= 1U << 8U;
        }
        if (configuration.direction == driver::gpio_direction::input) {
            next_pad |= 1U << 9U;
        }
        if (configuration.drive == driver::gpio_drive::middle) {
            next_pad |= 0x2U << 10U;
        }
        next_pad |= 1U << 12U; // PIN_FUNC_GPIO

        constexpr std::uint32_t pad_driver = 1U << 2U;
        const auto next_pin_control = configuration.output_type
                == driver::gpio_output_type::open_drain
            ? static_cast<std::uint32_t>(*current_pin_control | pad_driver)
            : static_cast<std::uint32_t>(*current_pin_control & ~pad_driver);

        // Route GPIO_OUT and force GPIO_ENABLE to own output enable.
        constexpr std::uint32_t output_controlled = 0xFFFU;
        constexpr std::uint32_t gpio_out_signal = 256U;
        constexpr std::uint32_t gpio_enable_control = 1U << 10U;
        const auto next_output_select = static_cast<std::uint32_t>(
            (*current_output_select & ~output_controlled)
            | gpio_out_signal | gpio_enable_control);

        if (!write(pin_control, next_pin_control)
            || !write(output_select, next_output_select)
            || !write(pad, next_pad)) {
            return mmio_failure();
        }
        arch::data_memory_barrier();

        const auto enable_address = configuration.direction
                == driver::gpio_direction::output
            ? output_enable_set_address(pin)
            : output_enable_clear_address(pin);
        if (!write(enable_address, register_mask(pin))) {
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
        const auto input = policy_.template read<std::uint32_t>(
            input_address(pin));
        if (!input) {
            return std::unexpected(driver::gpio_error::mmio);
        }
        return ((*input & register_mask(pin)) != 0U)
            ? driver::gpio_level::high
            : driver::gpio_level::low;
    }

private:
    [[nodiscard]] static constexpr bool valid_pin(core::pin_id pin) noexcept {
        return pin.port == 0U && pin.index < mcu::esp32p4::package_pins.size();
    }

    [[nodiscard]] bool is_owned(core::pin_id pin) const noexcept {
        return valid_pin(pin)
            && (owned_[ownership_word(pin)] & ownership_mask(pin)) != 0U;
    }

    [[nodiscard]] static constexpr std::size_t
    ownership_word(core::pin_id pin) noexcept {
        return pin.index / 32U;
    }

    [[nodiscard]] static constexpr std::uint32_t
    ownership_mask(core::pin_id pin) noexcept {
        return 1U << (pin.index % 32U);
    }

    [[nodiscard]] static constexpr std::uint32_t
    register_mask(core::pin_id pin) noexcept {
        return 1U << (pin.index % 32U);
    }

    [[nodiscard]] static constexpr std::uintptr_t
    pad_address(core::pin_id pin) noexcept {
        return mcu::esp32p4::io_mux_base + 0x04U + pin.index * 4U;
    }

    [[nodiscard]] static constexpr std::uintptr_t
    pin_control_address(core::pin_id pin) noexcept {
        return mcu::esp32p4::gpio_matrix_base + 0x74U + pin.index * 4U;
    }

    [[nodiscard]] static constexpr std::uintptr_t
    output_select_address(core::pin_id pin) noexcept {
        return mcu::esp32p4::gpio_matrix_base + 0x558U + pin.index * 4U;
    }

    [[nodiscard]] static constexpr std::uintptr_t
    output_set_address(core::pin_id pin) noexcept {
        return mcu::esp32p4::gpio_matrix_base
            + (pin.index < 32U ? 0x08U : 0x14U);
    }

    [[nodiscard]] static constexpr std::uintptr_t
    output_clear_address(core::pin_id pin) noexcept {
        return mcu::esp32p4::gpio_matrix_base
            + (pin.index < 32U ? 0x0CU : 0x18U);
    }

    [[nodiscard]] static constexpr std::uintptr_t
    output_enable_set_address(core::pin_id pin) noexcept {
        return mcu::esp32p4::gpio_matrix_base
            + (pin.index < 32U ? 0x24U : 0x30U);
    }

    [[nodiscard]] static constexpr std::uintptr_t
    output_enable_clear_address(core::pin_id pin) noexcept {
        return mcu::esp32p4::gpio_matrix_base
            + (pin.index < 32U ? 0x28U : 0x34U);
    }

    [[nodiscard]] static constexpr std::uintptr_t
    input_address(core::pin_id pin) noexcept {
        return mcu::esp32p4::gpio_matrix_base
            + (pin.index < 32U ? 0x3CU : 0x40U);
    }

    [[nodiscard]] std::expected<std::uint32_t, core::mmio_error>
    read(std::uintptr_t address) noexcept {
        return policy_.template read<std::uint32_t>(address);
    }

    [[nodiscard]] bool write(std::uintptr_t address,
                             std::uint32_t value) noexcept {
        return policy_.template write<std::uint32_t>(address, value).has_value();
    }

    [[nodiscard]] std::expected<void, driver::gpio_error>
    write_level(core::pin_id pin, driver::gpio_level level) noexcept {
        if (!is_owned(pin)) {
            return std::unexpected(driver::gpio_error::invalid_pin);
        }
        const auto address = level == driver::gpio_level::high
            ? output_set_address(pin)
            : output_clear_address(pin);
        if (!write(address, register_mask(pin))) {
            return mmio_failure();
        }
        arch::data_memory_barrier();
        return {};
    }

    [[nodiscard]] static std::expected<void, driver::gpio_error>
    mmio_failure() noexcept {
        return std::unexpected(driver::gpio_error::mmio);
    }

    Policy& policy_;
    std::array<std::uint32_t, 2> owned_{};
};

} // namespace mcuhw::backend::esp32p4
