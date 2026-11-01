#pragma once

// @req-file REQ-0053 REQ-0055
#include <array>
#include <cstdint>
#include <expected>
#include <mcubsp/arch/barrier.hpp>
#include <mcubsp/driver/gpio.hpp>
#include <mcubsp/backend/nrf52840/resources.hpp>
#include <mcubsp/mcu/nrf52840/peripherals.hpp>
#include <mcubsp/mcu/nrf52840/product/pins.hpp>

namespace mcubsp::backend::nrf52840 {

// One foreground owner; atomic OUTSET/OUTCLR do not disturb adjacent outputs.
template<class Mmio>
class gpio_backend {
public:
    explicit constexpr gpio_backend(Mmio& mmio, resource_registry& resources = default_resources) noexcept
        : mmio_(mmio), resources_(resources) {}
    gpio_backend(const gpio_backend&) = delete;
    gpio_backend& operator=(const gpio_backend&) = delete;
    [[nodiscard]] resource_registry& resources() noexcept { return resources_; }

    [[nodiscard]] std::expected<void, driver::gpio_error>
    claim(core::pin_id pin, driver::gpio_direction) noexcept {
        if (!mcu::nrf52840::valid_pin(pin))
            return std::unexpected(driver::gpio_error::invalid_pin);
        if (!mcu::nrf52840::gpio_supported(pin))
            return std::unexpected(driver::gpio_error::unsupported_configuration);
        if (owned(pin) || !resources_.claim_gpio(pin,this))
            return std::unexpected(driver::gpio_error::already_owned);
        owned_[pin.port] |= mask(pin);
        return {};
    }
    void release(core::pin_id pin) noexcept {
        if (owned(pin)) {
            resources_.release_gpio(pin,this);
            owned_[pin.port] &= ~mask(pin);
        }
    }
    [[nodiscard]] std::expected<void, driver::gpio_error>
    configure(core::pin_id pin, driver::gpio_configuration config) noexcept {
        if (!owned(pin)) return std::unexpected(driver::gpio_error::invalid_pin);
        if (config.drive == driver::gpio_drive::middle &&
            mcu::nrf52840::product::silicon_low_frequency_only(pin))
            return std::unexpected(driver::gpio_error::unsupported_configuration);
        const auto output = config.direction == driver::gpio_direction::output;
        // INPUT stays connected so read() works for output leases as well.
        std::uint32_t value = output ? 1U : 0U;
        if (config.pull == driver::gpio_pull::down) value |= 1U << 2U;
        if (config.pull == driver::gpio_pull::up) value |= 3U << 2U;
        const bool high_drive = config.drive == driver::gpio_drive::middle;
        const auto drive = config.output_type == driver::gpio_output_type::open_drain
            ? (high_drive ? 7U : 6U) : (high_drive ? 3U : 0U);
        value |= drive << 8U;
        // Set the latch before changing DIR to output. SENSE is disabled.
        if (output) {
            const auto initial = write_level(pin, config.initial_level);
            if (!initial) return initial;
        }
        if (!mmio_.template write<std::uint32_t>(mcu::nrf52840::pin_configuration(pin), value))
            return std::unexpected(driver::gpio_error::mmio);
        arch::data_synchronization_barrier();
        return {};
    }
    [[nodiscard]] auto set(core::pin_id pin) noexcept {
        return write_level(pin, driver::gpio_level::high);
    }
    [[nodiscard]] auto clear(core::pin_id pin) noexcept {
        return write_level(pin, driver::gpio_level::low);
    }
    [[nodiscard]] std::expected<driver::gpio_level, driver::gpio_error>
    read(core::pin_id pin) noexcept {
        if (!owned(pin)) return std::unexpected(driver::gpio_error::invalid_pin);
        const auto value = mmio_.template read<std::uint32_t>(
            mcu::nrf52840::gpio_base(pin) + mcu::nrf52840::gpio_registers::in::offset);
        if (!value) return std::unexpected(driver::gpio_error::mmio);
        return (*value & mask(pin)) ? driver::gpio_level::high : driver::gpio_level::low;
    }
private:
    [[nodiscard]] static constexpr std::uint32_t mask(core::pin_id pin) noexcept {
        return std::uint32_t{1} << pin.index;
    }
    [[nodiscard]] bool owned(core::pin_id pin) const noexcept {
        return mcu::nrf52840::valid_pin(pin) && (owned_[pin.port] & mask(pin)) != 0U;
    }
    [[nodiscard]] std::expected<void, driver::gpio_error>
    write_level(core::pin_id pin, driver::gpio_level level) noexcept {
        if (!owned(pin)) return std::unexpected(driver::gpio_error::invalid_pin);
        const auto offset = level == driver::gpio_level::high
            ? mcu::nrf52840::gpio_registers::outset::offset
            : mcu::nrf52840::gpio_registers::outclr::offset;
        if (!mmio_.template write<std::uint32_t>(mcu::nrf52840::gpio_base(pin) + offset, mask(pin)))
            return std::unexpected(driver::gpio_error::mmio);
        arch::data_memory_barrier();
        return {};
    }
    Mmio& mmio_;
    resource_registry& resources_;
    std::array<std::uint32_t, 2> owned_{};
};

} // namespace mcubsp::backend::nrf52840
