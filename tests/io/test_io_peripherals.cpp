#include <array>
#include <chrono>
#include <cstdint>
#include <span>

#include <mcutest/mcutest.hpp>
#include <mcuhw/io.hpp>
#include <mcuhw/io/boards/uno_r4_minima.hpp>
#include <mcuhw/execution/fake_policy.hpp>
#include <mcuhw/testing/recording_mmio.hpp>

namespace {
namespace io = mcuhw::io;
namespace driver = mcuhw::driver;
using Board = io::boards::uno_r4_minima;
using Policy = mcuhw::execution::fake_policy;
using Mmio = mcuhw::testing::recording_mmio<0x80000, 4096>;
using namespace std::chrono_literals;
constexpr io::board_config spi_only{.serial = false, .spi = true};
constexpr io::board_config i2c_only{.serial = false, .i2c = true};
constexpr io::board_config analog_pwm{.serial = false, .adc = true, .pwm = true};
constexpr auto spi_base = Board::spi_binding::metadata.base;
constexpr auto i2c_base = Board::i2c_binding::metadata.base;
constexpr auto adc_base = Board::adc_binding::metadata.base;

bool wrote(const Mmio& mmio, std::uintptr_t address, std::uint64_t value) {
    for (const auto& op : mmio.operations()) {
        if (op.kind == mcuhw::testing::mmio_operation_kind::write
            && op.address == address && op.value == value) return true;
    }
    return false;
}

struct SpiArrival {
    Mmio& mmio;
    unsigned phase{};
    static void advance(void* context, mcuhw::execution::fake_wait_stage stage) noexcept {
        if (stage != mcuhw::execution::fake_wait_stage::after_prepare) return;
        auto& self = *static_cast<SpiArrival*>(context);
        switch (self.phase++) {
        case 0:
            (void)self.mmio.write<std::uint8_t>(spi_base + 3U, 0x22U);
            break;
        case 1:
            (void)self.mmio.write<std::uint16_t>(spi_base + 4U, 0xBEEFU);
            (void)self.mmio.write<std::uint8_t>(spi_base + 3U, 0x82U);
            break;
        case 2:
            (void)self.mmio.write<std::uint8_t>(spi_base + 3U, 0U);
            break;
        default: break;
        }
    }
};

TEST(io_peripherals, spi_scalar_wait_keeps_local_segment_until_wire_idle, "REQ-0045") {
    Mmio mmio{0x4000'0000U};
    Policy policy;
    auto hw = io::make_board<Board, spi_only>(policy, mmio, 1000U);
    ASSERT_TRUE(hw.begin());
    ASSERT_TRUE(hw.spi.begin(1'000'000U, {.frame_width = driver::spi_frame_width::bits16}));
    SpiArrival arrival{mmio};
    policy.set_wait_hook(&SpiArrival::advance, &arrival);
    const auto result = hw.spi.transfer(std::uint16_t{0x1234U}, 10ms);
    policy.set_wait_hook(nullptr);
    ASSERT_TRUE(result);
    EXPECT_EQ(*result, 0xBEEFU);
    EXPECT_TRUE(arrival.phase >= 3U);
    EXPECT_TRUE(wrote(mmio, spi_base + 4U, 0x1234U));
    EXPECT_TRUE(wrote(mmio, spi_base, 0x09U)); // peripheral stopped at completion
    ASSERT_TRUE(hw.shutdown());
    EXPECT_EQ(policy.destruction_violations(), 0U);
}

TEST(io_peripherals, spi_timeout_quiesces_local_buffers_and_reuses_slot, "REQ-0045") {
    Mmio mmio{0x4000'0000U};
    Policy policy;
    auto hw = io::make_board<Board, spi_only>(policy, mmio, 1000U);
    ASSERT_TRUE(hw.begin());
    ASSERT_TRUE(hw.spi.begin());
    mmio.clear_operations();
    EXPECT_TRUE(hw.spi.transfer(std::uint16_t{0x5AU}, -1ms).error().code
                == io::errc::invalid_timeout);
    EXPECT_EQ(mmio.operations().size(), 0U);
    policy.set_wait_hook([](void* context, mcuhw::execution::fake_wait_stage stage) noexcept {
        if (stage == mcuhw::execution::fake_wait_stage::after_prepare) {
            // Model a peripheral that never produces TX/RX-ready or idle.
            (void)static_cast<Mmio*>(context)->write<std::uint8_t>(spi_base + 3U, 0x02U);
        }
    }, &mmio);
    const auto timed_out = hw.spi.transfer(std::uint16_t{0x5AU}, 2ms);
    policy.set_wait_hook(nullptr);
    ASSERT_FALSE(timed_out);
    EXPECT_TRUE(timed_out.error().code == io::errc::timeout);
    const auto too_wide = hw.spi.transfer(std::uint16_t{0x1234U}, 2ms);
    ASSERT_FALSE(too_wide);
    EXPECT_EQ(too_wide.error().native, static_cast<std::uint32_t>(driver::spi_error::invalid_frame));
    // A fresh operation can acquire the same fixed slot after timeout/start failure.
    std::array<std::uint16_t, 1> tx{0x33U}, rx{};
    const std::array segments{driver::spi_segment{tx, rx}};
    auto pending = hw.spi.transaction_async(segments);
    ASSERT_TRUE(pending);
    EXPECT_TRUE(hw.spi.shutdown().error().code == io::errc::busy);
    EXPECT_TRUE(pending->cancel());
    EXPECT_TRUE(pending->take().error().code == io::errc::cancelled);
    ASSERT_TRUE(hw.shutdown());
    EXPECT_EQ(policy.destruction_violations(), 0U);
}

TEST(io_peripherals, i2c_validates_unshifted_addresses_before_hardware_access, "REQ-0045") {
    Mmio mmio{0x4000'0000U};
    Policy policy;
    auto hw = io::make_board<Board, i2c_only>(policy, mmio, 1000U);
    ASSERT_TRUE(hw.begin());
    ASSERT_TRUE(hw.i2c.begin());
    const std::array<std::uint8_t, 1> command{0x20U};
    std::array<std::uint8_t, 1> response{};
    mmio.clear_operations();
    const auto invalid = hw.i2c.write_async({0xA0U}, command);
    ASSERT_FALSE(invalid);
    EXPECT_EQ(invalid.error().native, static_cast<std::uint32_t>(driver::i2c_error::invalid_address));
    EXPECT_FALSE(hw.i2c.read_async({0x07U}, response));
    EXPECT_FALSE(hw.i2c.write_read_async({0x78U}, command, response));
    EXPECT_EQ(mmio.operations().size(), 0U);
    ASSERT_TRUE(hw.shutdown());
    EXPECT_EQ(policy.destruction_violations(), 0U);
}

TEST(io_peripherals, i2c_combined_transfer_borrows_buffers_through_stop, "REQ-0045") {
    Mmio mmio{0x4000'0000U};
    Policy policy;
    auto hw = io::make_board<Board, i2c_only>(policy, mmio, 1000U);
    ASSERT_TRUE(hw.begin());
    ASSERT_TRUE(hw.i2c.begin());
    const std::array<std::uint8_t, 1> command{0x20U};
    std::array<std::uint8_t, 2> response{};
    auto pending = hw.i2c.write_read_async({0x50U}, command, response);
    ASSERT_TRUE(pending);
    const auto poll_status = [&](std::uint8_t status) {
        EXPECT_TRUE(mmio.write<std::uint8_t>(i2c_base + 9U, status));
        hw.poll();
    };
    poll_status(0x80U); // address W
    poll_status(0x80U); // caller's command byte
    poll_status(0x40U); // repeated START
    poll_status(0x80U); // address R
    EXPECT_TRUE(mmio.write<std::uint8_t>(i2c_base + 0x13U, 0U));
    poll_status(0x20U); // dummy read
    EXPECT_TRUE(mmio.write<std::uint8_t>(i2c_base + 0x13U, 0x34U));
    poll_status(0x20U);
    EXPECT_TRUE(mmio.write<std::uint8_t>(i2c_base + 0x13U, 0x12U));
    poll_status(0x20U);
    EXPECT_FALSE(pending->ready());
    poll_status(0x08U); // STOP releases the borrowed buffers
    const auto completed = pending->finish(2ms);
    ASSERT_TRUE(completed);
    EXPECT_EQ(completed->bytes_written, 1U);
    EXPECT_EQ(completed->bytes_read, 2U);
    EXPECT_EQ(response[0], 0x34U);
    EXPECT_EQ(response[1], 0x12U);
    EXPECT_TRUE(wrote(mmio, i2c_base + 0x12U, 0xA0U));
    EXPECT_TRUE(wrote(mmio, i2c_base + 0x12U, 0x20U));
    EXPECT_TRUE(wrote(mmio, i2c_base + 0x12U, 0xA1U));
    ASSERT_TRUE(hw.shutdown());
    EXPECT_EQ(policy.destruction_violations(), 0U);
}

struct AdcArrival {
    Mmio& mmio;
    static void complete(void* context, mcuhw::execution::fake_wait_stage stage) noexcept {
        if (stage != mcuhw::execution::fake_wait_stage::after_prepare) return;
        auto& self = *static_cast<AdcArrival*>(context);
        (void)self.mmio.write<std::uint16_t>(adc_base, 0U); // ADST cleared by converter
        (void)self.mmio.write<std::uint16_t>(adc_base + 0x32U, 0x2345U); // AN009
    }
};

TEST(io_peripherals, analog_invalid_configuration_releases_pin_and_scalar_read_consumes, "REQ-0045") {
    Mmio mmio{0x4000'0000U};
    Policy policy;
    auto hw = io::make_board<Board, analog_pwm>(policy, mmio, 1000U);
    ASSERT_TRUE(hw.begin());
    const auto rejected = hw.analog.begin({.sample_states = 5U});
    ASSERT_FALSE(rejected);
    EXPECT_EQ(rejected.error().native, static_cast<std::uint32_t>(driver::adc_error::invalid_sample_time));
    ASSERT_TRUE(hw.output(Board::analog_input_pin));
    ASSERT_TRUE(hw.analog.begin());
    EXPECT_TRUE(hw.output(Board::analog_input_pin).error().code == io::errc::pin_in_use);
    mmio.clear_operations();
    EXPECT_TRUE(hw.analog.read(-1ms).error().code == io::errc::invalid_timeout);
    EXPECT_EQ(mmio.operations().size(), 0U);
    AdcArrival arrival{mmio};
    policy.set_wait_hook(&AdcArrival::complete, &arrival);
    const auto sample = hw.analog.read(5ms);
    policy.set_wait_hook(nullptr);
    ASSERT_TRUE(sample);
    EXPECT_EQ(*sample, 0x2345U);
    ASSERT_TRUE(hw.analog.shutdown());
    ASSERT_TRUE(hw.output(Board::analog_input_pin));
    ASSERT_TRUE(hw.shutdown());
    EXPECT_EQ(policy.destruction_violations(), 0U);
}

TEST(io_peripherals, pwm_rejects_invalid_duty_before_claim_and_shares_pin_ownership, "REQ-0045") {
    Mmio mmio{0x4000'0000U};
    Policy policy;
    auto hw = io::make_board<Board, analog_pwm>(policy, mmio, 1000U);
    ASSERT_TRUE(hw.begin());
    mmio.clear_operations();
    EXPECT_FALSE(hw.pwm.begin(0U));
    EXPECT_FALSE(hw.pwm.begin(1000U, io::percent(101U)));
    EXPECT_EQ(mmio.operations().size(), 0U);
    ASSERT_TRUE(hw.output(Board::pwm_output_pin));
    ASSERT_TRUE(hw.pwm.begin(1000U, io::percent(25U)));
    EXPECT_TRUE(hw.pwm.running());
    EXPECT_TRUE(hw.output(Board::pwm_output_pin).error().code == io::errc::pin_in_use);
    mmio.clear_operations();
    EXPECT_FALSE(hw.pwm.duty({1U, 0U}));
    EXPECT_FALSE(hw.pwm.duty(io::percent(101U)));
    EXPECT_EQ(mmio.operations().size(), 0U);
    policy.set_isr(true);
    EXPECT_TRUE(hw.pwm.duty(io::percent(50U)).error().code == io::errc::invalid_context);
    EXPECT_TRUE(hw.pwm.stop().error().code == io::errc::invalid_context);
    EXPECT_EQ(mmio.operations().size(), 0U);
    policy.set_isr(false);
    ASSERT_TRUE(hw.pwm.duty(io::percent(0U)));
    ASSERT_TRUE(hw.pwm.duty(io::percent(100U)));
    ASSERT_TRUE(hw.pwm.shutdown());
    ASSERT_TRUE(hw.output(Board::pwm_output_pin));
    ASSERT_TRUE(hw.shutdown());
}

} // namespace
