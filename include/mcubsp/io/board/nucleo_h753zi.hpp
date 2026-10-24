#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include <mcubsp/backend/stm32h753/adc.hpp>
#include <mcubsp/backend/stm32h753/gpio.hpp>
#include <mcubsp/backend/stm32h753/i2c.hpp>
#include <mcubsp/backend/stm32h753/spi.hpp>
#include <mcubsp/backend/stm32h753/timer.hpp>
#include <mcubsp/backend/stm32h753/usart.hpp>
#include <mcubsp/board/nucleo_h753zi.hpp>
#include <mcubsp/io/detail/target_time.hpp>

#ifndef MCUBSP_EXTERNAL_STARTUP
extern "C" volatile std::uint32_t mcubsp_nucleo_clock_status;
#endif

namespace mcubsp::io::boards {

// Matches board/st/nucleo_h753zi/bsp/early_init.cpp: HSI SYSCLK/HCLK/PCLK at
// 64 MHz. Applications with another clock tree supply corresponding traits.
struct nucleo_h753zi {
    using target_identity = nucleo_h753zi;
#ifdef MCUBSP_EXTERNAL_STARTUP
    [[nodiscard]] static constexpr bool startup_ok() noexcept { return true; }
#else
    [[nodiscard]] static bool startup_ok() noexcept { return mcubsp_nucleo_clock_status == 0U; }
#endif
    struct pins_type {
        static constexpr auto led_builtin = mcubsp::board::nucleo_h753zi::user_led_yellow;
        static constexpr auto rx = mcubsp::board::nucleo_h753zi::stlink_vcp_rx;
        static constexpr auto tx = mcubsp::board::nucleo_h753zi::stlink_vcp_tx;
        static constexpr auto d10 = mcubsp::board::nucleo_h753zi::arduino_d10_cs;
        static constexpr auto d11 = mcubsp::board::nucleo_h753zi::arduino_d11_mosi;
        static constexpr auto d12 = mcubsp::board::nucleo_h753zi::arduino_d12_miso;
        static constexpr auto d13 = mcubsp::board::nucleo_h753zi::arduino_d13_sck;
        static constexpr auto d14 = mcubsp::board::nucleo_h753zi::arduino_d14_sda;
        static constexpr auto d15 = mcubsp::board::nucleo_h753zi::arduino_d15_scl;
        static constexpr auto a0 = mcubsp::board::nucleo_h753zi::arduino_a0;
        static constexpr auto miso = d12;
        static constexpr auto mosi = d11;
        static constexpr auto sck = d13;
        static constexpr auto cs = d10;
        static constexpr auto sda = d14;
        static constexpr auto scl = d15;
        static constexpr auto timer1_ch1 = mcubsp::board::nucleo_h753zi::timer1_ch1;
    };

    static constexpr std::size_t pin_slots = 176U;
    static constexpr std::size_t pin_index(core::pin_id pin) noexcept {
        return pin.port < 11U && pin.index < 16U
            ? static_cast<std::size_t>(pin.port) * 16U + pin.index : pin_slots;
    }
    static constexpr std::uint32_t ticks_per_second = 64'000'000U;
    using default_execution = detail::target_execution<
        detail::cortex_m_cycle_hooks<ticks_per_second, true>>;

    using serial_binding = backend::stm32h753::usart_binding<
        3, pins_type::rx, pins_type::tx>;
    using spi_binding = backend::stm32h753::spi_binding<
        1, pins_type::miso, pins_type::mosi, pins_type::sck, pins_type::cs>;
    using i2c_binding = backend::stm32h753::i2c_binding<
        1, pins_type::scl, pins_type::sda>;
    using adc_binding = backend::stm32h753::adc1_binding<
        backend::stm32h753::adc_input<15, pins_type::a0>>;
    using pwm_binding = backend::stm32h753::timer_binding<
        1, mcu::stm32h753::timer_output::ch1, pins_type::timer1_ch1>;

    template<typename M> using gpio_backend = backend::stm32h753::gpio_backend<M>;
    template<typename M, typename E> using serial_backend =
        backend::stm32h753::usart_backend<serial_binding, M, E>;
    template<typename M, typename E> using spi_backend =
        backend::stm32h753::spi_master_backend<spi_binding, M, E, true>;
    template<typename M, typename E> using i2c_backend =
        backend::stm32h753::i2c_controller_backend<i2c_binding, M, E>;
    template<typename M, typename E> using adc_backend =
        backend::stm32h753::adc_backend<adc_binding, M, E>;
    template<typename M, typename E> using pwm_backend =
        backend::stm32h753::timer_pwm_backend<pwm_binding, M, E>;
    static constexpr std::uint32_t dma_available = 0U;
    static constexpr bool adc_available = true;
    static constexpr bool pwm_available = true;
    static constexpr auto analog_input_pin = pins_type::a0;
    static constexpr auto pwm_output_pin = pins_type::timer1_ch1;
    static constexpr auto pwm_pin = pwm_output_pin;

    static constexpr std::array serial_pins{pins_type::rx, pins_type::tx};
    static constexpr std::array spi_pins{
        pins_type::miso, pins_type::mosi, pins_type::sck, pins_type::cs};
    static constexpr std::array i2c_pins{pins_type::scl, pins_type::sda};
    static constexpr std::array adc_pins{analog_input_pin};
    static constexpr std::array pwm_pins{pwm_output_pin};
    static constexpr std::array adc_channels{driver::adc_channel{15U}};

    static constexpr auto solve_serial(std::uint32_t baud) noexcept {
        return backend::stm32h753::solve_usart_baud(
            {64'000'000U, baud, 20'000U});
    }
    static constexpr auto solve_spi(std::uint32_t hz) noexcept {
        return backend::stm32h753::solve_stm32_spi_timing(
            {64'000'000U, hz, 50'000U});
    }
    static constexpr auto solve_i2c(std::uint32_t hz) noexcept {
        return backend::stm32h753::solve_stm32_i2c_timing(
            {64'000'000U, hz, 30'000U,
             hz > 100'000U ? 120U : 400U, 100U});
    }
    static constexpr auto solve_pwm(std::uint32_t hz) noexcept {
        return backend::stm32h753::solve_timer_pwm(
            {64'000'000U, hz, 30'000U,
             pwm_binding::timer_metadata.counter_width});
    }

    template<typename M> static void start(M&) noexcept {}
    template<typename M> static void stop(M&) noexcept {}

    template<typename M, typename B>
    static void poll_serial(M&, B& peripheral) noexcept {
        static_cast<void>(peripheral.handle_interrupt());
    }
    template<typename M, typename B>
    static void poll_spi(M&, B& peripheral) noexcept {
        static_cast<void>(peripheral.handle_interrupt());
    }
    template<typename M, typename B>
    static void poll_i2c(M&, B& peripheral) noexcept {
        static_cast<void>(peripheral.handle_interrupt());
    }
    template<typename M, typename B>
    static void poll_adc(M&, B& peripheral) noexcept {
        // Checks EOC/EOS/OVR before reading or publishing a conversion.
        static_cast<void>(peripheral.handle_conversion_interrupt());
    }
    template<typename M, typename B>
    static void poll_pwm(M&, B& peripheral) noexcept {
        // Checks and clears SR.UIF before publishing a period event.
        static_cast<void>(peripheral.handle_update_interrupt());
    }
};

} // namespace mcubsp::io::boards
