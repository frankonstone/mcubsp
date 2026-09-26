#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include <mcuhw/backend/ra4m1/adc.hpp>
#include <mcuhw/backend/ra4m1/dmac.hpp>
#include <mcuhw/backend/ra4m1/gpio.hpp>
#include <mcuhw/backend/ra4m1/gpt.hpp>
#include <mcuhw/backend/ra4m1/iic.hpp>
#include <mcuhw/backend/ra4m1/sci_uart.hpp>
#include <mcuhw/backend/ra4m1/spi.hpp>
#include <mcuhw/board/arduino_uno_r4_minima.hpp>
#include <mcuhw/io/detail/target_time.hpp>

namespace mcuhw::io::boards {

// Matches board/uno_r4_minima/early_init.cpp: ICLK/PCLKA 48 MHz,
// PCLKB 24 MHz. Startup, vectors and interrupt routing remain application-owned.
struct uno_r4_minima {
    using target_identity = uno_r4_minima;
    struct pins_type {
        static constexpr auto d0 = mcuhw::board::arduino_uno_r4_minima::d0_rx;
        static constexpr auto d1 = mcuhw::board::arduino_uno_r4_minima::d1_tx;
        static constexpr auto d2 = mcuhw::board::arduino_uno_r4_minima::d2;
        static constexpr auto d3 = mcuhw::board::arduino_uno_r4_minima::d3;
        static constexpr auto d4 = mcuhw::board::arduino_uno_r4_minima::d4;
        static constexpr auto d10 = mcuhw::board::arduino_uno_r4_minima::d10_ssl;
        static constexpr auto d11 = mcuhw::board::arduino_uno_r4_minima::d11_mosi;
        static constexpr auto d12 = mcuhw::board::arduino_uno_r4_minima::d12_miso;
        static constexpr auto d13 = mcuhw::board::arduino_uno_r4_minima::d13_sck;
        static constexpr auto led_builtin = d13;
        static constexpr auto a0 = mcuhw::board::arduino_uno_r4_minima::a0_an009;
        static constexpr auto a1 = mcuhw::board::arduino_uno_r4_minima::a1_an000;
        static constexpr auto a2 = mcuhw::board::arduino_uno_r4_minima::a2_an001;
        static constexpr auto a4 = mcuhw::board::arduino_uno_r4_minima::a4_sda;
        static constexpr auto a5 = mcuhw::board::arduino_uno_r4_minima::a5_scl;
        static constexpr auto rx = d0;
        static constexpr auto tx = d1;
        static constexpr auto miso = d12;
        static constexpr auto mosi = d11;
        static constexpr auto sck = d13;
        static constexpr auto cs = d10;
        static constexpr auto sda = a4;
        static constexpr auto scl = a5;
    };

    static constexpr std::size_t pin_slots = 160U;
    static constexpr std::size_t pin_index(core::pin_id pin) noexcept {
        return pin.port < 10U && pin.index < 16U
            ? static_cast<std::size_t>(pin.port) * 16U + pin.index : pin_slots;
    }
    static constexpr std::uint32_t ticks_per_second = 48'000'000U;
    using default_execution = detail::target_execution<
        detail::cortex_m_cycle_hooks<ticks_per_second>>;

    using serial_binding = backend::ra4m1::sci_uart_binding<
        2, pins_type::rx, pins_type::tx>;
    using spi_binding = backend::ra4m1::spi_binding<
        1, pins_type::miso, pins_type::mosi, pins_type::sck, pins_type::cs>;
    using i2c_binding = backend::ra4m1::iic_binding<
        1, pins_type::scl, pins_type::sda>;
    using adc_binding = backend::ra4m1::adc14_binding<
        backend::ra4m1::adc_input<9, pins_type::a0>>;
    using pwm_binding = backend::ra4m1::gpt_binding<
        1, mcu::ra4m1::gpt_output::b, pins_type::d3>;

    template<typename M> using gpio_backend = backend::ra4m1::gpio_backend<M>;
    template<typename M, typename E> using serial_backend =
        backend::ra4m1::sci_uart_backend<serial_binding, M, E>;
    template<typename M, typename E> using spi_backend =
        backend::ra4m1::spi_master_backend<spi_binding, M, E, true>;
    template<typename M, typename E> using i2c_backend =
        backend::ra4m1::iic_controller_backend<i2c_binding, M, E>;
    template<typename M, typename E> using adc_backend =
        backend::ra4m1::adc14_backend<adc_binding, M, E>;
    template<typename M, typename E> using pwm_backend =
        backend::ra4m1::gpt_pwm_backend<pwm_binding, M, E>;
    template<unsigned Channel, typename M, typename E> using dma_backend =
        backend::ra4m1::dmac_channel_backend<Channel, M, E>;
    static constexpr std::uint32_t dma_available = 0x0FU;
    static constexpr bool adc_available = true;
    static constexpr bool pwm_available = true;
    static constexpr auto analog_input_pin = pins_type::a0;
    static constexpr auto pwm_output_pin = pins_type::d3;
    static constexpr auto pwm_pin = pwm_output_pin;

    static constexpr std::array serial_pins{pins_type::rx, pins_type::tx};
    static constexpr std::array spi_pins{
        pins_type::miso, pins_type::mosi, pins_type::sck, pins_type::cs};
    static constexpr std::array i2c_pins{pins_type::scl, pins_type::sda};
    static constexpr std::array adc_pins{analog_input_pin};
    static constexpr std::array pwm_pins{pwm_output_pin};
    static constexpr std::array adc_channels{driver::adc_channel{9U}};

