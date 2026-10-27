#include <array>
#include <chrono>
#include <utility>
#include <mcutest/mcutest.hpp>
#include <mcubsp/backend/nrf52840/i2c.hpp>
#include <mcubsp/board/others/promicro_nrf52840.hpp>
#include <mcubsp/device/sensor/tmp102.hpp>
#include <mcubsp/execution/fake_policy.hpp>
#include <mcubsp/io/board.hpp>
#include "fake_twi.hpp"

namespace {
using namespace mcubsp;
using namespace std::chrono_literals;
using binding = backend::nrf52840::i2c_binding<0, {0, 11}, {1, 0}>;
using execution_type = execution::fake_policy;
using backend_type = backend::nrf52840::i2c_controller_backend<binding, nrf52840_test::fake_twi, execution_type>;
constexpr auto base = binding::base;
constexpr auto timing = backend::nrf52840::solve_i2c_timing({16'000'000, 100'000, 0, 300, 100});
static_assert(timing && timing->frequency == 0x0198'0000U);
static_assert(mcu::nrf52840::twi_registers::errorsrc::access == core::access_semantics::write_one_to_clear);

struct fixture {
    nrf52840_test::fake_twi mmio;
    execution_type execution;
    backend_type bus{mmio, execution};
    ~fixture() { mmio.failed_address.reset(); (void)bus.shutdown(); }
    void event(std::uintptr_t offset, std::uint32_t value = 1U) { mmio.poke(base + offset, value); }
};

TEST(nrf52840_i2c, validates_timing_and_native_frequency_presets, "REQ-0054") {
    using backend::nrf52840::solve_i2c_timing;
    EXPECT_EQ(solve_i2c_timing({16'000'000, 250'000, 0, 300, 100})->frequency, 0x0400'0000U);
    EXPECT_EQ(solve_i2c_timing({16'000'000, 400'000, 0, 300, 100})->frequency, 0x0668'0000U);
    EXPECT_FALSE(solve_i2c_timing({64'000'000, 100'000, 0, 300, 100}).has_value());
    EXPECT_FALSE(solve_i2c_timing({16'000'000, 1, 20'000, 300, 100}).has_value());
    EXPECT_FALSE(solve_i2c_timing({16'000'000, 0, 20'000, 300, 100}).has_value());
    EXPECT_FALSE(solve_i2c_timing({16'000'000, 200'000, 20'000, 300, 100}).has_value());
    EXPECT_FALSE(solve_i2c_timing({16'000'000, 400'000, 20'000, 1200, 100}).has_value());
    fixture f;
    auto invalid = *timing; invalid.frequency = 1;
    EXPECT_FALSE(f.bus.initialize(invalid).has_value());
    EXPECT_EQ(f.mmio.write_count, 0U);
}

TEST(nrf52840_i2c, configures_open_drain_native_routes_and_restores_on_shutdown, "REQ-0054") {
    fixture f;
    ASSERT_TRUE(f.bus.initialize(*timing).has_value());
    EXPECT_TRUE(f.mmio.wrote(0x5000'072CU, 0x600U));
    EXPECT_TRUE(f.mmio.wrote(0x5000'0A00U, 0x600U));
    EXPECT_EQ(f.mmio.peek(base + 0x508), 11U);
    EXPECT_EQ(f.mmio.peek(base + 0x50C), 32U);
    EXPECT_EQ(f.mmio.peek(base + 0x524), timing->frequency);
    EXPECT_EQ(f.mmio.peek(base + 0x500), 2U);
    ASSERT_TRUE(f.bus.shutdown().has_value());
    EXPECT_EQ(f.mmio.peek(base + 0x500), 0U);
    EXPECT_EQ(f.mmio.peek(base + 0x508), 0x8000'0000U);
    EXPECT_EQ(f.mmio.peek(0x5000'072CU), 0U);
}

TEST(nrf52840_i2c, shares_instance_ownership_across_different_pin_bindings, "REQ-0054") {
    fixture f;
    using alternate = backend::nrf52840::i2c_binding<0, {1, 6}, {1, 4}>;
    backend::nrf52840::i2c_controller_backend<alternate, nrf52840_test::fake_twi, execution_type> other{f.mmio, f.execution};
    ASSERT_TRUE(f.bus.initialize(*timing).has_value());
    const auto duplicate = other.initialize(*timing);
    ASSERT_FALSE(duplicate.has_value());
    EXPECT_TRUE(duplicate.error() == driver::i2c_error::already_owned);
    ASSERT_TRUE(f.bus.shutdown().has_value());
    ASSERT_TRUE(other.initialize(*timing).has_value());
    ASSERT_TRUE(other.shutdown().has_value());
    f.event(0x500, 7U); // Another driver owns this shared block.
    f.mmio.clear_trace();
    EXPECT_FALSE(f.bus.initialize(*timing).has_value());
    EXPECT_EQ(f.mmio.write_count, 0U);
}

TEST(nrf52840_i2c, writes_wait_for_final_stop_and_preserve_unconsumed_slot, "REQ-0054") {
    fixture f;
    ASSERT_TRUE(f.bus.initialize(*timing).has_value());
    const std::array<std::uint8_t, 2> data{0xAB, 0xCD};
    f.mmio.stop_responds = false;
    auto transfer = f.bus.write({0x48}, data);
    ASSERT_TRUE(transfer.has_value());
    f.mmio.clear_trace();
    f.event(0x11C);
    (void)f.bus.poll();
    EXPECT_TRUE(f.mmio.wrote(base + 0x51C, 0xCD));
    EXPECT_FALSE(transfer->ready());
    f.event(0x11C);
    (void)f.bus.poll();
    EXPECT_TRUE(f.mmio.wrote(base + 0x014, 1U));
    EXPECT_FALSE(transfer->ready());
    EXPECT_FALSE(f.bus.shutdown().has_value());
    f.event(0x104);
    ASSERT_TRUE(f.bus.poll());
    EXPECT_TRUE(transfer->ready());
    EXPECT_FALSE(f.bus.write({0x48}, data).has_value());
    const auto result = transfer->take();
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->bytes_written, 2U);
    EXPECT_EQ(result->bytes_read, 0U);
}

TEST(nrf52840_i2c, combined_transaction_restarts_without_stop_and_keeps_final_rx_byte, "REQ-0054") {
    fixture f;
    ASSERT_TRUE(f.bus.initialize(*timing).has_value());
    const std::array<std::uint8_t, 1> command{0x00};
    std::array<std::uint8_t, 2> reply{};
    auto transfer = f.bus.write_read({0x48}, command, reply);
    ASSERT_TRUE(transfer.has_value());
    f.mmio.clear_trace();
    f.event(0x11C); (void)f.bus.poll();
    EXPECT_TRUE(f.mmio.wrote(base + 0x01C, 1U));
    EXPECT_FALSE(f.mmio.wrote(base + 0x014, 1U));
    f.event(0x148); (void)f.bus.poll();
    EXPECT_TRUE(f.mmio.wrote(base + 0x000, 1U));
    EXPECT_EQ(f.mmio.peek(base + 0x200), 1U);
    f.event(0x518, 0x19); f.event(0x108); (void)f.bus.poll();
    EXPECT_EQ(f.mmio.peek(base + 0x200), 2U);
    f.event(0x518, 0x80); f.event(0x108); f.event(0x104);
    ASSERT_TRUE(f.bus.poll());
    const auto result = transfer->take();
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->bytes_read, 2U);
    EXPECT_EQ(reply[0], 0x19);
    EXPECT_EQ(reply[1], 0x80);
    EXPECT_FALSE(f.mmio.wrote(base + 0x014, 1U));
}

TEST(nrf52840_i2c, single_byte_receive_arms_stop_before_start_and_rejects_premature_stop, "REQ-0054") {
    fixture f;
    ASSERT_TRUE(f.bus.initialize(*timing).has_value());
    std::array<std::uint8_t, 1> reply{};
    auto transfer = f.bus.read({0x48}, reply);
    ASSERT_TRUE(transfer.has_value());
    EXPECT_EQ(f.mmio.peek(base + 0x200), 2U);
    auto stop_shortcut = f.mmio.trace.size();
    auto receive_start = f.mmio.trace.size();
    for (std::size_t i = 0; i < f.mmio.write_count; ++i) {
        if (f.mmio.trace[i].address == base + 0x200 && f.mmio.trace[i].value == 2U)
            stop_shortcut = i;
        if (f.mmio.trace[i].address == base && f.mmio.trace[i].value == 1U)
            receive_start = i;
    }
    EXPECT_TRUE(stop_shortcut < receive_start);
    f.event(0x518, 0x42); f.event(0x108); f.event(0x104);
    ASSERT_TRUE(f.bus.poll());
    ASSERT_TRUE(transfer->take().has_value());
    EXPECT_EQ(reply[0], 0x42);
    auto premature = f.bus.read({0x48}, reply);
    ASSERT_TRUE(premature.has_value());
    f.event(0x104); (void)f.bus.poll();
    const auto result = premature->take();
    ASSERT_FALSE(result.has_value());
    EXPECT_TRUE(result.error().cause == driver::i2c_error::transport);
    EXPECT_TRUE(f.bus.faulted());
}

TEST(nrf52840_i2c, maps_observable_nacks_and_errors_only_after_stop, "REQ-0054") {
    fixture f;
    ASSERT_TRUE(f.bus.initialize(*timing).has_value());
    const std::array<std::uint8_t, 1> data{1};
    f.mmio.stop_responds = false;
    for (auto code : {2U, 4U, 1U}) {
        auto transfer = f.bus.write({0x48}, data);
        ASSERT_TRUE(transfer.has_value());
        f.event(0x4C4, code); f.event(0x124); (void)f.bus.poll();
        EXPECT_FALSE(transfer->ready());
        EXPECT_EQ(f.mmio.peek(base + 0x4C4), 0U);
        f.event(0x104); ASSERT_TRUE(f.bus.poll());
        const auto result = transfer->take();
        ASSERT_FALSE(result.has_value());
        const auto expected = code == 2 ? driver::i2c_error::address_nack
            : code == 4 ? driver::i2c_error::data_nack : driver::i2c_error::transport;
        EXPECT_TRUE(result.error().cause == expected);
    }
}

TEST(nrf52840_i2c, rejects_invalid_requests_stuck_lines_and_busy_hardware, "REQ-0054") {
    fixture f;
    const std::array<std::uint8_t, 1> data{1};
    EXPECT_FALSE(f.bus.write({0x48}, data).has_value());
    ASSERT_TRUE(f.bus.initialize(*timing).has_value());
    EXPECT_FALSE(f.bus.write({0x48}, {}).has_value());
    EXPECT_FALSE(f.bus.write({0}, data).has_value());
    EXPECT_FALSE(f.bus.write({0x78}, data).has_value());
    f.mmio.poke(0x5000'0510U, 0U);
    auto stuck = f.bus.write({0x48}, data);
    ASSERT_FALSE(stuck.has_value());
    EXPECT_TRUE(stuck.error().cause == driver::i2c_error::bus_stuck);
    f.mmio.poke(0x5000'0510U, 0xFFFF'FFFFU);
    auto transfer = f.bus.write({0x48}, data);
    ASSERT_TRUE(transfer.has_value());
    EXPECT_FALSE(f.bus.write({0x48}, data).has_value());
    ASSERT_TRUE(transfer->cancel().has_value());
}

TEST(nrf52840_i2c, cancellation_clears_late_events_and_allows_reuse, "REQ-0054") {
    fixture f;
    ASSERT_TRUE(f.bus.initialize(*timing).has_value());
    std::array<std::uint8_t, 1> data{};
    auto transfer = f.bus.read({0x48}, data);
    ASSERT_TRUE(transfer.has_value());
    ASSERT_TRUE(transfer->cancel().has_value());
    EXPECT_TRUE(transfer->take().error().kind == core::transfer_error_kind::cancelled);
    f.event(0x108); f.event(0x104); f.event(0x518, 0xFF);
    EXPECT_FALSE(f.bus.poll());
    auto next = f.bus.read({0x48}, data);
    ASSERT_TRUE(next.has_value());
    EXPECT_FALSE(next->ready());
    EXPECT_EQ(f.mmio.peek(base + 0x108), 0U);
    EXPECT_EQ(data[0], 0U);
    ASSERT_TRUE(next->cancel().has_value());
}

TEST(nrf52840_i2c, timeout_uses_common_transfer_policy_and_failed_stop_is_bounded, "REQ-0054") {
    fixture f;
    ASSERT_TRUE(f.bus.initialize(*timing).has_value());
    std::array<std::uint8_t, 1> data{};
    auto transfer = f.bus.read({0x48}, data);
    ASSERT_TRUE(transfer.has_value());
    const auto waited = transfer->wait_until(execution::deadline{3U});
    ASSERT_FALSE(waited.has_value());
    EXPECT_TRUE(transfer->take().error().kind == core::transfer_error_kind::timeout);
    auto failed = f.bus.read({0x48}, data);
    ASSERT_TRUE(failed.has_value());
    f.mmio.stop_responds = false;
    const auto before = f.mmio.reads;
    EXPECT_FALSE(failed->cancel().has_value());
    EXPECT_TRUE(f.mmio.reads - before <= backend_type::stop_poll_limit + 4U);
    EXPECT_TRUE(failed->take().error().kind == core::transfer_error_kind::quiesce_failed);
    EXPECT_TRUE(f.bus.faulted());
    EXPECT_FALSE(f.bus.read({0x48}, data).has_value());
    ASSERT_TRUE(f.bus.shutdown().has_value());
    EXPECT_FALSE(f.bus.initialize(*timing).has_value());
}

TEST(nrf52840_i2c, rolls_back_initialization_and_poisoned_transfer_mmio_failures, "REQ-0054") {
    fixture f;
    f.mmio.failed_address = base + 0x524; f.mmio.fail_once = true;
    EXPECT_FALSE(f.bus.initialize(*timing).has_value());
    EXPECT_EQ(f.mmio.peek(base + 0x500), 0U);
    EXPECT_FALSE(backend::nrf52840::detail::serial_owner<0>::claimed);
    backend_type replacement{f.mmio, f.execution};
    ASSERT_TRUE(replacement.initialize(*timing).has_value());
    std::array<std::uint8_t, 1> data{};
    auto transfer = replacement.read({0x48}, data);
    ASSERT_TRUE(transfer.has_value());
    f.mmio.failed_address = base + 0x108; f.mmio.fail_once = true;
    ASSERT_TRUE(replacement.poll());
    EXPECT_TRUE(transfer->take().error().cause == driver::i2c_error::mmio);
    EXPECT_TRUE(replacement.faulted());
    EXPECT_FALSE(replacement.read({0x48}, data).has_value());
    ASSERT_TRUE(replacement.shutdown().has_value());
}

TEST(nrf52840_i2c, device_leases_pin_conflicts_and_real_tmp102_work_through_facade, "REQ-0054") {
    nrf52840_test::fake_twi mmio;
    mmio.automatic = true;
    execution_type policy;
    using profile = board::others::promicro_nrf52840;
    auto hw = io::make_board<profile, {.serial = false, .i2c = true}>(policy, mmio, 1000U);
    ASSERT_TRUE(hw.begin().has_value());
    auto pin = hw.output(hw.pins.sda);
    ASSERT_TRUE(pin.has_value());
    EXPECT_FALSE(hw.i2c.begin().has_value());
    ASSERT_TRUE(pin->release().has_value());
    ASSERT_TRUE(hw.i2c.begin().has_value());
    EXPECT_FALSE(hw.output(hw.pins.scl).has_value());
    {
        auto endpoint = hw.i2c.device(0x48);
        ASSERT_TRUE(endpoint.has_value());
        EXPECT_FALSE(hw.i2c.device(0x48).has_value());
        device::tmp102 sensor{std::move(*endpoint)};
        ASSERT_TRUE(sensor.begin(100ms).has_value());
        const auto sample = sensor.read(100ms);
        ASSERT_TRUE(sample.has_value());
        EXPECT_EQ(sample->celsius_sixteenths, 25 * 16);
        EXPECT_FALSE(hw.i2c.shutdown().has_value());
        std::array<std::uint8_t, 1> byte{};
        EXPECT_FALSE(hw.i2c.read_async({0x48}, byte).has_value());
    }
    ASSERT_TRUE(hw.i2c.shutdown().has_value());
    ASSERT_TRUE(hw.shutdown().has_value());
}
TEST(nrf52840_i2c, failed_disable_retains_owner_until_explicit_successful_shutdown, "REQ-0054") {
    fixture f;
    ASSERT_TRUE(f.bus.initialize(*timing).has_value());
    std::array<std::uint8_t, 1> byte{};
    auto transfer = f.bus.read({0x48}, byte);
    ASSERT_TRUE(transfer.has_value());
    f.mmio.failed_address = base + 0x500;
    EXPECT_FALSE(transfer->cancel().has_value());
    EXPECT_TRUE(transfer->take().error().kind == core::transfer_error_kind::quiesce_failed);
    EXPECT_FALSE(f.bus.shutdown().has_value());
    EXPECT_TRUE(backend::nrf52840::detail::serial_owner<0>::claimed);
    f.mmio.failed_address.reset();
    ASSERT_TRUE(f.bus.shutdown().has_value());
    EXPECT_FALSE(backend::nrf52840::detail::serial_owner<0>::claimed);
    backend_type fresh{f.mmio, f.execution};
    ASSERT_TRUE(fresh.initialize(*timing).has_value());
    ASSERT_TRUE(fresh.shutdown().has_value());
}

TEST(nrf52840_i2c, selected_twi1_and_default_twi0_share_runtime_without_pin_conflicts, "REQ-0054") {
    using extra = io::i2c_selection<backend::nrf52840::i2c_binding<1, {1, 6}, {1, 4}>>;
    nrf52840_test::fake_twi mmio;
    execution_type policy;
    auto hw = io::make_board<board::others::promicro_nrf52840,
        {.serial = false, .i2c = true}, extra>(policy, mmio, 1000U);
    ASSERT_TRUE(hw.begin().has_value());
    ASSERT_TRUE(hw.i2c.begin().has_value());
    ASSERT_TRUE(hw.peripheral<0>().begin(400'000U).has_value());
    EXPECT_EQ(mmio.peek(0x4000'4508U), 38U);
    EXPECT_EQ(mmio.peek(0x4000'450CU), 36U);
    EXPECT_EQ(mmio.peek(0x4000'4524U), 0x0668'0000U);
    std::array<std::uint8_t, 1> byte{};
    auto transfer = hw.peripheral<0>().read_async({0x48}, byte);
    ASSERT_TRUE(transfer.has_value());
    mmio.poke(0x4000'4518U, 0x5AU);
    mmio.poke(0x4000'4108U, 1U);
    mmio.poke(0x4000'4104U, 1U);
    hw.poll();
    ASSERT_TRUE(transfer->take().has_value());
    EXPECT_EQ(byte[0], 0x5A);
    ASSERT_TRUE(hw.shutdown().has_value());
}

TEST(nrf52840_i2c, start_and_data_mmio_failures_do_not_publish_success_or_modify_rx_buffers, "REQ-0054") {
    for (auto offset : {0x008U, 0x51CU, 0x000U, 0x518U}) {
        fixture f;
        ASSERT_TRUE(f.bus.initialize(*timing).has_value());
        const std::array<std::uint8_t, 1> output{0xAA};
        std::array<std::uint8_t, 1> input{0x55};
        f.mmio.failed_address = base + offset;
        f.mmio.fail_once = true;
        auto operation = (offset == 0x008U || offset == 0x51CU)
            ? f.bus.write({0x48}, output) : f.bus.read({0x48}, input);
        ASSERT_TRUE(operation.has_value());
        if (offset == 0x518U) { f.event(0x108); (void)f.bus.poll(); }
        EXPECT_TRUE(operation->ready());
        const auto result = operation->take();
        ASSERT_FALSE(result.has_value());
        EXPECT_TRUE(result.error().cause == driver::i2c_error::mmio);
        EXPECT_TRUE(f.bus.faulted());
        EXPECT_EQ(input[0], 0x55);
        EXPECT_EQ(f.mmio.peek(base + 0x500), 0U);
    }
}
} // namespace
