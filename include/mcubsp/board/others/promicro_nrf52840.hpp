#pragma once

// @req-file REQ-0053 REQ-0054
#include <cstddef>
#include <mcubsp/backend/nrf52840/gpio.hpp>
#include <mcubsp/backend/nrf52840/i2c.hpp>
#include <mcubsp/board/others/promicro_nrf52840/bindings.hpp>
#include <mcubsp/io/detail/target_time.hpp>

#ifndef MCUBSP_EXTERNAL_STARTUP
extern "C" std::uint32_t mcubsp_promicro_startup_status;
#endif

namespace mcubsp::board::others {

// GPIO/I2C profile. Serial remains unavailable; select {.serial = false}.
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
    static constexpr bool i2c_available = true;
    using i2c_binding = backend::nrf52840::i2c_binding<0, pins_type::scl, pins_type::sda>;
    static constexpr std::array i2c_pins{pins_type::scl, pins_type::sda};
    template<class Binding, class M, class E>
    using i2c_backend_for = backend::nrf52840::i2c_controller_backend<Binding, M, E>;
    template<class M, class E> using i2c_backend = i2c_backend_for<i2c_binding, M, E>;
    static constexpr auto solve_i2c(std::uint32_t hz) noexcept {
        return backend::nrf52840::solve_i2c_timing({mcu::nrf52840::twi_clock_hz, hz, 20'000U, 300U, 100U});
    }
    template<class M, class B> static void poll_i2c(M&, B& bus) noexcept { (void)bus.poll(); }
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
