#pragma once

// @req-file REQ-0053 REQ-0054
#include <cstddef>
#include <mcubsp/backend/nrf52840/gpio.hpp>
#include <mcubsp/backend/nrf52840/i2c.hpp>
#include <mcubsp/backend/nrf52840/io.hpp>
#include <mcubsp/board/others/promicro_nrf52840/bindings.hpp>
#include <mcubsp/io/detail/target_time.hpp>

#ifndef MCUBSP_EXTERNAL_STARTUP
extern "C" std::uint32_t mcubsp_promicro_startup_status;
#endif

namespace mcubsp::board::others {

// Native GPIO, UART/UARTE, TWI/TWIM and SPI/SPIM facade profile.
struct promicro_nrf52840 : promicro_nrf52840_bindings {
    using target_identity = promicro_nrf52840;
    using pins_type = promicro_nrf52840_pins;
    static constexpr std::uint32_t ticks_per_second =
        mcu::nrf52840::cpu_frequency;
    using default_execution = io::detail::target_execution<
        io::detail::cortex_m_cycle_hooks<ticks_per_second>>;
    template <class M> using gpio_backend = backend::nrf52840::gpio_backend<M>;
    static constexpr std::size_t pin_slots = 64U;
    static constexpr std::size_t pin_index(core::pin_id pin) noexcept {
        return mcu::nrf52840::gpio_supported(pin)
                   ? static_cast<std::size_t>(pin.port) * 32U + pin.index
                   : pin_slots;
    }
    template <class M, class E>
    using native_services = backend::nrf52840::io_services<M, E>;
    static constexpr bool serial_available = true;
    using serial_binding =
        backend::nrf52840::uart_binding<backend::nrf52840::peripheral::uart0,
                                        {0, 6}, {0, 8}>;
    static constexpr auto serial_pins = serial_binding::facade_pins;
    template <class Binding, class M, class E>
    using serial_backend_for =
        backend::nrf52840::io_serial_backend<Binding, M, E>;
    template <class M, class E>
    using serial_backend = serial_backend_for<serial_binding, M, E>;
    template <class Binding>
    static constexpr auto solve_serial_binding(std::uint32_t baud) noexcept {
        return backend::nrf52840::solve_uart_baud(Binding::instance,
                                                  {16000000, baud, 10000});
    }
    static constexpr auto solve_serial(std::uint32_t baud) noexcept {
        return solve_serial_binding<serial_binding>(baud);
    }
    template <class M, class B>
    static void poll_serial(M &, B &serial) noexcept {
        (void)serial.poll();
    }
    static constexpr bool i2c_available = true;
    using i2c_binding =
        backend::nrf52840::i2c_binding<0, pins_type::scl, pins_type::sda>;
    static constexpr std::array i2c_pins{pins_type::scl, pins_type::sda};
    template <class Binding, class M, class E>
    using i2c_backend_for =
        typename backend::nrf52840::io_i2c_backend_selector<Binding, M,
                                                            E>::type;
    template <class M, class E>
    using i2c_backend = i2c_backend_for<i2c_binding, M, E>;
    static constexpr auto solve_i2c(std::uint32_t hz) noexcept {
        return backend::nrf52840::solve_i2c_timing(
            {mcu::nrf52840::twi_clock_hz, hz, 20'000U, 300U, 100U});
    }
    template <class Binding>
    static constexpr auto solve_i2c_binding(std::uint32_t hz) noexcept {
        if constexpr (requires { Binding::easy_dma; })
            return backend::nrf52840::solve_twim_timing(
                {16000000, hz, 30000, 300, 100});
        else
            return solve_i2c(hz);
    }
    template <class M, class B> static void poll_i2c(M &, B &bus) noexcept {
        (void)bus.poll();
    }
    static constexpr bool spi_available = true;
    using spi_binding =
        backend::nrf52840::spim_binding<1, {0, 20}, {0, 22}, {0, 24}, {0, 17}>;
    static constexpr std::array spi_pins{
        spi_binding::miso_pin, spi_binding::mosi_pin, spi_binding::clock_pin,
        spi_binding::select_pin};
    template <class Binding, class M, class E>
    using spi_backend_for = backend::nrf52840::io_spi_backend<Binding, M, E>;
    template <class M, class E>
    using spi_backend = spi_backend_for<spi_binding, M, E>;
    template <class Binding>
    static constexpr auto solve_spi_binding(std::uint32_t hz) noexcept {
        return backend::nrf52840::solve_spim_timing(Binding::instance,
                                                    {16000000, hz, 20000});
    }
    static constexpr auto solve_spi(std::uint32_t hz) noexcept {
        return solve_spi_binding<spi_binding>(hz);
    }
    template <class M, class B> static void poll_spi(M &, B &spi) noexcept {
        (void)spi.poll();
    }
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
    template <class M> static void start(M &) noexcept {}
    template <class M> static void stop(M &) noexcept {}
};

} // namespace mcubsp::board::others
