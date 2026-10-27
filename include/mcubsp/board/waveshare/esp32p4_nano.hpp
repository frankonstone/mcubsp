#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include <mcubsp/backend/esp32p4/gpio.hpp>
#include <mcubsp/backend/esp32p4/i2c.hpp>
#include <mcubsp/backend/esp32p4/spi.hpp>
#include <mcubsp/backend/esp32p4/uart.hpp>
#include <mcubsp/board/waveshare/esp32p4_nano/bindings.hpp>
#include <mcubsp/io/detail/target_time.hpp>

namespace mcubsp::board::waveshare {

// The existing NANO board integration is development silicon v1.0, HP CPU0
// only, booted by src/board/waveshare/esp32p4_nano/test/firmware at a fixed 360 MHz.
// These traits intentionally preserve its observed I2C clock/reset wiring and
// pre-v3 CLIC layout. They do not qualify production-v3 silicon. A different
// silicon revision or CPU clock requires a corresponding board composition.
struct esp32p4_nano : esp32p4_nano_bindings {
    using target_identity = esp32p4_nano;
    [[nodiscard]] static constexpr bool startup_ok() noexcept { return true; }
    struct pins_type {
        static constexpr auto rx = mcubsp::board::waveshare::esp32p4_nano_bindings::uart0_rx;
        static constexpr auto tx = mcubsp::board::waveshare::esp32p4_nano_bindings::uart0_tx;
        static constexpr auto sda = mcubsp::board::waveshare::esp32p4_nano_bindings::default_i2c_sda;
        static constexpr auto scl = mcubsp::board::waveshare::esp32p4_nano_bindings::default_i2c_scl;
        static constexpr auto miso = mcubsp::board::waveshare::esp32p4_nano_bindings::default_spi_miso;
        static constexpr auto mosi = mcubsp::board::waveshare::esp32p4_nano_bindings::default_spi_mosi;
        static constexpr auto sck = mcubsp::board::waveshare::esp32p4_nano_bindings::default_spi_sck;
        static constexpr auto cs = mcubsp::board::waveshare::esp32p4_nano_bindings::default_spi_cs;
        static constexpr core::pin_id gpio2{0U, 2U};
        static constexpr auto uart0_rx = rx;
        static constexpr auto uart0_tx = tx;
        static constexpr auto microsd_power_enable =
            mcubsp::board::waveshare::esp32p4_nano_bindings::microsd_power_enable;
        static constexpr auto audio_amplifier_enable =
            mcubsp::board::waveshare::esp32p4_nano_bindings::audio_amplifier_enable;
        static constexpr auto wifi_coprocessor_enable =
            mcubsp::board::waveshare::esp32p4_nano_bindings::wifi_coprocessor_enable;
    };

    static constexpr std::size_t pin_slots = 55U;
    static constexpr std::size_t pin_index(core::pin_id pin) noexcept {
        return pin.port == 0U && pin.index < pin_slots ? pin.index : pin_slots;
    }
    static constexpr std::uint32_t ticks_per_second = 360'000'000U;
    using default_execution = io::detail::target_execution<
        io::detail::esp32p4_cycle_hooks<ticks_per_second, 0x346U>>;

    using serial_binding = backend::esp32p4::uart_binding<
        0, pins_type::rx, pins_type::tx>;
    using spi_binding = backend::esp32p4::spi_binding<
        2, pins_type::miso, pins_type::mosi, pins_type::sck, pins_type::cs>;
    using i2c_binding = backend::esp32p4::i2c_binding<
        0, pins_type::scl, pins_type::sda,
        mcubsp::board::waveshare::esp32p4_nano_bindings::development_i2c0_apb_gate_mask,
        mcubsp::board::waveshare::esp32p4_nano_bindings::development_i2c0_reset_mask>;

    template<class Binding, class M, class E> using serial_backend_for = backend::esp32p4::uart_backend<Binding, M, E>;
    template<class Binding, class M, class E> using i2c_backend_for = backend::esp32p4::i2c_backend<Binding, M, E>;
    template<class Binding, class M, class E> using spi_backend_for = backend::esp32p4::spi_master_backend<Binding, M, E, true>;

    template<typename M> using gpio_backend = backend::esp32p4::gpio_backend<M>;
    template<typename M, typename E> using serial_backend =
        backend::esp32p4::uart_backend<serial_binding, M, E>;
    template<typename M, typename E> using spi_backend =
        backend::esp32p4::spi_master_backend<spi_binding, M, E, true>;
    template<typename M, typename E> using i2c_backend =
        backend::esp32p4::i2c_backend<i2c_binding, M, E>;
    static constexpr std::uint32_t dma_available = 0U;
    static constexpr bool adc_available = false;
    static constexpr bool pwm_available = false;

    static constexpr std::array serial_pins{pins_type::rx, pins_type::tx};
    static constexpr std::array spi_pins{
        pins_type::miso, pins_type::mosi, pins_type::sck, pins_type::cs};
    static constexpr std::array i2c_pins{pins_type::scl, pins_type::sda};

    static constexpr auto solve_serial(std::uint32_t baud) noexcept {
        return backend::esp32p4::solve_uart_baud(
            {40'000'000U, baud, 20'000U});
    }
    static constexpr auto solve_spi(std::uint32_t hz) noexcept {
        return backend::esp32p4::solve_spi_timing({40'000'000U, hz, 50'000U});
    }
    static constexpr auto solve_i2c(std::uint32_t hz) noexcept {
        return backend::esp32p4::solve_i2c_timing(
            {40'000'000U, hz, 30'000U, 300U, 300U});
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
};

} // namespace mcubsp::board::waveshare
