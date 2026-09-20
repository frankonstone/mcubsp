#include <array>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <span>
#include <utility>

#include <mcutest/mcutest.hpp>

#include <mcuhw/backend/esp32p4/uart.hpp>
#include <mcuhw/board/esp32p4_nano.hpp>
#include <mcuhw/core/mmio.hpp>
#include <mcuhw/driver/uart.hpp>
#include <mcuhw/execution/fake_policy.hpp>

namespace {

constexpr std::uintptr_t uart_base = mcuhw::mcu::esp32p4::uart0_base;
constexpr std::uintptr_t interrupt_status = uart_base + 0x08U;
constexpr std::uintptr_t interrupt_enable = uart_base + 0x0CU;
constexpr std::uintptr_t status_register = uart_base + 0x1CU;

class sparse_mmio {
public:
    template<std::unsigned_integral Value>
        requires(sizeof(Value) == 1 || sizeof(Value) == 2 || sizeof(Value) == 4)
    [[nodiscard]] std::expected<Value, mcuhw::core::mmio_error>
    read(std::uintptr_t address) noexcept {
        if (address % alignof(Value) != 0U) {
            return std::unexpected(mcuhw::core::mmio_error::misaligned);
        }
        return static_cast<Value>(value(address));
    }

    template<std::unsigned_integral Value>
        requires(sizeof(Value) == 1 || sizeof(Value) == 2 || sizeof(Value) == 4)
    [[nodiscard]] std::expected<void, mcuhw::core::mmio_error>
    write(std::uintptr_t address, Value next) noexcept {
        if (address % alignof(Value) != 0U) {
            return std::unexpected(mcuhw::core::mmio_error::misaligned);
        }
        if (address == uart_base && transmitted_size_ < transmitted_.size()) {
            transmitted_[transmitted_size_++] =
                static_cast<std::uint8_t>(next);
        }
        if (address == uart_base + 0x10U) {
            set(interrupt_status,
                value(interrupt_status) & ~static_cast<std::uint32_t>(next));
        }
        set(address, address == uart_base + 0x98U
                         ? 0U : static_cast<std::uint32_t>(next));
        return {};
    }

    void set(std::uintptr_t address, std::uint32_t next) noexcept {
        for (std::size_t index = 0; index < count_; ++index) {
            if (cells_[index].address == address) {
                cells_[index].value = next;
                return;
            }
        }
        if (count_ < cells_.size()) cells_[count_++] = {address, next};
    }

    [[nodiscard]] std::uint32_t value(std::uintptr_t address) const noexcept {
        for (std::size_t index = 0; index < count_; ++index) {
            if (cells_[index].address == address) return cells_[index].value;
        }
        return 0U;
    }

    [[nodiscard]] std::span<const std::uint8_t> transmitted() const noexcept {
        return {transmitted_.data(), transmitted_size_};
    }

private:
    struct cell {
        std::uintptr_t address{};
        std::uint32_t value{};
    };

