#pragma once

#include <cstdint>
#include <limits>

#include <mcuhw/arch/barrier.hpp>
#include <mcuhw/core/metadata.hpp>
#include <mcuhw/mcu/stm32h753/description.hpp>

namespace mcuhw::backend::stm32h753::detail {

[[nodiscard]] constexpr mcu::stm32h753::instance_record instance(
    mcu::stm32h753::peripheral_kind kind, std::uint16_t number) noexcept {
    for (const auto& candidate : mcu::stm32h753::instances) {
        if (candidate.id
            == mcu::stm32h753::peripheral(kind, number)) {
            return candidate;
        }
    }
    return {};
}

[[nodiscard]] constexpr std::uint32_t alternate_function(
    core::pin_id pin, mcu::stm32h753::signal signal) noexcept {
    for (const auto& candidate : mcu::stm32h753::pin_routes) {
        if (candidate.pin == pin
            && candidate.signal == mcu::stm32h753::signal_id(signal)) {
            return candidate.vendor_encoding;
        }
    }
    return std::numeric_limits<std::uint32_t>::max();
}

template<typename MmioPolicy>
class pinmux {
public:
    explicit constexpr pinmux(MmioPolicy& mmio) noexcept : mmio_(mmio) {}

    [[nodiscard]] bool alternate(core::pin_id pin, std::uint32_t af,
                                 bool open_drain = false,
                                 std::uint32_t pull = 0U) noexcept {
        if (pin.port >= 11U || pin.index >= 16U || af > 15U
            || !enable_clock(pin.port)) {
            return false;
        }
        const auto base = mcu::stm32h753::gpio_base(pin.port);
        const auto pair_shift = static_cast<unsigned>(pin.index * 2U);
        const auto pair_mask = static_cast<std::uint32_t>(3U << pair_shift);
        const auto bit = static_cast<std::uint32_t>(1U << pin.index);
        const auto afr = base + (pin.index < 8U ? 0x20U : 0x24U);
        const auto af_shift = static_cast<unsigned>((pin.index % 8U) * 4U);
        const auto af_mask = static_cast<std::uint32_t>(0xFU << af_shift);

        const auto mode = read(base + 0x00U);
        const auto type = read(base + 0x04U);
        const auto speed = read(base + 0x08U);
        const auto pulls = read(base + 0x0CU);
        const auto function = read(afr);
        if (!mode || !type || !speed || !pulls || !function) {
            return false;
        }
        const auto next_type = open_drain ? (*type | bit) : (*type & ~bit);
        if (!write(base + 0x04U, next_type)
            || !write(base + 0x08U, (*speed & ~pair_mask)
                                      | (2U << pair_shift))
            || !write(base + 0x0CU, (*pulls & ~pair_mask)
                                      | ((pull & 3U) << pair_shift))
            || !write(afr, (*function & ~af_mask) | (af << af_shift))) {
            return false;
        }
        arch::data_memory_barrier();
        if (!write(base + 0x00U, (*mode & ~pair_mask)
                                      | (2U << pair_shift))) {
            return false;
        }
        arch::data_synchronization_barrier();
        return true;
    }

    [[nodiscard]] bool analog(core::pin_id pin) noexcept {
        if (pin.port >= 11U || pin.index >= 16U || !enable_clock(pin.port)) {
            return false;
        }
        const auto base = mcu::stm32h753::gpio_base(pin.port);
        const auto shift = static_cast<unsigned>(pin.index * 2U);
        const auto mask = static_cast<std::uint32_t>(3U << shift);
        const auto mode = read(base + 0x00U);
        const auto pull = read(base + 0x0CU);
        return mode && pull
            && write(base + 0x0CU, *pull & ~mask)
            && write(base + 0x00U, (*mode & ~mask) | mask);
    }

    [[nodiscard]] bool output(core::pin_id pin, bool high) noexcept {
        if (pin.port >= 11U || pin.index >= 16U || !enable_clock(pin.port)) {
            return false;
        }
        const auto base = mcu::stm32h753::gpio_base(pin.port);
        const auto bit = static_cast<std::uint32_t>(1U << pin.index);
        if (!write(base + 0x18U, high ? bit : bit << 16U)) {
            return false;
        }
        const auto shift = static_cast<unsigned>(pin.index * 2U);
        const auto mask = static_cast<std::uint32_t>(3U << shift);
        const auto mode = read(base + 0x00U);
        const auto type = read(base + 0x04U);
        const auto pull = read(base + 0x0CU);
        return mode && type && pull
            && write(base + 0x04U, *type & ~bit)
            && write(base + 0x0CU, *pull & ~mask)
            && write(base + 0x00U, (*mode & ~mask) | (1U << shift));
    }

    [[nodiscard]] bool set_output(core::pin_id pin, bool high) noexcept {
        const auto bit = static_cast<std::uint32_t>(1U << pin.index);
        return write(mcu::stm32h753::gpio_base(pin.port) + 0x18U,
                     high ? bit : bit << 16U);
    }

    [[nodiscard]] bool restore(core::pin_id pin) noexcept {
        if (pin.port >= 11U || pin.index >= 16U || !enable_clock(pin.port)) {
            return false;
        }
        const auto base = mcu::stm32h753::gpio_base(pin.port);
        const auto shift = static_cast<unsigned>(pin.index * 2U);
        const auto mask = static_cast<std::uint32_t>(3U << shift);
        const auto mode = read(base + 0x00U);
        const auto pull = read(base + 0x0CU);
        return mode && pull
            && write(base + 0x0CU, *pull & ~mask)
            && write(base + 0x00U, *mode & ~mask);
    }

private:
    [[nodiscard]] bool enable_clock(std::uint16_t port) noexcept {
        constexpr auto ahb4enr = mcu::stm32h753::rcc_base + 0xE0U;
        const auto current = read(ahb4enr);
        if (!current) {
            return false;
        }
        const auto bit = static_cast<std::uint32_t>(1U << port);
        if ((*current & bit) == 0U && !write(ahb4enr, *current | bit)) {
            return false;
        }
        arch::data_memory_barrier();
        return true;
    }

    [[nodiscard]] auto read(std::uintptr_t address) noexcept {
        return mmio_.template read<std::uint32_t>(address);
    }

    [[nodiscard]] bool write(std::uintptr_t address,
                             std::uint32_t value) noexcept {
        return mmio_.template write<std::uint32_t>(address, value).has_value();
    }

    MmioPolicy& mmio_;
};

template<typename MmioPolicy>
[[nodiscard]] bool enable_peripheral_clock(
    MmioPolicy& mmio, mcu::stm32h753::instance_record metadata) noexcept {
    const auto value = mmio.template read<std::uint32_t>(
        metadata.clock_enable_register);
    if (!value
        || !mmio.template write<std::uint32_t>(metadata.clock_enable_register,
                                               *value
                                                   | metadata.clock_enable_mask)) {
        return false;
    }
    arch::data_memory_barrier();
    return true;
}

} // namespace mcuhw::backend::stm32h753::detail
