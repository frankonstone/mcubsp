#include <array>
#include <chrono>
#include <cstdint>
#include <mcutest/mcutest.hpp>
#include <mcubsp/io/board.hpp>
#include <mcubsp/board/arduino/uno_r4_minima.hpp>
#include <mcubsp/execution/fake_policy.hpp>
#include <mcubsp/execution/monotonic_policy.hpp>
#include <mcubsp/testing/recording_mmio.hpp>
namespace {
namespace io = mcubsp::io;
namespace ra = mcubsp::backend::ra4m1;
using Board = mcubsp::board::arduino::uno_r4_minima;
using Mmio = mcubsp::testing::recording_mmio<0x80000, 4096>;
using Policy = mcubsp::execution::fake_policy;
using namespace std::chrono_literals;
using SecondSerial = io::serial_selection<ra::sci_uart_binding<0, {1, 0}, {1, 1}>>;
using FirstPwm = io::pwm_selection<ra::gpt_binding<0, mcubsp::mcu::ra4m1::gpt_output::a, {3, 0}>>;
using SecondPwm = io::pwm_selection<Board::pwm_binding>;
constexpr io::board_config no_defaults{.serial = false};

TEST(io_composition, additional_uart_progress_and_shared_pin_ownership, "REQ-0045", "REQ-0014") {
    Mmio mmio{0x4000'0000U};
    Policy policy;
    auto hw = io::make_board<Board, {}, SecondSerial>(policy, mmio, 1000U);
    ASSERT_TRUE(hw.begin());
    ASSERT_TRUE(hw.serial.begin());
    auto& second = hw.peripheral<0>();
    ASSERT_TRUE(second.begin());
    EXPECT_TRUE(hw.output({1, 0}).error().code == io::errc::pin_in_use);
    std::array<std::uint8_t, 1> data{0x55};
    auto started = second.write_async(data);
    ASSERT_TRUE(started);
    EXPECT_TRUE(hw.shutdown().error().code == io::errc::busy);
    ASSERT_TRUE(started->cancel());
    EXPECT_TRUE(started->take().error().code == io::errc::cancelled);
    ASSERT_TRUE(hw.shutdown());
    EXPECT_EQ(policy.destruction_violations(), 0U);
}
TEST(io_composition, two_independent_pwm_timers_share_runtime, "REQ-0045", "REQ-0014") {
    Mmio mmio{0x4000'0000U};
    Policy policy;
    auto hw = io::make_board<Board, no_defaults, FirstPwm, SecondPwm>(policy, mmio, 1000U);
    ASSERT_TRUE(hw.begin());
    ASSERT_TRUE(hw.peripheral<0>().begin(20'000U, io::percent(25)));
    ASSERT_TRUE(hw.peripheral<1>().begin(10'000U, io::percent(75)));
    EXPECT_TRUE(hw.output({3, 0}).error().code == io::errc::pin_in_use);
    ASSERT_TRUE(hw.shutdown());
}
TEST(io_composition, selected_adc_scan_preserves_binding_order, "REQ-0045", "REQ-0029") {
    using Inputs = ra::adc14_binding<ra::adc_input<1, {0, 1}>, ra::adc_input<0, {0, 0}>>;
    using Selected = io::select<Board, io::analog_selection<Inputs>>;
    static_assert(std::is_same_v<Selected::target_identity, Board>);
    Mmio mmio{0x4000'0000U};
    Policy policy;
    auto hw = io::make_board<Selected, {.serial=false, .adc=true}>(policy, mmio, 1000U);
    ASSERT_TRUE(hw.begin());
    ASSERT_TRUE(hw.analog.begin());
    std::array<std::uint16_t, 2> samples{};
    auto op = hw.analog.sample_async(samples);
    ASSERT_TRUE(op);
    ASSERT_TRUE(mmio.write<std::uint16_t>(0x4005'C000U, 0U));
    ASSERT_TRUE(mmio.write<std::uint16_t>(0x4005'C020U, 123U));
    ASSERT_TRUE(mmio.write<std::uint16_t>(0x4005'C022U, 456U));
    hw.poll();
    ASSERT_TRUE(op->take());
    EXPECT_EQ(samples[0], 456U);
    EXPECT_EQ(samples[1], 123U);
    ASSERT_TRUE(hw.shutdown());
}
struct MillisecondClock {
    static constexpr std::uint32_t ticks_per_second = 1000U;
    std::uint64_t value{};
    std::uint64_t now() const noexcept { return value; }
};
TEST(io_clock, independent_clock_supports_long_deadlines_and_sleep, "REQ-0015", "REQ-0045") {
    Mmio mmio{0x4000'0000U};
    Policy synchronization;
    MillisecondClock clock{0xFFFF'FFFEULL};
    mcubsp::execution::monotonic_policy policy{synchronization, clock};
    auto hw = io::make_board<Board, no_defaults>(policy, mmio, decltype(policy)::ticks_per_second);
    ASSERT_TRUE(hw.begin());
    const auto long_wait = hw.deadline_after(24h);
    ASSERT_TRUE(long_wait);
    EXPECT_TRUE(hw.deadline_after(25 * 24h).error().code == io::errc::invalid_timeout);
    EXPECT_TRUE(hw.deadline_after(-1min).error().code == io::errc::invalid_timeout);
    const auto short_wait = hw.deadline_after(10ms);
    ASSERT_TRUE(short_wait);
    clock.value += 11U; // independent clock continues across simulated sleep/wrap
    EXPECT_TRUE(mcubsp::execution::reached(hw.now(), *short_wait));
    EXPECT_FALSE(mcubsp::execution::reached(hw.now(), *long_wait));
    const auto before = synchronization.now();
    const auto token = policy.prepare_wait();
    policy.wait_once(token, *long_wait);
    EXPECT_EQ(synchronization.now(), before + 1U); // no foreign-clock deadline
    ASSERT_TRUE(hw.shutdown());
}
} // namespace
