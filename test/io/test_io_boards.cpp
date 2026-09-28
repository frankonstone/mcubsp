#include <array>
#include <concepts>
#include <cstdint>
#include <expected>
#include <type_traits>
#include <mcutest/mcutest.hpp>
#include <mcuhw/io.hpp>
#include <mcuhw/io/analog.hpp>
#include <mcuhw/io/pwm.hpp>
#include <mcuhw/io/board/uno_r4_minima.hpp>
#include <mcuhw/io/board/nucleo_h753zi.hpp>
#include <mcuhw/io/board/esp32p4_nano.hpp>
#include <mcuhw/execution/fake_policy.hpp>
#include <mcuhw/testing/recording_mmio.hpp>

namespace {
namespace io = mcuhw::io;
using Uno = io::boards::uno_r4_minima;
using Nucleo = io::boards::nucleo_h753zi;
using Nano = io::boards::esp32p4_nano;
using Policy = mcuhw::execution::fake_policy;
using Mmio = mcuhw::testing::recording_mmio<0x80000, 4096>;
constexpr io::board_config buses{.serial = true, .i2c = true, .spi = true};

class changing_spi_status_mmio : public Mmio {
public:
    changing_spi_status_mmio() : Mmio{0x4000'0000U} {}
    bool become_busy_after_receive{};
    template<mcuhw::testing::mmio_value T>
    auto read(std::uintptr_t address) noexcept {
        auto value = Mmio::read<T>(address);
        if (become_busy_after_receive
            && address == Uno::spi_binding::metadata.base + 4U) {
            become_busy_after_receive = false;
            static_cast<void>(write<std::uint8_t>(
                Uno::spi_binding::metadata.base + 3U, 0x02U));
        }
        return value;
    }
};

std::size_t writes_to(const Mmio& mmio, std::uintptr_t address) {
    std::size_t count{};
    for (const auto& operation : mmio.operations()) {
        if (operation.kind == mcuhw::testing::mmio_operation_kind::write
            && operation.address == address) ++count;
    }
    return count;
}

// Sparse register map for widely separated STM32/P4 peripheral windows. Only
// the P4 configuration-update self-clear is modeled; no transfer success is
// manufactured by this fixture.
class sparse_mmio {
public:
    template<mcuhw::testing::mmio_value T>
    std::expected<T, mcuhw::core::mmio_error> read(std::uintptr_t address) noexcept {
        if (address % alignof(T) != 0U)
            return std::unexpected(mcuhw::core::mmio_error::misaligned);
        for (const auto& cell : cells_)
            if (cell.address == address) return static_cast<T>(cell.value);
        return T{};
    }
    template<mcuhw::testing::mmio_value T>
    std::expected<void, mcuhw::core::mmio_error>
    write(std::uintptr_t address, T value) noexcept {
        if (address % alignof(T) != 0U)
            return std::unexpected(mcuhw::core::mmio_error::misaligned);
        std::uint32_t next = value;
        if (address == Nano::serial_binding::base + 0x98U) next &= ~1U;
        if (address == mcuhw::mcu::esp32p4::spi2_base) next &= ~(1U << 23U);
        for (auto& cell : cells_) {
            if (cell.address == address || cell.address == 0U) {
                cell = {address, next};
                return {};
            }
        }
        return std::unexpected(mcuhw::core::mmio_error::trace_full);
    }
private:
    struct cell { std::uintptr_t address{}; std::uint32_t value{}; };
    std::array<cell, 256> cells_{};
};

TEST(io_boards, traits_keep_physical_pin_and_clock_profiles_explicit, "REQ-0045") {
    static_assert(std::is_same_v<Uno::target_identity, Uno>);
    static_assert(std::is_same_v<Nucleo::target_identity, Nucleo>);
    static_assert(std::is_same_v<Nano::target_identity, Nano>);
    static_assert(Uno::pwm_pin == Uno::pins_type::d3);
    static_assert(Nucleo::pwm_pin == mcuhw::core::pin_id{0U, 8U});
    static_assert(Uno::adc_channels[0].value == 9U);
    static_assert(Nucleo::adc_channels[0].value == 15U);
    static_assert(!Nano::adc_available && !Nano::pwm_available);
    EXPECT_EQ(Uno::pin_index({9U, 15U}), 159U);
    EXPECT_EQ(Uno::pin_index({0U, 16U}), Uno::pin_slots);
    EXPECT_EQ(Nano::pin_index({0U, 54U}), 54U);
    EXPECT_EQ(Nano::pin_index({1U, 0U}), Nano::pin_slots);
    EXPECT_EQ(Uno::solve_pwm(20'000U)->period_counts, 2400U);
    EXPECT_EQ(Nucleo::solve_pwm(20'000U)->period_counts, 3200U);
}

TEST(io_boards, uart_poll_does_not_reuse_stale_end_flag_after_feeding_data, "REQ-0045") {
    Mmio mmio{0x4000'0000U};
    Policy policy;
    auto hw = io::make_board<Uno>(policy, mmio, 1000U);
    ASSERT_TRUE(hw.begin());
    ASSERT_TRUE(hw.serial.begin());
    constexpr auto base = Uno::serial_binding::metadata.base;
    const std::array<std::uint8_t, 1> bytes{0x5AU};
    auto transfer = hw.serial.write_async(bytes);
    ASSERT_TRUE(transfer);
    ASSERT_TRUE(mmio.write<std::uint8_t>(base + 4U, 0x84U));
    mmio.clear_operations();
    hw.poll();
    EXPECT_FALSE(transfer->ready());
    EXPECT_EQ(writes_to(mmio, base + 3U), 1U);
    // TDRE is no longer an enabled source. Only a later TEND can finish.
    ASSERT_TRUE(mmio.write<std::uint8_t>(base + 4U, 0x80U));
    hw.poll();
    EXPECT_FALSE(transfer->ready());
    ASSERT_TRUE(mmio.write<std::uint8_t>(base + 4U, 0x04U));
    hw.poll();
    EXPECT_TRUE(transfer->ready());
    if (!transfer->ready()) static_cast<void>(transfer->cancel());
    EXPECT_TRUE(transfer->take());
    ASSERT_TRUE(hw.shutdown());
}

TEST(io_boards, derived_clock_profiles_share_physical_target_ownership, "REQ-0045") {
    struct custom_profile : Uno {};
    Mmio mmio{0x4000'0000U};
    Policy policy;
    constexpr io::board_config profile{.serial = false};
    auto first = io::make_board<Uno, profile>(policy, mmio, 1000U);
    auto second = io::make_board<custom_profile, profile>(policy, mmio, 1000U);
    ASSERT_TRUE(first.begin());
    EXPECT_TRUE(second.begin().error().code == io::errc::busy);
    ASSERT_TRUE(first.shutdown());
    ASSERT_TRUE(second.begin());
    ASSERT_TRUE(second.shutdown());
}

TEST(io_boards, i2c_poll_obeys_enabled_conditions_and_waits_for_stop, "REQ-0045") {
    Mmio mmio{0x4000'0000U};
    Policy policy;
    auto hw = io::make_board<Uno, buses>(policy, mmio, 1000U);
    ASSERT_TRUE(hw.begin());
    ASSERT_TRUE(hw.i2c.begin());
    constexpr auto base = Uno::i2c_binding::metadata.base;
    const std::array<std::uint8_t, 1> bytes{0xA5U};
    const auto address = mcuhw::driver::make_i2c_address(0x48U);
    ASSERT_TRUE(address);
    auto transfer = hw.i2c.write_async(*address, bytes);
    ASSERT_TRUE(transfer);
    ASSERT_TRUE(mmio.write<std::uint8_t>(base + 9U, 0U));
    mmio.clear_operations();
    hw.poll();
    EXPECT_EQ(writes_to(mmio, base + 0x12U), 0U);
    // TEND alongside TDRE must not advance a phase in the same poll.
    ASSERT_TRUE(mmio.write<std::uint8_t>(base + 9U, 0xC0U));
    hw.poll(); // Address.
    EXPECT_EQ(writes_to(mmio, base + 0x12U), 1U);
    hw.poll(); // Data and switch from TIE to TEIE.
    EXPECT_EQ(writes_to(mmio, base + 0x12U), 2U);
    EXPECT_EQ(*mmio.read<std::uint8_t>(base + 7U), 0x5BU);
    EXPECT_FALSE(transfer->ready());
    hw.poll(); // Enabled TEND requests STOP; stale TDRE cannot send again.
    EXPECT_EQ(writes_to(mmio, base + 0x12U), 2U);
    EXPECT_TRUE((*mmio.read<std::uint8_t>(base + 1U) & 8U) != 0U);
    EXPECT_FALSE(transfer->ready());
    ASSERT_TRUE(mmio.write<std::uint8_t>(base + 9U, 0x08U));
    hw.poll();
    EXPECT_TRUE(transfer->ready());
    if (!transfer->ready()) static_cast<void>(transfer->cancel());
    const auto result = transfer->take();
    EXPECT_TRUE(result);
    if (result) EXPECT_EQ(result->bytes_written, 1U);
    ASSERT_TRUE(hw.shutdown());
}

TEST(io_boards, spi_poll_requires_final_receive_and_current_idle, "REQ-0045") {
    Mmio mmio{0x4000'0000U};
    Policy policy;
    auto hw = io::make_board<Uno, buses>(policy, mmio, 1000U);
    ASSERT_TRUE(hw.begin());
    ASSERT_TRUE(hw.spi.begin());
    constexpr auto base = Uno::spi_binding::metadata.base;
    const std::array<std::uint16_t, 2> transmit{0x12U, 0x34U};
    std::array<std::uint16_t, 2> receive{};
    const mcuhw::driver::spi_segment segment{transmit, receive};
    auto transfer = hw.spi.transaction_async(std::span{&segment, 1U});
    ASSERT_TRUE(transfer);
    // Idle plus empty transmitter cannot complete an unreceived transaction.
    ASSERT_TRUE(mmio.write<std::uint8_t>(base + 3U, 0x20U));
    hw.poll();
    EXPECT_FALSE(transfer->ready());
    ASSERT_TRUE(mmio.write<std::uint8_t>(base + 4U, 0xABU));
    ASSERT_TRUE(mmio.write<std::uint8_t>(base + 3U, 0xA2U));
    hw.poll(); // Receive first frame and feed second frame.
    EXPECT_FALSE(transfer->ready());
    EXPECT_EQ(receive[0], 0xABU);
    ASSERT_TRUE(mmio.write<std::uint8_t>(base + 4U, 0xCDU));
    ASSERT_TRUE(mmio.write<std::uint8_t>(base + 3U, 0x82U));
    hw.poll(); // Last receive does not imply the peripheral is idle.
    EXPECT_EQ(receive[1], 0xCDU);
    EXPECT_FALSE(transfer->ready());
    ASSERT_TRUE(mmio.write<std::uint8_t>(base + 3U, 0U));
    hw.poll();
    EXPECT_TRUE(transfer->ready());
    if (!transfer->ready()) static_cast<void>(transfer->cancel());
    const auto result = transfer->take();
    EXPECT_TRUE(result);
    if (result) EXPECT_EQ(result->frames, 2U);
    ASSERT_TRUE(hw.shutdown());
}

TEST(io_boards, analog_and_pwm_claim_the_same_resources_as_gpio, "REQ-0045") {
    Mmio mmio{0x4000'0000U};
    Policy policy;
    constexpr io::board_config profile{.serial = false, .adc = true, .pwm = true};
    auto hw = io::make_board<Uno, profile>(policy, mmio, 1000U);
    ASSERT_TRUE(hw.begin());
    auto a0 = hw.input(Uno::analog_input_pin);
    ASSERT_TRUE(a0);
    EXPECT_TRUE(hw.analog.begin().error().code == io::errc::pin_in_use);
    ASSERT_TRUE(a0->release());
    ASSERT_TRUE(hw.analog.begin());
    EXPECT_TRUE(hw.input(Uno::analog_input_pin).error().code == io::errc::pin_in_use);
    ASSERT_TRUE(hw.analog.shutdown());
    auto a0_again = hw.input(Uno::analog_input_pin);
    ASSERT_TRUE(a0_again);
    ASSERT_TRUE(a0_again->release());
    auto d3 = hw.input(Uno::pwm_pin);
    ASSERT_TRUE(d3);
    EXPECT_TRUE(hw.pwm.begin().error().code == io::errc::pin_in_use);
    ASSERT_TRUE(d3->release());
    ASSERT_TRUE(hw.pwm.begin(20'000U, io::percent(50U)));
    EXPECT_TRUE(hw.input(Uno::pwm_pin).error().code == io::errc::pin_in_use);
    ASSERT_TRUE(hw.pwm.shutdown());
    auto d3_again = hw.input(Uno::pwm_pin);
    ASSERT_TRUE(d3_again);
    ASSERT_TRUE(d3_again->release());
    ASSERT_TRUE(hw.shutdown());
}

TEST(io_boards, spi_idle_is_rechecked_after_receive_changes_hardware_status, "REQ-0045") {
    changing_spi_status_mmio mmio;
    Policy policy;
    auto hw = io::make_board<Uno, buses>(policy, mmio, 1000U);
    ASSERT_TRUE(hw.begin());
    ASSERT_TRUE(hw.spi.begin());
    constexpr auto base = Uno::spi_binding::metadata.base;
    const std::array<std::uint16_t, 1> transmit{0x12U};
    std::array<std::uint16_t, 1> receive{};
    const mcuhw::driver::spi_segment segment{transmit, receive};
    auto transfer = hw.spi.transaction_async(std::span{&segment, 1U});
    ASSERT_TRUE(transfer);
    ASSERT_TRUE(mmio.write<std::uint8_t>(base + 3U, 0x20U));
    hw.poll();
    ASSERT_TRUE(mmio.write<std::uint8_t>(base + 4U, 0xABU));
    ASSERT_TRUE(mmio.write<std::uint8_t>(base + 3U, 0x80U));
    // Inject a hardware transition between the dispatch snapshot and the
    // completion decision. Reusing the old idle bit would complete too soon.
    mmio.become_busy_after_receive = true;
    hw.poll();
    EXPECT_EQ(receive[0], 0xABU);
    EXPECT_FALSE(transfer->ready());
    ASSERT_TRUE(mmio.write<std::uint8_t>(base + 3U, 0U));
    hw.poll();
    EXPECT_TRUE(transfer->ready());
    if (!transfer->ready()) static_cast<void>(transfer->cancel());
    EXPECT_TRUE(transfer->take());
    ASSERT_TRUE(hw.shutdown());
}

TEST(io_boards, nucleo_gpio_serial_and_spi_lifecycles, "REQ-0045") {
    sparse_mmio mmio;
    Policy policy;
    constexpr io::board_config profile{.serial = true, .spi = true};
    auto hw = io::make_board<Nucleo, profile>(policy, mmio, 1000U);
    ASSERT_TRUE(hw.begin());
    auto led = hw.output(hw.pins.led_builtin, true);
    ASSERT_TRUE(led);
    ASSERT_TRUE(led->toggle());
    ASSERT_TRUE(led->release());
    ASSERT_TRUE(hw.serial.begin());
    ASSERT_TRUE(hw.spi.begin(8'000'000U));
    EXPECT_TRUE(hw.output(hw.pins.tx).error().code == io::errc::pin_in_use);
    EXPECT_TRUE(hw.output(hw.pins.sck).error().code == io::errc::pin_in_use);
    ASSERT_TRUE(hw.shutdown());
}

TEST(io_boards, nano_gpio_serial_and_spi_lifecycles, "REQ-0045") {
    sparse_mmio mmio;
    Policy policy;
    constexpr io::board_config profile{.serial = true, .spi = true};
    auto hw = io::make_board<Nano, profile>(policy, mmio, 1000U);
    ASSERT_TRUE(hw.begin());
    auto pin = hw.output(hw.pins.gpio2, true);
    ASSERT_TRUE(pin);
    ASSERT_TRUE(pin->toggle());
    ASSERT_TRUE(pin->release());
    ASSERT_TRUE(hw.serial.begin());
    ASSERT_TRUE(hw.spi.begin());
    EXPECT_TRUE(hw.output(hw.pins.tx).error().code == io::errc::pin_in_use);
    EXPECT_TRUE(hw.output(hw.pins.sck).error().code == io::errc::pin_in_use);
    ASSERT_TRUE(hw.shutdown());
}
} // namespace
