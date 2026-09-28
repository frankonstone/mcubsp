// @req-file REQ-0014 REQ-0018 REQ-0019 REQ-0045 REQ-0046
#include <array>
#include <chrono>
#include <type_traits>
#include <utility>
#include <mcutest/mcutest.hpp>
#include <mcuhw/io.hpp>
#include <mcuhw/io/board/uno_r4_minima.hpp>
#include <mcuhw/device/sensor/tmp102.hpp>
#include <mcuhw/execution/fake_policy.hpp>
#include <mcuhw/testing/recording_mmio.hpp>

namespace {
namespace io = mcuhw::io;
using Board = io::boards::uno_r4_minima;
using Policy = mcuhw::execution::fake_policy;
using Mmio = mcuhw::testing::recording_mmio<0x80000, 4096>;
using namespace std::chrono_literals;
constexpr io::board_config i2c_only{.serial = false, .i2c = true};
constexpr auto base = Board::i2c_binding::metadata.base;

TEST(io_i2c_device, address_claims_exclude_raw_access_and_retain_bus, "REQ-0046") {
    Mmio mmio{0x4000'0000U};
    Policy policy;
    auto hw = io::make_board<Board, i2c_only>(policy, mmio, 1000U);
    EXPECT_FALSE(hw.i2c.device(0x48));
    ASSERT_TRUE(hw.begin());
    ASSERT_TRUE(hw.i2c.begin());
    auto first = hw.i2c.device(0x48);
    auto second = hw.i2c.device(0x49);
    ASSERT_TRUE(first);
    ASSERT_TRUE(second);
    static_assert(!std::is_copy_constructible_v<typename decltype(first)::value_type>);
    EXPECT_TRUE(hw.i2c.device(0x48).error().code == io::errc::busy);
    EXPECT_FALSE(hw.i2c.device(0x07));
    EXPECT_FALSE(hw.i2c.device(0x78));
    const std::array<std::uint8_t, 1U> output{1U};
    std::array<std::uint8_t, 1U> input{};
    mmio.clear_operations();
    EXPECT_TRUE(hw.i2c.write_async({0x48}, output).error().code == io::errc::busy);
    EXPECT_TRUE(hw.i2c.read_async({0x48}, input).error().code == io::errc::busy);
    EXPECT_TRUE(hw.i2c.write_read_async({0x49}, output, input).error().code == io::errc::busy);
    EXPECT_EQ(mmio.operations().size(), 0U);
    EXPECT_TRUE(hw.i2c.shutdown().error().code == io::errc::busy);
    EXPECT_TRUE(hw.shutdown().error().code == io::errc::busy);
    ASSERT_TRUE(first->release());
    ASSERT_TRUE(second->release());
    ASSERT_TRUE(hw.shutdown());
}

TEST(io_i2c_device, moves_keep_operation_storage_stable_and_hold_claim_until_take, "REQ-0046") {
    Mmio mmio{0x4000'0000U};
    Policy policy;
    auto hw = io::make_board<Board, i2c_only>(policy, mmio, 1000U);
    ASSERT_TRUE(hw.begin());
    ASSERT_TRUE(hw.i2c.begin());
    auto acquired = hw.i2c.device(0x48);
    ASSERT_TRUE(acquired);
    const std::array<std::uint8_t, 1U> bytes{1U};
    auto pending = acquired->write_async(bytes);
    ASSERT_TRUE(pending);
    auto moved = std::move(*acquired);
    EXPECT_FALSE(acquired->valid());
    EXPECT_TRUE(acquired->write_async(bytes).error().code == io::errc::not_started);
    EXPECT_TRUE(moved.release().error().code == io::errc::busy);
    ASSERT_TRUE(pending->cancel());
    EXPECT_TRUE(pending->ready());
    EXPECT_TRUE(moved.release().error().code == io::errc::busy);
    EXPECT_TRUE(pending->take().error().code == io::errc::cancelled);
    ASSERT_TRUE(moved.release());
    auto replacement = hw.i2c.device(0x48);
    ASSERT_TRUE(replacement);
    ASSERT_TRUE(replacement->release());
    ASSERT_TRUE(hw.shutdown());
    EXPECT_EQ(policy.destruction_violations(), 0U);
}

TEST(io_i2c_device, raw_operation_prevents_claim_and_terminal_destruction_releases_count, "REQ-0046") {
    Mmio mmio{0x4000'0000U};
    Policy policy;
    auto hw = io::make_board<Board, i2c_only>(policy, mmio, 1000U);
    ASSERT_TRUE(hw.begin());
    ASSERT_TRUE(hw.i2c.begin());
    const std::array<std::uint8_t, 1U> bytes{1U};
    {
        auto pending = hw.i2c.write_async({0x48}, bytes);
        ASSERT_TRUE(pending);
        EXPECT_TRUE(hw.i2c.device(0x48).error().code == io::errc::busy);
        ASSERT_TRUE(pending->cancel());
        EXPECT_TRUE(hw.i2c.device(0x49).error().code == io::errc::busy);
    }
    auto acquired = hw.i2c.device(0x48);
    ASSERT_TRUE(acquired);
    ASSERT_TRUE(acquired->release());
    ASSERT_TRUE(hw.shutdown());
}

TEST(io_i2c_device, rejected_deadlines_and_isr_access_do_not_touch_hardware, "REQ-0046") {
    Mmio mmio{0x4000'0000U};
    Policy policy;
    auto hw = io::make_board<Board, i2c_only>(policy, mmio, 1000U);
    ASSERT_TRUE(hw.begin());
    ASSERT_TRUE(hw.i2c.begin());
    auto acquired = hw.i2c.device(0x48);
    ASSERT_TRUE(acquired);
    const std::array<std::uint8_t, 1U> output{0U};
    std::array<std::uint8_t, 2U> input{};
    policy.set_time(30U);
    mmio.clear_operations();
    EXPECT_TRUE(acquired->write(output, -1ms).error().code == io::errc::invalid_timeout);
    EXPECT_TRUE(acquired->read_until(input, {30U}).error().code == io::errc::timeout);
    EXPECT_TRUE(acquired->write_read_until(output, input, {29U}).error().code == io::errc::timeout);
    policy.set_isr(true);
    EXPECT_TRUE(acquired->write_async(output).error().code == io::errc::invalid_context);
    EXPECT_TRUE(acquired->read_async(input).error().code == io::errc::invalid_context);
    EXPECT_TRUE(acquired->write_read_async(output, input).error().code == io::errc::invalid_context);
    EXPECT_TRUE(acquired->release().error().code == io::errc::invalid_context);
    EXPECT_EQ(mmio.operations().size(), 0U);
    policy.set_isr(false);
    ASSERT_TRUE(acquired->release());
    ASSERT_TRUE(hw.shutdown());
}

TEST(io_i2c_device, combined_transfer_holds_address_through_repeated_start_and_stop, "REQ-0046") {
    Mmio mmio{0x4000'0000U};
    Policy policy;
    auto hw = io::make_board<Board, i2c_only>(policy, mmio, 1000U);
    ASSERT_TRUE(hw.begin());
    ASSERT_TRUE(hw.i2c.begin());
    auto acquired = hw.i2c.device(0x48);
    ASSERT_TRUE(acquired);
    const std::array<std::uint8_t, 1U> output{0U};
    std::array<std::uint8_t, 2U> input{};
    auto pending = acquired->write_read_async(output, input);
    ASSERT_TRUE(pending);
    const auto status = [&](std::uint8_t value) {
        EXPECT_TRUE(mmio.write<std::uint8_t>(base + 9U, value));
        hw.poll();
    };
    status(0x80U); // address write
    status(0x80U); // register pointer
    status(0x40U); // repeated START
    status(0x80U); // address read
    ASSERT_TRUE(mmio.write<std::uint8_t>(base + 0x13U, 0U));
    status(0x20U); // dummy receive
    ASSERT_TRUE(mmio.write<std::uint8_t>(base + 0x13U, 0x19U));
    status(0x20U);
    ASSERT_TRUE(mmio.write<std::uint8_t>(base + 0x13U, 0x00U));
    status(0x20U);
    EXPECT_FALSE(pending->ready());
    EXPECT_TRUE(acquired->release().error().code == io::errc::busy);
    status(0x08U); // STOP
    EXPECT_TRUE(pending->ready());
    EXPECT_TRUE(acquired->release().error().code == io::errc::busy);
    const auto result = pending->take();
    ASSERT_TRUE(result);
    EXPECT_EQ(result->bytes_written, 1U);
    EXPECT_EQ(result->bytes_read, 2U);
    EXPECT_EQ(input[0], 0x19U);
    EXPECT_EQ(input[1], 0U);
    ASSERT_TRUE(acquired->release());
    ASSERT_TRUE(hw.shutdown());
}

TEST(io_i2c_device, blocking_timeout_consumes_operation_before_endpoint_release, "REQ-0046") {
    Mmio mmio{0x4000'0000U};
    Policy policy;
    auto hw = io::make_board<Board, i2c_only>(policy, mmio, 1000U);
    ASSERT_TRUE(hw.begin());
    ASSERT_TRUE(hw.i2c.begin());
    auto acquired = hw.i2c.device(0x48);
    ASSERT_TRUE(acquired);
    std::array<std::uint8_t, 2U> input{};
    const auto result = acquired->read(input, 3ms);
    EXPECT_FALSE(result);
    EXPECT_TRUE(result.error().code == io::errc::timeout);
    EXPECT_EQ(policy.now(), 3U);
    ASSERT_TRUE(acquired->release());
    ASSERT_TRUE(hw.shutdown());
    EXPECT_EQ(policy.destruction_violations(), 0U);
}

TEST(io_i2c_device, tmp102_composes_with_claimed_address_and_real_repeated_start_backend,
     "REQ-0046") {
    Mmio mmio{0x4000'0000U};
    Policy policy;
    auto hw = io::make_board<Board, i2c_only>(policy, mmio, 1000U);
    ASSERT_TRUE(hw.begin());
    ASSERT_TRUE(hw.i2c.begin());
    auto acquired = hw.i2c.device(0x48);
    ASSERT_TRUE(acquired);
    struct Arrival {
        Mmio& mmio;
        unsigned phase{};
        static void advance(void* context, mcuhw::execution::fake_wait_stage stage) noexcept {
            if (stage != mcuhw::execution::fake_wait_stage::after_prepare) return;
            auto& self = *static_cast<Arrival*>(context);
            const auto enabled = self.mmio.read<std::uint8_t>(base + 7U);
            if (!enabled || *enabled == 0U) return; // terminal inspection is not a bus event
            constexpr std::array<std::uint8_t, 8U> statuses{
                0x80U, 0x80U, 0x40U, 0x80U, 0x20U, 0x20U, 0x20U, 0x08U};
            const auto step = self.phase % statuses.size();
            const bool temperature_register = self.phase >= statuses.size();
            const std::uint8_t byte = step == 5U
                ? (temperature_register ? 0xE7U : 0x60U)
                : (step == 6U && !temperature_register ? 0xA0U : 0U);
            (void)self.mmio.write<std::uint8_t>(base + 0x13U, byte);
            (void)self.mmio.write<std::uint8_t>(base + 9U, statuses[step]);
            ++self.phase;
        }
    } arrival{mmio};
    policy.set_wait_hook(&Arrival::advance, &arrival);
    {
        mcuhw::device::tmp102 sensor{std::move(*acquired)};
        ASSERT_TRUE(sensor.begin(20ms));
        const auto reading = sensor.read(20ms);
        ASSERT_TRUE(reading);
        EXPECT_EQ(reading->celsius_sixteenths, -400);
        EXPECT_EQ(arrival.phase, 16U);
        EXPECT_TRUE(hw.i2c.device(0x48).error().code == io::errc::busy);
        EXPECT_TRUE(hw.shutdown().error().code == io::errc::busy);
    }
    policy.set_wait_hook(nullptr);
    ASSERT_TRUE(hw.i2c.device(0x48)); // sensor destruction released its lease
    ASSERT_TRUE(hw.shutdown());
    EXPECT_EQ(policy.destruction_violations(), 0U);
}
} // namespace
