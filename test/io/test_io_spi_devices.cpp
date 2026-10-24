#include <array>
#include <chrono>
#include <cstdint>
#include <utility>
#include <mcutest/mcutest.hpp>
#include <mcubsp/io.hpp>
#include <mcubsp/io/board/uno_r4_minima.hpp>
#include <mcubsp/execution/fake_policy.hpp>
#include <mcubsp/testing/recording_mmio.hpp>

namespace {
namespace io = mcubsp::io;
namespace driver = mcubsp::driver;
using Board = io::boards::uno_r4_minima;
using Policy = mcubsp::execution::fake_policy;
using Recording = mcubsp::testing::recording_mmio<0x80000, 4096>;
constexpr io::board_config configuration{.serial = false, .spi = true};
constexpr auto base = Board::spi_binding::metadata.base;
using namespace std::chrono_literals;

class SpiModel : public Recording {
public:
    SpiModel() : Recording{0x4000'0000U} {}
    Policy* ticking{};
    bool stall{};
    bool fail_configuration{};
    bool fail_deselect{};
    bool fail_stop{};
    template<mcubsp::testing::mmio_value T>
    std::expected<T, mcubsp::core::mmio_error> read(std::uintptr_t address) noexcept {
        auto recorded = Recording::read<T>(address);
        if (!recorded) return recorded;
        if (address == base + 3U) {
            if (ticking) ticking->advance();
            return static_cast<T>(stall ? 0x02U : 0xA0U);
        }
        if (address == base + 4U) return static_cast<T>(0x5AU);
        return recorded;
    }
    template<mcubsp::testing::mmio_value T>
    std::expected<void, mcubsp::core::mmio_error> write(std::uintptr_t address, T value) noexcept {
        if (fail_stop && address == base && value == 0U) {
            fail_stop = false;
            return std::unexpected(mcubsp::core::mmio_error::out_of_range);
        }
        constexpr auto pin = Board::pins_type::d10;
        if (fail_deselect && address == 0x4004'0008U + pin.port * 0x20U
            && value == (1U << pin.index)) {
            fail_deselect = false;
            return std::unexpected(mcubsp::core::mmio_error::out_of_range);
        }
        if (address == base + 0x0AU && fail_configuration) {
            fail_configuration = false;
            return std::unexpected(mcubsp::core::mmio_error::out_of_range);
        }
        return Recording::write<T>(address, value);
    }
};
bool wrote(const Recording& mmio, std::uintptr_t address, std::uint64_t value) {
    for (const auto& op : mmio.operations())
        if (op.kind == mcubsp::testing::mmio_operation_kind::write
            && op.address == address && op.value == value) return true;
    return false;
}
constexpr auto select_address(mcubsp::core::pin_id pin) {
    return 0x4004'0008U + pin.port * 0x20U;
}

TEST(io_spi_devices, device_claims_are_exclusive_and_keep_controller_alive, "REQ-0046") {
    SpiModel mmio;
    Policy policy;
    auto hw = io::make_board<Board, configuration>(policy, mmio, 1000U);
    ASSERT_TRUE(hw.begin());
    ASSERT_TRUE(hw.spi.begin());
    auto first = hw.spi.device({.cs = hw.pins.d10});
    ASSERT_TRUE(first);
    auto second = hw.spi.device({.cs = hw.pins.d2});
    ASSERT_TRUE(second);
    EXPECT_TRUE(hw.spi.device({.cs = hw.pins.d10}).error().code == io::errc::pin_in_use);
    EXPECT_TRUE(hw.output(hw.pins.d2).error().code == io::errc::pin_in_use);
    EXPECT_TRUE(hw.spi.device({.cs = hw.pins.sck}).error().code == io::errc::pin_in_use);
    EXPECT_TRUE(hw.spi.shutdown().error().code == io::errc::busy);
    // The default convenience connection cannot bypass a device's CS lease.
    EXPECT_TRUE(hw.spi.transfer(0x12U, 1ms).error().code == io::errc::pin_in_use);
    auto moved = std::move(*first);
    EXPECT_FALSE(first->valid());
    EXPECT_TRUE(first->transfer(0x12U).error().code == io::errc::not_started);
    ASSERT_TRUE(moved.release());
    auto reclaimed = hw.output(hw.pins.d10);
    ASSERT_TRUE(reclaimed);
    ASSERT_TRUE(reclaimed->release());
    ASSERT_TRUE(second->release());
    ASSERT_TRUE(hw.shutdown());
}

TEST(io_spi_devices, transactions_switch_clock_mode_and_select_only_their_device, "REQ-0046") {
    SpiModel mmio;
    Policy policy;
    auto hw = io::make_board<Board, configuration>(policy, mmio, 1000U);
    ASSERT_TRUE(hw.begin());
    ASSERT_TRUE(hw.spi.begin());
    auto first = hw.spi.device({.cs = hw.pins.d10, .frequency = 6'000'000U});
    auto second = hw.spi.device({.cs = hw.pins.d2, .frequency = 2'000'000U, .mode = io::spi_mode::mode3});
    ASSERT_TRUE(first); ASSERT_TRUE(second);
    const std::array<std::uint8_t, 2> tx{0xA5U, 0x12U};
    std::array<std::uint8_t, 2> rx{};
    mmio.clear_operations();
    ASSERT_TRUE(first->transaction_immediate(tx, rx, {100U}));
    EXPECT_EQ(rx[0], 0x5AU);
    EXPECT_TRUE(wrote(mmio, select_address(hw.pins.d10), (1U << hw.pins.d10.index) << 16U));
    EXPECT_TRUE(wrote(mmio, select_address(hw.pins.d10), 1U << hw.pins.d10.index));
    mmio.clear_operations();
    ASSERT_TRUE(second->transaction_immediate(tx, rx, {100U}));
    EXPECT_EQ(*mmio.read<std::uint8_t>(base + 0x0AU), Board::solve_spi(2'000'000U)->spbr);
    EXPECT_EQ(*mmio.read<std::uint16_t>(base + 0x10U) & 3U, 3U);
    EXPECT_TRUE(wrote(mmio, select_address(hw.pins.d2), (1U << hw.pins.d2.index) << 16U));
    EXPECT_TRUE(wrote(mmio, select_address(hw.pins.d2), 1U << hw.pins.d2.index));
    EXPECT_FALSE(wrote(mmio, select_address(hw.pins.d10), (1U << hw.pins.d10.index) << 16U));
    mmio.clear_operations();
    ASSERT_TRUE(first->transaction_immediate(tx, rx, {100U}));
    EXPECT_EQ(*mmio.read<std::uint8_t>(base + 0x0AU), Board::solve_spi(6'000'000U)->spbr);
    EXPECT_EQ(*mmio.read<std::uint16_t>(base + 0x10U) & 3U, 0U);
    // Consecutive commands to the same chip keep the precomputed setup.
    mmio.clear_operations();
    ASSERT_TRUE(first->transaction_immediate(tx, rx, {100U}));
    EXPECT_FALSE(wrote(mmio, base + 0x0AU, Board::solve_spi(6'000'000U)->spbr));
    ASSERT_TRUE(first->release()); ASSERT_TRUE(second->release());
    ASSERT_TRUE(hw.shutdown());
}

TEST(io_spi_devices, pending_and_unconsumed_operations_prevent_reconfigure_and_release, "REQ-0046") {
    SpiModel mmio;
    Policy policy;
    auto hw = io::make_board<Board, configuration>(policy, mmio, 1000U);
    ASSERT_TRUE(hw.begin()); ASSERT_TRUE(hw.spi.begin());
    auto first = hw.spi.device({.cs = hw.pins.d10});
    auto second = hw.spi.device({.cs = hw.pins.d2});
    ASSERT_TRUE(first); ASSERT_TRUE(second);
    const std::array<std::uint16_t, 1> tx{0x33U};
    const std::array segments{driver::spi_segment{.transmit = tx}};
    auto pending = first->transaction_async(segments);
    ASSERT_TRUE(pending);
    auto moved = std::move(*first);
    mmio.clear_operations();
    EXPECT_TRUE(second->transaction_async(segments).error().code == io::errc::busy);
    EXPECT_TRUE(second->release().error().code == io::errc::busy);
    EXPECT_TRUE(moved.release().error().code == io::errc::busy);
    EXPECT_EQ(mmio.operations().size(), 0U);
    // Borrowing the expert view must not invalidate CS cleanup in flight.
    static_cast<void>(hw.spi.unsafe_driver());
    ASSERT_TRUE(pending->cancel());
    EXPECT_TRUE(wrote(mmio, select_address(hw.pins.d10), 1U << hw.pins.d10.index));
    EXPECT_TRUE(moved.release().error().code == io::errc::busy);
    EXPECT_TRUE(pending->take().error().code == io::errc::cancelled);
    auto next = second->transaction_async(segments);
    ASSERT_TRUE(next);
    ASSERT_TRUE(next->cancel());
    EXPECT_TRUE(next->take().error().code == io::errc::cancelled);
    ASSERT_TRUE(moved.release()); ASSERT_TRUE(second->release());
    ASSERT_TRUE(hw.shutdown());
    EXPECT_EQ(policy.destruction_violations(), 0U);
}

TEST(io_spi_devices, immediate_deadline_stops_waiting_and_returns_bus_ownership, "REQ-0046") {
    SpiModel mmio;
    Policy policy;
    auto hw = io::make_board<Board, configuration>(policy, mmio, 1000U);
    ASSERT_TRUE(hw.begin()); ASSERT_TRUE(hw.spi.begin());
    auto device = hw.spi.device({.cs = hw.pins.d10});
    ASSERT_TRUE(device);
    const std::array<std::uint8_t, 1> tx{0x99U};
    mmio.clear_operations();
    EXPECT_TRUE(device->transaction_immediate(tx, {}, {0U}).error().code == io::errc::timeout);
    policy.set_isr(true);
    EXPECT_TRUE(device->transaction_immediate(tx, {}, {100U}).error().code == io::errc::invalid_context);
    EXPECT_TRUE(device->release().error().code == io::errc::invalid_context);
    EXPECT_EQ(mmio.operations().size(), 0U);
    policy.set_isr(false);
    mmio.ticking = &policy;
    mmio.stall = true;
    EXPECT_TRUE(device->transaction_immediate(tx, {}, {5U}).error().code == io::errc::timeout);
    EXPECT_TRUE(policy.now() < 10U);
    EXPECT_TRUE(wrote(mmio, select_address(hw.pins.d10), 1U << hw.pins.d10.index));
    mmio.ticking = nullptr; mmio.stall = false;
    ASSERT_TRUE(device->transaction_immediate(tx, {}, {100U}));
    ASSERT_TRUE(device->release()); ASSERT_TRUE(hw.shutdown());
}

TEST(io_spi_devices, invalid_configuration_and_failed_switch_do_not_select_chip, "REQ-0046") {
    SpiModel mmio;
    Policy policy;
    auto hw = io::make_board<Board, configuration>(policy, mmio, 1000U);
    ASSERT_TRUE(hw.begin()); ASSERT_TRUE(hw.spi.begin());
    mmio.clear_operations();
    EXPECT_FALSE(hw.spi.device({.cs = hw.pins.d10, .mode = static_cast<io::spi_mode>(8U)}));
    EXPECT_FALSE(hw.spi.device({.cs = hw.pins.d10, .frequency = 0U}));
    EXPECT_EQ(mmio.operations().size(), 0U);
    auto device = hw.spi.device({.cs = hw.pins.d10});
    ASSERT_TRUE(device);
    const std::array<std::uint8_t, 1> tx{0x11U};
    mmio.clear_operations(); mmio.fail_configuration = true;
    EXPECT_FALSE(device->transaction_immediate(tx, {}, {100U}));
    EXPECT_FALSE(wrote(mmio, select_address(hw.pins.d10), (1U << hw.pins.d10.index) << 16U));
    ASSERT_TRUE(device->transaction_immediate(tx, {}, {100U}));
    ASSERT_TRUE(device->release()); ASSERT_TRUE(hw.shutdown());
}

TEST(io_spi_devices, transient_select_release_failure_requires_explicit_retry, "REQ-0046") {
    SpiModel mmio;
    Policy policy;
    auto hw = io::make_board<Board, configuration>(policy, mmio, 1000U);
    ASSERT_TRUE(hw.begin()); ASSERT_TRUE(hw.spi.begin());
    auto device = hw.spi.device({.cs = hw.pins.d10});
    ASSERT_TRUE(device);
    const std::array<std::uint8_t, 1> tx{0x11U};
    mmio.fail_deselect = true;
    EXPECT_FALSE(device->transaction_immediate(tx, {}, {100U}));
    EXPECT_TRUE(device->release().error().code == io::errc::busy);
    ASSERT_TRUE(hw.spi.recover_select());
    ASSERT_TRUE(device->transaction_immediate(tx, {}, {100U}));
    ASSERT_TRUE(device->release()); ASSERT_TRUE(hw.shutdown());
}

TEST(io_spi_devices, failed_shutdown_blocks_use_until_shutdown_retry_completes, "REQ-0046") {
    SpiModel mmio;
    Policy policy;
    auto hw = io::make_board<Board, configuration>(policy, mmio, 1000U);
    ASSERT_TRUE(hw.begin()); ASSERT_TRUE(hw.spi.begin());
    mmio.fail_stop = true;
    EXPECT_FALSE(hw.spi.shutdown());
    mmio.clear_operations();
    EXPECT_TRUE(hw.spi.device({.cs = hw.pins.d10}).error().code == io::errc::invalid_operation);
    EXPECT_TRUE(hw.spi.transfer(0x11U, 1ms).error().code == io::errc::invalid_operation);
    EXPECT_EQ(mmio.operations().size(), 0U);
    ASSERT_TRUE(hw.spi.shutdown());
    ASSERT_TRUE(hw.spi.begin());
    auto device = hw.spi.device({.cs = hw.pins.d10});
    ASSERT_TRUE(device); ASSERT_TRUE(device->release());
    ASSERT_TRUE(hw.shutdown());
}
} // namespace
