#include <array>
#include <cstdint>
#include <expected>
#include <span>
#include <utility>

#include <mcuhw/backend/ra4m1/iic.hpp>
#include <mcuhw/backend/ra4m1/sci_uart.hpp>
#include <mcuhw/board/arduino_uno_r4_minima.hpp>
#include <mcuhw/core/mmio.hpp>
#include <mcuhw/driver/i2c.hpp>
#include <mcuhw/driver/uart.hpp>
#include <mcuhw/execution/fake_policy.hpp>

namespace {

// A compile-only MMIO policy keeps this example valid on every architecture
// preset without accessing RA4M1 addresses at run time.
struct example_mmio {
    template<std::unsigned_integral Value>
    [[nodiscard]] std::expected<Value, mcuhw::core::mmio_error>
    read(std::uintptr_t) noexcept {
        return Value{};
    }

    template<std::unsigned_integral Value>
    [[nodiscard]] std::expected<void, mcuhw::core::mmio_error>
    write(std::uintptr_t, Value) noexcept {
        return {};
    }
};

using uart_binding = mcuhw::backend::ra4m1::sci_uart_binding<
    2, mcuhw::board::arduino_uno_r4_minima::d0_rx,
    mcuhw::board::arduino_uno_r4_minima::d1_tx>;
using i2c_binding = mcuhw::backend::ra4m1::iic_binding<
    1, mcuhw::board::arduino_uno_r4_minima::a5_scl,
    mcuhw::board::arduino_uno_r4_minima::a4_sda>;
using execution_type = mcuhw::execution::fake_policy;
using uart_backend = mcuhw::backend::ra4m1::sci_uart_backend<
    uart_binding, example_mmio, execution_type>;
using i2c_backend = mcuhw::backend::ra4m1::iic_controller_backend<
    i2c_binding, example_mmio, execution_type>;

constexpr auto uart_timing = mcuhw::backend::ra4m1::solve_uart_baud({
    .clock_hz = 24'000'000U,
    .target_baud = 115'200U,
    .maximum_error_ppm = 20'000U,
});
constexpr auto i2c_timing = mcuhw::backend::ra4m1::solve_i2c_timing({
    .clock_hz = 24'000'000U,
    .target_hz = 400'000U,
    .maximum_error_ppm = 30'000U,
    .rise_time_ns = 120U,
    .fall_time_ns = 60U,
});
static_assert(uart_timing.has_value());
static_assert(i2c_timing.has_value());

} // namespace

// @req REQ-0024 REQ-0026 REQ-0027 REQ-0038
int mcuhw_uart_i2c_example() {
    example_mmio mmio;
    execution_type execution;

    uart_backend uart_hardware{mmio, execution};
    mcuhw::driver::uart uart{uart_hardware};
    if (!uart.initialize(*uart_timing)) {
        return 1;
    }
    const std::array<std::uint8_t, 2> message{'O', 'K'};
    auto tx_started = uart.transmit(message);
    if (!tx_started) {
        return 2;
    }
    auto tx = std::move(*tx_started);
    static_cast<void>(tx.cancel());
    static_cast<void>(tx.take());
    if (!uart.shutdown()) {
        return 3;
    }

    i2c_backend i2c_hardware{mmio, execution};
    mcuhw::driver::i2c_controller bus{i2c_hardware};
    if (!bus.initialize(*i2c_timing)) {
        return 4;
    }
    mcuhw::driver::i2c_device<decltype(bus), 0x50U> device{bus};
    mcuhw::driver::i2c_register_request<2> request;
    auto read_started = device.read_register(0x20U, request, 2U);
    if (!read_started) {
        return 5;
    }
    auto read = std::move(*read_started);
    static_cast<void>(read.cancel());
    static_cast<void>(read.take());
    return bus.shutdown() ? 0 : 6;
}
