// SPDX-License-Identifier: Apache-2.0 OR MIT

// @req-file REQ-0011 REQ-0014 REQ-0016 REQ-0017 REQ-0019 REQ-0027 REQ-0034 REQ-0036

#include "support.hpp"

#include <cstddef>
#include <cstdint>
#include <span>
#include <utility>

#include <mcutest/assert.hpp>
#include <mcutest/core.hpp>

#include <mcubsp/backend/esp32p4/uart.hpp>
#include <mcubsp/board/waveshare/esp32p4_nano/bindings.hpp>
#include <mcubsp/core/direct_mmio.hpp>
#include <mcubsp/execution/fake_policy.hpp>

extern "C" void mcubsp_esp32p4_uart_test_link_anchor() noexcept {}

namespace mcubsp::board_test::esp32p4_nano {
namespace {

using binding = backend::esp32p4::uart_binding<
    0, board::waveshare::esp32p4_nano_bindings::uart0_rx, board::waveshare::esp32p4_nano_bindings::uart0_tx>;
using backend_type = backend::esp32p4::uart_backend<
    binding, core::direct_mmio, execution::fake_policy>;

constexpr auto baud_115200 = backend::esp32p4::solve_uart_baud({
    .clock_hz = 40'000'000U,
    .target_baud = 115'200U,
    .maximum_error_ppm = 1'000U,
});
static_assert(baud_115200.has_value());

struct uart_preflight_result {
    bool initialized{};
    bool clock_readback{};
    bool transmit_started{};
    bool transmit_completed{};
};

core::direct_mmio uart_mmio;
execution::fake_policy uart_execution;
backend_type uart{uart_mmio, uart_execution};
uart_preflight_result preflight{};

[[nodiscard]] volatile const std::uint32_t&
register_at(std::uintptr_t address) noexcept {
    return *reinterpret_cast<volatile const std::uint32_t*>(address);
}

} // namespace

void run_uart_preflight() noexcept {
    const auto clocks = binding::clocks;
    const auto initialized = uart.initialize(*baud_115200, {});
    preflight.initialized = initialized.has_value();
    if (!initialized) return;

    preflight.clock_readback =
        (register_at(clocks.system_gate_register)
         & clocks.system_gate_mask) != 0U
        && (register_at(clocks.apb_gate_register)
            & clocks.apb_gate_mask) != 0U
        && (register_at(clocks.source_register) & (1U << 26U)) != 0U
        && (register_at(clocks.source_register) & (0x3U << 24U)) == 0U
        && (register_at(clocks.divider_register) & 0xFFU) == 0U
        && (register_at(binding::base + 0x14U) & 0x00F0'0FFFU)
            == (347U | (4U << 20U));

    constexpr char marker[] =
        "MCUBSP_UART backend=esp32p4 uart=0 baud=115191 completion=tx_done\r\n";
    const auto bytes = std::span<const std::uint8_t>{
        reinterpret_cast<const std::uint8_t*>(marker), sizeof(marker) - 1U};
    auto started = uart.transmit(bytes);
    preflight.transmit_started = started.has_value();
    if (!started) return;
    auto transfer = std::move(*started);
    for (std::size_t attempt = 0U;
         attempt < 2'000'000U && !transfer.ready(); ++attempt) {
        static_cast<void>(uart.handle_interrupt());
    }
    if (!transfer.ready()) return;
    preflight.transmit_completed = transfer.take().has_value();

    // UART0 remains configured at the same rate so the test runner and MCUCOV
    // dump can continue through the board's CH343 USB-to-UART bridge.
}

TEST(esp32p4_uart, uart0_transmits_through_direct_usb_bridge,
     "REQ-0011", "REQ-0014", "REQ-0016", "REQ-0017", "REQ-0019",
     "REQ-0027", "REQ-0034", "REQ-0036") {
    ASSERT_TRUE(preflight.initialized);
    ASSERT_TRUE(preflight.clock_readback);
    ASSERT_TRUE(preflight.transmit_started);
    ASSERT_TRUE(preflight.transmit_completed);
}

} // namespace mcubsp::board_test::esp32p4_nano
