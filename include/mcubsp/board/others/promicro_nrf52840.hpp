#pragma once

// @req-file REQ-0053
#include <cstddef>
#include <mcubsp/backend/nrf52840/gpio.hpp>
#include <mcubsp/board/others/promicro_nrf52840/bindings.hpp>
#include <mcubsp/io/detail/target_time.hpp>

#ifndef MCUBSP_EXTERNAL_STARTUP
extern "C" std::uint32_t mcubsp_promicro_startup_status;
#endif

namespace mcubsp::board::others {

// GPIO-only bring-up profile. Instantiate io::board with {.serial = false}.
struct promicro_nrf52840 : promicro_nrf52840_bindings {
    using target_identity = promicro_nrf52840;
    using pins_type = promicro_nrf52840_pins;
    static constexpr std::uint32_t ticks_per_second = mcu::nrf52840::cpu_frequency;
    using default_execution = io::detail::target_execution<
        io::detail::cortex_m_cycle_hooks<ticks_per_second>>;
    template<class M> using gpio_backend = backend::nrf52840::gpio_backend<M>;
    static constexpr std::size_t pin_slots = 64U;
    static constexpr std::size_t pin_index(core::pin_id pin) noexcept {
        return mcu::nrf52840::gpio_supported(pin)
            ? static_cast<std::size_t>(pin.port) * 32U + pin.index : pin_slots;
    }
    static constexpr bool serial_available = false;
    static constexpr bool i2c_available = false;
    static constexpr bool spi_available = false;
    static constexpr bool adc_available = false;
    static constexpr bool pwm_available = false;
    static constexpr std::uint32_t dma_available = 0U;
    [[nodiscard]] static bool startup_ok() noexcept {
#ifdef MCUBSP_EXTERNAL_STARTUP
        // The external BSP owns silicon initialization, clocks and its checks.
        return true;
#else
        return mcubsp_promicro_startup_status == 0U;
#endif
    }
    template<class M> static void start(M&) noexcept {}
    template<class M> static void stop(M&) noexcept {}
};

} // namespace mcubsp::board::others