    std::array<cell, 64> cells_{};
    std::array<std::uint8_t, 256> transmitted_{};
    std::size_t count_{};
    std::size_t transmitted_size_{};
};

using binding = mcuhw::backend::esp32p4::uart_binding<
    0, mcuhw::board::esp32p4_nano::uart0_rx,
    mcuhw::board::esp32p4_nano::uart0_tx>;
using backend_type = mcuhw::backend::esp32p4::uart_backend<
    binding, sparse_mmio, mcuhw::execution::fake_policy>;

constexpr auto baud_115200 = mcuhw::backend::esp32p4::solve_uart_baud({
    .clock_hz = 40'000'000U,
    .target_baud = 115'200U,
    .maximum_error_ppm = 1'000U,
});
static_assert(baud_115200.has_value());
constexpr auto pll_baud_115200 = mcuhw::backend::esp32p4::solve_uart_baud({
    .clock_hz = 80'000'000U,
    .target_baud = 115'200U,
    .maximum_error_ppm = 1'000U,
});
static_assert(pll_baud_115200.has_value());

TEST(esp32p4_uart, solves_fractional_baud_and_rejects_bad_requests,
     "REQ-0011", "REQ-0027", "REQ-0034") {
    ASSERT_TRUE(baud_115200.has_value());
    EXPECT_EQ(baud_115200->divisor, 5'556U);
    EXPECT_EQ(baud_115200->backend_encoding, 1U);
    EXPECT_EQ(baud_115200->achieved_baud, 115'191U);
    EXPECT_EQ(baud_115200->error_ppm, 80U);
    EXPECT_EQ(pll_baud_115200->divisor, 11'111U);
    EXPECT_EQ(pll_baud_115200->clock_select, 2U);
    EXPECT_EQ(pll_baud_115200->achieved_baud, 115'201U);
    EXPECT_EQ(pll_baud_115200->error_ppm, 10U);
    EXPECT_TRUE(mcuhw::backend::esp32p4::solve_uart_baud(
                    {0U, 115'200U, 1'000U}).error()
                == mcuhw::driver::uart_error::invalid_clock);
    EXPECT_TRUE(mcuhw::backend::esp32p4::solve_uart_baud(
                    {48'000'000U, 115'200U, 1'000U}).error()
                == mcuhw::driver::uart_error::invalid_clock);
    EXPECT_TRUE(mcuhw::backend::esp32p4::solve_uart_baud(
                    {40'000'000U, 0U, 1'000U}).error()
                == mcuhw::driver::uart_error::invalid_baud);
    EXPECT_TRUE(mcuhw::backend::esp32p4::solve_uart_baud(
                    {40'000'000U, 115'200U, 0U}).error()
                == mcuhw::driver::uart_error::unattainable_tolerance);
}

TEST(esp32p4_uart, initializes_clocks_reset_format_and_direct_pins,
     "REQ-0011", "REQ-0014", "REQ-0027", "REQ-0034") {
    sparse_mmio mmio;
    mcuhw::execution::fake_policy execution;
    backend_type backend{mmio, execution};
    const auto initialized = backend.initialize(*baud_115200, {
        .data_bits = mcuhw::driver::uart_data_bits::seven,
        .parity = mcuhw::driver::uart_parity::odd,
        .stop_bits = mcuhw::driver::uart_stop_bits::two,
    });
    ASSERT_TRUE(initialized.has_value());

    const auto clocks = binding::clocks;
    EXPECT_TRUE((mmio.value(clocks.system_gate_register)
                 & clocks.system_gate_mask) != 0U);
    EXPECT_TRUE((mmio.value(clocks.apb_gate_register)
                 & clocks.apb_gate_mask) != 0U);
    EXPECT_TRUE((mmio.value(clocks.reset_register)
                 & clocks.apb_reset_mask) == 0U);
    EXPECT_TRUE((mmio.value(clocks.source_register) & (1U << 26U)) != 0U);
    EXPECT_EQ(mmio.value(clocks.source_register) & (0x3U << 24U), 0U);
    EXPECT_EQ(mmio.value(clocks.divider_register) & 0x00FF'FFFFU, 0U);
    EXPECT_EQ(mmio.value(uart_base + 0x14U), 347U | (4U << 20U));
    EXPECT_EQ(mmio.value(0x88U), 0U);
    EXPECT_EQ(mmio.value(uart_base + 0x88U),
              (1U << 24U) | (1U << 25U));
    EXPECT_EQ(mmio.value(uart_base + 0x20U) & 0x3FU,
              1U | (1U << 1U) | (2U << 2U) | (3U << 4U));
    EXPECT_EQ((mmio.value(mcuhw::mcu::esp32p4::io_mux_base
                          + 0x04U + binding::tx_pin.index * 4U)
               >> 12U) & 0x7U, 0U);
    EXPECT_TRUE((mmio.value(mcuhw::mcu::esp32p4::io_mux_base
                            + 0x04U + binding::rx_pin.index * 4U)
                 & (1U << 9U)) != 0U);
    EXPECT_TRUE(backend.shutdown().has_value());
}

TEST(esp32p4_uart, finite_transmit_completes_only_after_tx_done,
     "REQ-0014", "REQ-0016", "REQ-0017", "REQ-0019", "REQ-0027") {
    sparse_mmio mmio;
    mcuhw::execution::fake_policy execution;
    backend_type backend{mmio, execution};
    ASSERT_TRUE(backend.initialize(*baud_115200, {}).has_value());
    constexpr std::array<std::uint8_t, 3> bytes{0x31U, 0x32U, 0x33U};
    auto started = backend.transmit(bytes);
    ASSERT_TRUE(started.has_value());
    auto transfer = std::move(*started);

    EXPECT_FALSE(transfer.ready());
    ASSERT_EQ(mmio.transmitted().size(), bytes.size());
    EXPECT_EQ(mmio.transmitted()[0], bytes[0]);
    EXPECT_EQ(mmio.transmitted()[1], bytes[1]);
    EXPECT_EQ(mmio.transmitted()[2], bytes[2]);
    EXPECT_EQ(mmio.value(interrupt_enable), 1U << 14U);

    mmio.set(interrupt_status, 1U << 14U);
    EXPECT_TRUE(backend.handle_interrupt());
    ASSERT_TRUE(transfer.ready());
    EXPECT_TRUE(transfer.take().has_value());
    EXPECT_TRUE(backend.shutdown().has_value());
}

TEST(esp32p4_uart, finite_receive_reports_count_and_overrun,
     "REQ-0014", "REQ-0016", "REQ-0017", "REQ-0019", "REQ-0027") {
    sparse_mmio mmio;
    mcuhw::execution::fake_policy execution;
    backend_type backend{mmio, execution};
    ASSERT_TRUE(backend.initialize(*baud_115200, {}).has_value());
    std::array<std::uint8_t, 1> bytes{};
    auto started = backend.receive(bytes);
    ASSERT_TRUE(started.has_value());
    auto transfer = std::move(*started);
    mmio.set(uart_base, 0xA5U);
    mmio.set(status_register, 1U);
    mmio.set(interrupt_status, 1U);
    EXPECT_TRUE(backend.handle_interrupt());
    const auto received = transfer.take();
    ASSERT_TRUE(received.has_value());
    EXPECT_EQ(*received, 1U);
    EXPECT_EQ(bytes[0], 0xA5U);

    auto errored_start = backend.receive(bytes);
    ASSERT_TRUE(errored_start.has_value());
    auto errored = std::move(*errored_start);
    mmio.set(interrupt_status, 1U << 4U);
    EXPECT_TRUE(backend.handle_interrupt());
    const auto result = errored.take();
    ASSERT_FALSE(result.has_value());
    EXPECT_TRUE(result.error().cause == mcuhw::driver::uart_error::overrun);
    EXPECT_TRUE(backend.shutdown().has_value());
}

} // namespace
