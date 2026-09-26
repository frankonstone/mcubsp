#include <array>
#include <chrono>
#include <cstdint>
#include <limits>
#include <utility>
#include <mcutest/mcutest.hpp>
#include <mcuhw/io.hpp>
#include <mcuhw/io/boards/uno_r4_minima.hpp>
#include <mcuhw/execution/fake_policy.hpp>
#include <mcuhw/testing/recording_mmio.hpp>

namespace {
namespace io = mcuhw::io;
using Board = io::boards::uno_r4_minima;
using Policy = mcuhw::execution::fake_policy;
using Mmio = mcuhw::testing::recording_mmio<0x80000, 4096>;
using namespace std::chrono_literals;
constexpr io::board_config gpio_only{.serial = false};
constexpr io::board_config buses{.serial = true, .i2c = true, .spi = true, .dma = io::channels<0, 1>};
constexpr auto sci = Board::serial_binding::metadata.base;

TEST(io, configuration_removes_backend_storage, "REQ-0045") {
    using Small = io::board<Board, gpio_only, Policy, Mmio, true, true>;
    using Large = io::board<Board, buses, Policy, Mmio, true, true>;
    static_assert(!std::is_move_constructible_v<Small>);
    EXPECT_TRUE(sizeof(Large) > sizeof(Small) + 300U);
    EXPECT_TRUE(sizeof(Small) < 256U);
}

TEST(io, construction_is_passive_and_runtime_ownership_is_shared, "REQ-0045") {
    Mmio mmio{0x4000'0000U};
    Policy policy;
    auto hw = io::make_board<Board, gpio_only>(policy, mmio, 1000U);
    auto second = io::make_board<Board, buses>(policy, mmio, 1000U);
    EXPECT_EQ(mmio.operations().size(), 0U);
    EXPECT_TRUE(hw.output(hw.pins.d2).error().code == io::errc::not_started);
    ASSERT_TRUE(hw.begin());
    EXPECT_TRUE(second.begin().error().code == io::errc::busy);
    EXPECT_TRUE(hw.begin().error().code == io::errc::already_started);
    ASSERT_TRUE(hw.shutdown());
    ASSERT_TRUE(second.begin());
    ASSERT_TRUE(second.shutdown());
}

TEST(io, gpio_lease_moves_and_prevents_runtime_shutdown, "REQ-0045") {
    Mmio mmio{0x4000'0000U};
    Policy policy;
    auto hw = io::make_board<Board, gpio_only>(policy, mmio, 1000U);
    ASSERT_TRUE(hw.begin());
    auto opened = hw.output(hw.pins.d2, true);
    ASSERT_TRUE(opened);
    auto pin = std::move(*opened);
    EXPECT_TRUE(opened->write(false).error().code == io::errc::not_started);
    EXPECT_TRUE(hw.output(hw.pins.d2).error().code == io::errc::pin_in_use);
    EXPECT_TRUE(hw.shutdown().error().code == io::errc::busy);
    ASSERT_TRUE(pin.toggle());
    ASSERT_TRUE(pin.release());
    ASSERT_TRUE(hw.shutdown());
}

TEST(io, spi_and_gpio_share_pin_claims_and_release_them, "REQ-0045") {
    Mmio mmio{0x4000'0000U};
    Policy policy;
    auto hw = io::make_board<Board, buses>(policy, mmio, 1000U);
    ASSERT_TRUE(hw.begin());
    auto led = hw.output(hw.pins.led_builtin);
    ASSERT_TRUE(led);
    EXPECT_TRUE(hw.spi.begin().error().code == io::errc::pin_in_use);
    ASSERT_TRUE(led->release());
    ASSERT_TRUE(hw.spi.begin());
    EXPECT_TRUE(hw.output(hw.pins.d13).error().code == io::errc::pin_in_use);
    ASSERT_TRUE(hw.spi.shutdown());
    ASSERT_TRUE(hw.output(hw.pins.d13)); // temporary lease releases immediately
    ASSERT_TRUE(hw.shutdown());
}

TEST(io, failed_configuration_rolls_back_gpio_and_bus_claims, "REQ-0045") {
    Mmio mmio{0x4000'0000U};
    Policy policy;
    auto hw = io::make_board<Board, buses>(policy, mmio, 1000U);
    ASSERT_TRUE(hw.begin());
    EXPECT_FALSE(hw.input(hw.pins.d2, io::pull::down)); // unsupported on RA4M1
    ASSERT_TRUE(hw.output(hw.pins.d2));
    EXPECT_FALSE(hw.serial.begin(0));
    ASSERT_TRUE(hw.output(hw.pins.d0));
    ASSERT_TRUE(hw.serial.begin());
    ASSERT_TRUE(hw.shutdown());
}

TEST(io, duration_conversion_checks_overflow_rounds_up_and_wraps, "REQ-0045") {
    EXPECT_EQ(*io::detail::duration_ticks(1us, 1000U), 1U);
    EXPECT_EQ(*io::detail::duration_ticks(0ms, 48'000'000U), 0U);
    EXPECT_FALSE(io::detail::duration_ticks(-1ms, 1000U));
    EXPECT_FALSE(io::detail::duration_ticks(std::chrono::seconds::max(), 48'000'000U));
    EXPECT_FALSE(io::detail::duration_ticks(1ms, 0U));
    Mmio mmio{0x4000'0000U};
    Policy policy;
    auto hw = io::make_board<Board, gpio_only>(policy, mmio, 1000U);
    ASSERT_TRUE(hw.begin());
    policy.set_time(0xFFFF'FFFEU);
    ASSERT_TRUE(hw.delay(4ms));
    EXPECT_EQ(policy.now(), 2U);
    ASSERT_TRUE(hw.shutdown());
}

TEST(io, blocking_invalid_timeout_and_isr_never_start_hardware, "REQ-0045") {
    Mmio mmio{0x4000'0000U};
    Policy policy;
    auto hw = io::make_board<Board>(policy, mmio, 1000U);
    ASSERT_TRUE(hw.begin());
    ASSERT_TRUE(hw.serial.begin());
    const std::array<std::uint8_t, 1> bytes{0x5AU};
    mmio.clear_operations();
    EXPECT_TRUE(hw.serial.write(bytes, -1ms).error().code == io::errc::invalid_timeout);
    EXPECT_EQ(mmio.operations().size(), 0U);
    policy.set_isr(true);
    EXPECT_TRUE(hw.serial.write(bytes, 10ms).error().code == io::errc::invalid_context);
    EXPECT_TRUE(hw.delay(1ms).error().code == io::errc::invalid_context);
    EXPECT_EQ(mmio.operations().size(), 0U);
    policy.set_isr(false);
    ASSERT_TRUE(hw.shutdown());
}

TEST(io, serial_polling_and_finish_consume_completion, "REQ-0045") {
    Mmio mmio{0x4000'0000U};
    Policy policy;
    auto hw = io::make_board<Board>(policy, mmio, 1000U);
    ASSERT_TRUE(hw.begin());
    ASSERT_TRUE(hw.serial.begin());
    // Recording MMIO retains status bits; each progress step advances one byte.
    ASSERT_TRUE(mmio.write<std::uint8_t>(sci + 4U, 0x84U));
    const std::array<std::uint8_t, 3> bytes{1, 2, 3};
    ASSERT_TRUE(hw.serial.write(bytes, 20ms));
    auto second = hw.serial.write_async(bytes);
    ASSERT_TRUE(second);
    auto moved = std::move(*second);
    EXPECT_FALSE(moved.ready());
    ASSERT_TRUE(moved.finish(20ms));
    EXPECT_TRUE(moved.take().error().code == io::errc::invalid_operation);
    ASSERT_TRUE(hw.shutdown());
}

TEST(io, isr_poll_and_cancellation_leave_active_operations_untouched, "REQ-0045") {
    Mmio mmio{0x4000'0000U};
    Policy policy;
    auto hw = io::make_board<Board, buses>(policy, mmio, 1000U);
    ASSERT_TRUE(hw.begin());
    ASSERT_TRUE(hw.serial.begin());
    auto channel = hw.dma.channel<0>();
    ASSERT_TRUE(channel);
    const std::array<std::uint8_t, 1> bytes{1};
    auto operation = hw.serial.write_async(bytes);
    ASSERT_TRUE(operation);
    mmio.clear_operations();
    policy.set_isr(true);
    hw.poll();
    hw.dma.poll();
    EXPECT_TRUE(operation->cancel().error().code == io::errc::invalid_context);
    EXPECT_TRUE(operation->take().error().code == io::errc::invalid_context);
    EXPECT_FALSE(operation->ready());
    EXPECT_EQ(mmio.operations().size(), 0U);
    policy.set_isr(false);
    ASSERT_TRUE(operation->cancel());
    EXPECT_TRUE(operation->take().error().code == io::errc::cancelled);
    ASSERT_TRUE(channel->release());
    ASSERT_TRUE(hw.shutdown());
    EXPECT_EQ(policy.destruction_violations(), 0U);
}

TEST(io, delay_progresses_async_operations_and_terminal_handle_keeps_runtime_alive, "REQ-0045") {
    Mmio mmio{0x4000'0000U};
    Policy policy;
    auto hw = io::make_board<Board>(policy, mmio, 1000U);
    ASSERT_TRUE(hw.begin());
    ASSERT_TRUE(hw.serial.begin());
    ASSERT_TRUE(mmio.write<std::uint8_t>(sci + 4U, 0x84U));
    const std::array<std::uint8_t, 1> bytes{1};
    auto operation = hw.serial.write_async(bytes);
    ASSERT_TRUE(operation);
    ASSERT_TRUE(hw.delay(5ms));
    EXPECT_TRUE(operation->ready());
    // shutdown may stop the port, but must retain storage for terminal handles.
    EXPECT_TRUE(hw.shutdown().error().code == io::errc::busy);
    ASSERT_TRUE(operation->take());
    ASSERT_TRUE(hw.shutdown());
}

TEST(io, timeout_and_cancellation_release_slots_and_quiesce, "REQ-0045") {
    Mmio mmio{0x4000'0000U};
    Policy policy;
    auto hw = io::make_board<Board>(policy, mmio, 1000U);
    ASSERT_TRUE(hw.begin());
    ASSERT_TRUE(hw.serial.begin());
    const std::array<std::uint8_t, 1> bytes{1};
    const auto timeout = hw.serial.write(bytes, 2ms);
    EXPECT_TRUE(timeout.error().code == io::errc::timeout);
    auto operation = hw.serial.write_async(bytes);
    ASSERT_TRUE(operation);
    EXPECT_TRUE(hw.serial.shutdown().error().code == io::errc::busy);
    ASSERT_TRUE(operation->cancel());
    EXPECT_TRUE(operation->take().error().code == io::errc::cancelled);
    ASSERT_TRUE(hw.shutdown());
    EXPECT_EQ(policy.destruction_violations(), 0U);
}

TEST(io, dma_channels_are_exclusive_and_released_tags_are_rejected, "REQ-0045") {
    Mmio mmio{0x4000'0000U};
    Policy policy;
    auto hw = io::make_board<Board, buses>(policy, mmio, 1000U);
    ASSERT_TRUE(hw.begin());
    ASSERT_TRUE(hw.serial.begin());
    auto channel = hw.dma.channel<0>();
    ASSERT_TRUE(channel);
    EXPECT_TRUE(hw.dma.channel<0>().error().code == io::errc::busy);
    EXPECT_TRUE(hw.shutdown().error().code == io::errc::busy);
    ASSERT_TRUE(channel->release());
    const std::array<std::uint8_t, 1> bytes{1};
    const auto started = hw.serial.write_async(bytes, io::use_dma(*channel));
    EXPECT_FALSE(started);
    EXPECT_TRUE(started.error().code == io::errc::not_started);
    ASSERT_TRUE(hw.shutdown());
}
} // namespace

int main() {
    mcutest::Runner<mcutest::JsonOutput> runner;
    return mcutest::run_json_output(runner);
}