    static constexpr auto solve_serial(std::uint32_t baud) noexcept {
        return driver::solve_uart_baud({48'000'000U, baud, 20'000U});
    }
    static constexpr auto solve_spi(std::uint32_t hz) noexcept {
        return driver::solve_spi_timing({48'000'000U, hz, 50'000U});
    }
    static constexpr auto solve_i2c(std::uint32_t hz) noexcept {
        return driver::solve_i2c_timing({24'000'000U, hz, 30'000U,
                                       hz > 100'000U ? 120U : 300U,
                                       hz > 100'000U ? 60U : 100U});
    }
    static constexpr auto solve_pwm(std::uint32_t hz) noexcept {
        return driver::solve_pwm_timing(
            {48'000'000U, hz, 30'000U, pwm_binding::metadata.counter_width});
    }

    template<typename M> static void start(M&) noexcept {}
    template<typename M> static void stop(M&) noexcept {}

    template<typename M, typename B>
    static void poll_serial(M& mmio, B& peripheral) noexcept {
        static_cast<void>(peripheral.handle_dma_receive_completion());
        static_cast<void>(peripheral.handle_dma_transmit_completion());
        const auto status = mmio.template read<std::uint8_t>(
            serial_binding::metadata.base + 0x04U);
        const auto enabled = mmio.template read<std::uint8_t>(
            serial_binding::metadata.base + 0x02U);
        if (!status || !enabled) return;
        if ((*status & 0x38U) != 0U && (*enabled & 0x40U) != 0U) {
            static_cast<void>(peripheral.handle_receive_error_interrupt());
        } else if ((*status & 0x40U) != 0U && (*enabled & 0x40U) != 0U) {
            static_cast<void>(peripheral.handle_receive_interrupt());
        }
        if ((*status & 0x80U) != 0U && (*enabled & 0x80U) != 0U) {
            static_cast<void>(peripheral.handle_transmit_data_empty_interrupt());
        }
        // Use the enable snapshot from before feeding TDR: an old TEND flag
        // must not complete a byte just written by the TX-empty handler.
        if ((*status & 0x04U) != 0U && (*enabled & 0x04U) != 0U) {
            static_cast<void>(peripheral.handle_transmit_end_interrupt());
        }
    }

    template<typename M, typename B>
    static void poll_spi(M& mmio, B& peripheral) noexcept {
        static_cast<void>(peripheral.handle_dma_receive_completion());
        static_cast<void>(peripheral.handle_dma_transmit_completion());
        const auto status = mmio.template read<std::uint8_t>(
            spi_binding::metadata.base + 0x03U);
        if (!status) return;
        if ((*status & 0x0DU) != 0U) {
            static_cast<void>(peripheral.handle_error_interrupt());
            return;
        }
        if ((*status & 0x80U) != 0U) {
            static_cast<void>(peripheral.handle_receive_interrupt());
        }
        if ((*status & 0x20U) != 0U) {
            static_cast<void>(peripheral.handle_transmit_empty_interrupt());
        }
        if ((*status & 0x02U) == 0U) {
            static_cast<void>(peripheral.handle_idle_interrupt());
        }
    }

    template<typename M, typename B>
    static void poll_i2c(M& mmio, B& peripheral) noexcept {
        const auto status = mmio.template read<std::uint8_t>(
            i2c_binding::metadata.base + 0x09U);
        const auto enabled = mmio.template read<std::uint8_t>(
            i2c_binding::metadata.base + 0x07U);
        if (!status || !enabled) return;
        const auto pending = static_cast<std::uint8_t>(*status & *enabled);
        if ((pending & 0x1BU) != 0U) {
            static_cast<void>(peripheral.handle_error_interrupt());
            return;
        }
        // Dispatch one condition per poll: handlers may change the phase and
        // interrupt enables, making the original status snapshot stale.
        if ((pending & 0x20U) != 0U) {
            static_cast<void>(peripheral.handle_receive_interrupt());
        } else if ((pending & 0x80U) != 0U) {
            static_cast<void>(peripheral.handle_transmit_empty_interrupt());
        } else if ((pending & 0x40U) != 0U) {
            static_cast<void>(peripheral.handle_transmit_end_interrupt());
        }
    }

    template<unsigned Channel, typename M, typename B>
    static void poll_dma(M& mmio, B& peripheral) noexcept {
        static_assert(Channel < 4U);
        const auto status = mmio.template read<std::uint8_t>(
            backend::ra4m1::dmac_detail::metadata(Channel).base + 0x1EU);
        if (status && (*status & 0x80U) == 0U
            && (*status & 0x11U) != 0U) {
            static_cast<void>(peripheral.handle_transfer_end_interrupt());
        }
    }

    template<typename M, typename B>
    static void poll_adc(M&, B& peripheral) noexcept {
        static_cast<void>(peripheral.handle_dma_completion());
        // The scan handler checks ADST has cleared before reading results.
        static_cast<void>(peripheral.handle_scan_end_interrupt());
    }
    template<typename M, typename B>
    static void poll_pwm(M&, B& peripheral) noexcept {
        // The handler checks and clears GTST.TCFPO before publishing an event.
        static_cast<void>(peripheral.handle_overflow_interrupt());
    }
};

} // namespace mcuhw::io::boards
