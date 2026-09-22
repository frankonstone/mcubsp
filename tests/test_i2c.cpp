#include <array>
#include <cstdint>
#include <expected>
#include <span>
#include <utility>

#include <mcutest/mcutest.hpp>

#include <mcuhw/backend/ra4m1/iic.hpp>
#include <mcuhw/board/arduino_uno_r4_minima.hpp>
#include <mcuhw/driver/i2c.hpp>
#include <mcuhw/execution/fake_policy.hpp>
#include <mcuhw/testing/recording_mmio.hpp>

namespace {

using binding = mcuhw::backend::ra4m1::iic_binding<
    1, mcuhw::board::arduino_uno_r4_minima::a5_scl,
    mcuhw::board::arduino_uno_r4_minima::a4_sda>;
using mmio_type = mcuhw::testing::recording_mmio<0x60000, 512>;
using execution_type = mcuhw::execution::fake_policy;
using backend_type = mcuhw::backend::ra4m1::iic_controller_backend<
    binding, mmio_type, execution_type>;

constexpr auto timing_100khz = mcuhw::driver::solve_i2c_timing({
    .clock_hz = 24'000'000U,
    .target_hz = 100'000U,
    .maximum_error_ppm = 20'000U,
    .rise_time_ns = 300U,
    .fall_time_ns = 100U,
});
static_assert(timing_100khz.has_value());
constexpr auto timing_400khz = mcuhw::driver::solve_i2c_timing({
    .clock_hz = 24'000'000U,
    .target_hz = 400'000U,
    .maximum_error_ppm = 30'000U,
    .rise_time_ns = 120U,
    .fall_time_ns = 60U,
});
static_assert(timing_400khz.has_value());
constexpr auto address = mcuhw::driver::make_i2c_address(0x50U);
static_assert(address.has_value());

[[nodiscard]] bool has_write(std::span<const mcuhw::testing::mmio_operation> ops,
                             std::uintptr_t address_value,
                             std::uint64_t value) {
    for (const auto& operation : ops) {
        if (operation.kind == mcuhw::testing::mmio_operation_kind::write
            && operation.address == address_value
            && operation.value == value) {
            return true;
        }
    }
    return false;
}

[[nodiscard]] bool has_write_bits(
    std::span<const mcuhw::testing::mmio_operation> ops,
    std::uintptr_t address_value, std::uint64_t set,
    std::uint64_t clear) {
    for (const auto& operation : ops) {
        if (operation.kind == mcuhw::testing::mmio_operation_kind::write
            && operation.address == address_value
            && (operation.value & set) == set
            && (operation.value & clear) == 0U) {
            return true;
        }
    }
    return false;
}

[[nodiscard]] bool write_precedes(
    std::span<const mcuhw::testing::mmio_operation> ops,
    std::uintptr_t first_address, std::uint64_t first_value,
    std::uintptr_t second_address, std::uint64_t second_set) {
    bool first_seen = false;
    for (const auto& operation : ops) {
        if (operation.kind != mcuhw::testing::mmio_operation_kind::write) {
            continue;
        }
        if (!first_seen && operation.address == first_address
            && operation.value == first_value) {
            first_seen = true;
        }
        if (operation.address == second_address
            && (operation.value & second_set) == second_set) {
            return first_seen;
        }
    }
    return false;
}

TEST(i2c_contract, rejects_reserved_addresses_and_solves_real_bus_timing,
     "REQ-0024") {
    EXPECT_TRUE(mcuhw::driver::make_i2c_address(0x07U).error()
                == mcuhw::driver::i2c_error::invalid_address);
    EXPECT_TRUE(mcuhw::driver::make_i2c_address(0x78U).error()
                == mcuhw::driver::i2c_error::invalid_address);
    ASSERT_TRUE(timing_100khz.has_value());
    EXPECT_TRUE(timing_100khz->achieved_hz >= 98'000U);
    EXPECT_TRUE(timing_100khz->achieved_hz <= 102'000U);
    EXPECT_TRUE(timing_100khz->brl <= 31U);
    EXPECT_TRUE(timing_100khz->brh <= 31U);
    ASSERT_TRUE(timing_400khz.has_value());
    EXPECT_TRUE(timing_400khz->achieved_hz >= 388'000U);
    EXPECT_TRUE(timing_400khz->achieved_hz <= 412'000U);
    EXPECT_TRUE(mcuhw::driver::solve_i2c_timing(
                    {24'000'000U, 400'000U, 50'000U, 1'000U, 100U})
                    .error()
                == mcuhw::driver::i2c_error::invalid_rise_fall);
}

TEST(ra4m1_i2c, configures_iic1_timing_and_uno_a4_a5_open_drain_routes,
     "REQ-0024") {
    mmio_type mmio{0x4001'E000U};
    execution_type execution;
    backend_type backend{mmio, execution};
    ASSERT_TRUE(backend.initialize(*timing_100khz).has_value());
    EXPECT_TRUE(has_write(mmio.operations(), 0x4005'3110U,
                          0xE0U | timing_100khz->brl));
    EXPECT_TRUE(has_write(mmio.operations(), 0x4005'3111U,
                          0xE0U | timing_100khz->brh));
    EXPECT_TRUE(has_write(mmio.operations(), 0x4004'0840U,
                          (0x07U << 24U) | (1U << 16U) | (1U << 6U)));
    EXPECT_TRUE(has_write(mmio.operations(), 0x4004'0844U,
                          (0x07U << 24U) | (1U << 16U) | (1U << 6U)));
    ASSERT_TRUE(backend.shutdown().has_value());
}

TEST(ra4m1_i2c, write_uses_unshifted_address_and_completes_after_stop,
     "REQ-0017", "REQ-0024") {
    mmio_type mmio{0x4001'E000U};
    execution_type execution;
    backend_type backend{mmio, execution};
    ASSERT_TRUE(backend.initialize(*timing_100khz).has_value());
    const std::array<std::uint8_t, 2> bytes{0x10U, 0xABU};
    auto started = backend.write(*address, bytes);
    ASSERT_TRUE(started.has_value());
    auto transfer = std::move(*started);
    EXPECT_TRUE(backend.handle_transmit_empty_interrupt());
    EXPECT_TRUE(backend.handle_transmit_empty_interrupt());
    EXPECT_TRUE(backend.handle_transmit_empty_interrupt());
    EXPECT_FALSE(transfer.ready());
    EXPECT_TRUE(backend.handle_transmit_end_interrupt());
    EXPECT_FALSE(transfer.ready());
    ASSERT_TRUE(mmio.write<std::uint8_t>(0x4005'3109U, 1U << 3U)
                    .has_value());
    EXPECT_TRUE(backend.handle_error_interrupt());
    const auto result = transfer.take();
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->bytes_written, 2U);
    EXPECT_EQ(result->bytes_read, 0U);
    EXPECT_TRUE(has_write(mmio.operations(), 0x4005'3112U, 0xA0U));
    EXPECT_TRUE(has_write(mmio.operations(), 0x4005'3112U, 0x10U));
    EXPECT_TRUE(has_write(mmio.operations(), 0x4005'3112U, 0xABU));
    ASSERT_TRUE(backend.shutdown().has_value());
}

TEST(ra4m1_i2c, combined_transfer_uses_repeated_start_without_stop,
     "REQ-0024") {
    mmio_type mmio{0x4001'E000U};
    execution_type execution;
    backend_type backend{mmio, execution};
    ASSERT_TRUE(backend.initialize(*timing_100khz).has_value());
    const std::array<std::uint8_t, 1> command{0x20U};
    std::array<std::uint8_t, 2> response{};
    auto started = backend.write_read(*address, command, response);
    ASSERT_TRUE(started.has_value());
    auto transfer = std::move(*started);
    EXPECT_TRUE(backend.handle_transmit_empty_interrupt()); // address W
    EXPECT_TRUE(backend.handle_transmit_empty_interrupt()); // register
    const auto restart_begin = mmio.operations().size();
    EXPECT_TRUE(backend.handle_transmit_end_interrupt());   // repeated START
    const auto restart_operations = mmio.operations().subspan(restart_begin);
    EXPECT_TRUE(has_write_bits(restart_operations, 0x4005'3101U,
                               1U << 2U, 1U << 3U));
    EXPECT_TRUE(write_precedes(restart_operations,
                               0x4005'3107U, 0x9BU,
                               0x4005'3101U, 1U << 2U));
    EXPECT_TRUE(backend.handle_transmit_empty_interrupt()); // address R
    ASSERT_TRUE(mmio.write<std::uint8_t>(0x4005'3113U, 0U).has_value());
    EXPECT_TRUE(backend.handle_receive_interrupt());        // dummy read
    ASSERT_TRUE(mmio.write<std::uint8_t>(0x4005'3113U, 0x34U).has_value());
    EXPECT_TRUE(backend.handle_receive_interrupt());
    ASSERT_TRUE(mmio.write<std::uint8_t>(0x4005'3113U, 0x12U).has_value());
    EXPECT_TRUE(backend.handle_receive_interrupt());
    ASSERT_TRUE(mmio.write<std::uint8_t>(0x4005'3109U, 1U << 3U)
                    .has_value());
    EXPECT_TRUE(backend.handle_error_interrupt());
    const auto result = transfer.take();
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->bytes_written, 1U);
    EXPECT_EQ(result->bytes_read, 2U);
    EXPECT_EQ(response[0], 0x34U);
    EXPECT_EQ(response[1], 0x12U);
    EXPECT_TRUE(has_write(mmio.operations(), 0x4005'3112U, 0xA1U));
    EXPECT_TRUE(has_write(mmio.operations(), 0x4005'3104U, 0x40U));
    EXPECT_TRUE(has_write(mmio.operations(), 0x4005'3104U, 0x50U));
    EXPECT_TRUE(has_write(mmio.operations(), 0x4005'3104U, 0x58U));
    EXPECT_TRUE(has_write(mmio.operations(), 0x4005'3104U, 0x48U));
    EXPECT_TRUE(has_write(mmio.operations(), 0x4005'3104U, 0x08U));
    ASSERT_TRUE(backend.shutdown().has_value());
}

TEST(ra4m1_i2c, one_byte_read_programs_nack_before_dummy_and_releases_wait,
     "REQ-0024") {
    mmio_type mmio{0x4001'E000U};
    execution_type execution;
    backend_type backend{mmio, execution};
    ASSERT_TRUE(backend.initialize(*timing_100khz).has_value());
    std::array<std::uint8_t, 1> response{};
    auto started = backend.read(*address, response);
    ASSERT_TRUE(started.has_value());
    auto transfer = std::move(*started);
    EXPECT_TRUE(backend.handle_transmit_empty_interrupt()); // address R
    ASSERT_TRUE(mmio.write<std::uint8_t>(0x4005'3113U, 0U).has_value());
    EXPECT_TRUE(backend.handle_receive_interrupt());        // dummy read
    ASSERT_TRUE(mmio.write<std::uint8_t>(0x4005'3113U, 0x5AU).has_value());
    EXPECT_TRUE(backend.handle_receive_interrupt());        // final byte
    ASSERT_TRUE(mmio.write<std::uint8_t>(0x4005'3109U, 1U << 3U)
                    .has_value());
    EXPECT_TRUE(backend.handle_error_interrupt());
    const auto result = transfer.take();
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->bytes_written, 0U);
    EXPECT_EQ(result->bytes_read, 1U);
    EXPECT_EQ(response[0], 0x5AU);
    EXPECT_TRUE(has_write(mmio.operations(), 0x4005'3104U, 0x40U));
    EXPECT_TRUE(has_write(mmio.operations(), 0x4005'3104U, 0x50U));
    EXPECT_TRUE(has_write(mmio.operations(), 0x4005'3104U, 0x58U));
    EXPECT_TRUE(has_write(mmio.operations(), 0x4005'3104U, 0x48U));
    EXPECT_TRUE(has_write(mmio.operations(), 0x4005'3104U, 0x08U));
    ASSERT_TRUE(backend.shutdown().has_value());
}

TEST(ra4m1_i2c, three_byte_read_sets_wait_before_penultimate_nack,
     "REQ-0024") {
    mmio_type mmio{0x4001'E000U};
    execution_type execution;
    backend_type backend{mmio, execution};
    ASSERT_TRUE(backend.initialize(*timing_100khz).has_value());
    const std::array<std::uint8_t, 1> command{0x20U};
    std::array<std::uint8_t, 3> response{};
    auto started = backend.write_read(*address, command, response);
    ASSERT_TRUE(started.has_value());
    auto transfer = std::move(*started);
    EXPECT_TRUE(backend.handle_transmit_empty_interrupt()); // address W
    EXPECT_TRUE(backend.handle_transmit_empty_interrupt()); // register
    EXPECT_TRUE(backend.handle_transmit_end_interrupt());   // repeated START
    EXPECT_TRUE(backend.handle_transmit_empty_interrupt()); // address R
    ASSERT_TRUE(mmio.write<std::uint8_t>(0x4005'3113U, 0U).has_value());
    EXPECT_TRUE(backend.handle_receive_interrupt());        // dummy read
    for (std::uint8_t value : std::array<std::uint8_t, 3>{0x31U, 0x32U,
                                                          0x33U}) {
        ASSERT_TRUE(mmio.write<std::uint8_t>(0x4005'3113U, value)
                        .has_value());
        EXPECT_TRUE(backend.handle_receive_interrupt());
    }
    ASSERT_TRUE(mmio.write<std::uint8_t>(0x4005'3109U, 1U << 3U)
                    .has_value());
    EXPECT_TRUE(backend.handle_error_interrupt());
    const auto result = transfer.take();
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->bytes_written, 1U);
    EXPECT_EQ(result->bytes_read, 3U);
    EXPECT_EQ(response[0], 0x31U);
    EXPECT_EQ(response[1], 0x32U);
    EXPECT_EQ(response[2], 0x33U);
    EXPECT_TRUE(has_write(mmio.operations(), 0x4005'3104U, 0x40U));
    EXPECT_TRUE(has_write(mmio.operations(), 0x4005'3104U, 0x50U));
    EXPECT_TRUE(has_write(mmio.operations(), 0x4005'3104U, 0x58U));
    EXPECT_TRUE(has_write(mmio.operations(), 0x4005'3104U, 0x48U));
    EXPECT_TRUE(has_write(mmio.operations(), 0x4005'3104U, 0x08U));
    ASSERT_TRUE(backend.shutdown().has_value());
}

TEST(ra4m1_i2c, distinguishes_address_data_nack_and_arbitration_loss,
     "REQ-0025") {
    mmio_type mmio{0x4001'E000U};
    execution_type execution;
    backend_type backend{mmio, execution};
    ASSERT_TRUE(backend.initialize(*timing_100khz).has_value());
    const std::array<std::uint8_t, 1> bytes{0x44U};

    auto address_start = backend.write(*address, bytes);
    ASSERT_TRUE(address_start.has_value());
    auto address_transfer = std::move(*address_start);
    EXPECT_TRUE(backend.handle_transmit_empty_interrupt());
    ASSERT_TRUE(mmio.write<std::uint8_t>(0x4005'3109U, 1U << 4U)
                    .has_value());
    EXPECT_TRUE(backend.handle_error_interrupt());
    ASSERT_TRUE(mmio.write<std::uint8_t>(0x4005'3109U, 1U << 3U)
                    .has_value());
    EXPECT_TRUE(backend.handle_error_interrupt());
    const auto address_error = address_transfer.take();
    ASSERT_FALSE(address_error.has_value());
    EXPECT_TRUE(address_error.error().cause
                == mcuhw::driver::i2c_error::address_nack);

    auto data_start = backend.write(*address, bytes);
    ASSERT_TRUE(data_start.has_value());
    auto data_transfer = std::move(*data_start);
    EXPECT_TRUE(backend.handle_transmit_empty_interrupt());
    EXPECT_TRUE(backend.handle_transmit_empty_interrupt());
    ASSERT_TRUE(mmio.write<std::uint8_t>(0x4005'3109U, 1U << 4U)
                    .has_value());
    EXPECT_TRUE(backend.handle_error_interrupt());
    ASSERT_TRUE(mmio.write<std::uint8_t>(0x4005'3109U, 1U << 3U)
                    .has_value());
    EXPECT_TRUE(backend.handle_error_interrupt());
    const auto data_error = data_transfer.take();
    ASSERT_FALSE(data_error.has_value());
    EXPECT_TRUE(data_error.error().cause
                == mcuhw::driver::i2c_error::data_nack);

    mmio.clear_operations();
    auto arbitration_start = backend.write(*address, bytes);
    ASSERT_TRUE(arbitration_start.has_value());
    auto arbitration = std::move(*arbitration_start);
    ASSERT_TRUE(mmio.write<std::uint8_t>(0x4005'3109U, 1U << 1U)
                    .has_value());
    EXPECT_TRUE(backend.handle_error_interrupt());
    const auto arbitration_error = arbitration.take();
    ASSERT_FALSE(arbitration_error.has_value());
    EXPECT_TRUE(arbitration_error.error().cause
                == mcuhw::driver::i2c_error::arbitration_lost);
    EXPECT_FALSE(has_write(mmio.operations(), 0x4005'3100U, 0xC0U));
    ASSERT_TRUE(backend.shutdown().has_value());
}

TEST(ra4m1_i2c, timeout_cancels_hardware_and_operation_slot_is_reusable,
     "REQ-0019", "REQ-0025") {
    mmio_type mmio{0x4001'E000U};
    execution_type execution;
    backend_type backend{mmio, execution};
    ASSERT_TRUE(backend.initialize(*timing_100khz).has_value());
    const std::array<std::uint8_t, 1> bytes{0x44U};
    const std::array<std::uint8_t, 0> empty{};
    const auto empty_start = backend.write(*address, empty);
    ASSERT_FALSE(empty_start.has_value());
    EXPECT_TRUE(empty_start.error().cause
                == mcuhw::driver::i2c_error::empty_transfer);
    auto started = backend.write(*address, bytes);
    ASSERT_TRUE(started.has_value());
    auto transfer = std::move(*started);
    const auto waited = transfer.wait_until(mcuhw::execution::deadline{0U});
    ASSERT_FALSE(waited.has_value());
    EXPECT_TRUE(waited.error().kind
                == mcuhw::core::transfer_error_kind::timeout);
    EXPECT_TRUE(transfer.take().error().kind
                == mcuhw::core::transfer_error_kind::timeout);
    auto reused = backend.write(*address, bytes);
    ASSERT_TRUE(reused.has_value());
    auto cancelled = std::move(*reused);
    ASSERT_TRUE(cancelled.cancel().has_value());
    EXPECT_TRUE(cancelled.take().error().kind
                == mcuhw::core::transfer_error_kind::cancelled);
    ASSERT_TRUE(backend.shutdown().has_value());
}

TEST(ra4m1_i2c, hardware_timeout_and_opt_in_reset_recover_before_reuse,
     "REQ-0025") {
    mmio_type mmio{0x4001'E000U};
    execution_type execution;
    backend_type backend{mmio, execution};
    ASSERT_TRUE(backend.initialize(*timing_100khz).has_value());
    const std::array<std::uint8_t, 1> bytes{0x44U};
    auto started = backend.write(*address, bytes);
    ASSERT_TRUE(started.has_value());
    auto transfer = std::move(*started);
    ASSERT_TRUE(mmio.write<std::uint8_t>(0x4005'3109U, 1U << 0U)
                    .has_value());
    EXPECT_TRUE(backend.handle_error_interrupt());
    ASSERT_TRUE(mmio.write<std::uint8_t>(0x4005'3109U, 1U << 3U)
                    .has_value());
    EXPECT_TRUE(backend.handle_error_interrupt());
    const auto timed = transfer.take();
    ASSERT_FALSE(timed.has_value());
    EXPECT_TRUE(timed.error().cause == mcuhw::driver::i2c_error::timeout);

    ASSERT_TRUE(mmio.write<std::uint8_t>(0x4005'3101U, 0U).has_value());
    mmio.clear_operations();
    ASSERT_TRUE(backend.recover_bus(
        mcuhw::backend::ra4m1::iic_recovery_policy::exclusive_peripheral_reset)
                    .has_value());
    EXPECT_TRUE(has_write(mmio.operations(), 0x4005'3100U, 0xC0U));
    auto reused_start = backend.write(*address, bytes);
    ASSERT_TRUE(reused_start.has_value());
    auto reused = std::move(*reused_start);
    ASSERT_TRUE(reused.cancel().has_value());
    EXPECT_TRUE(reused.take().error().kind
                == mcuhw::core::transfer_error_kind::cancelled);
    ASSERT_TRUE(backend.shutdown().has_value());
}

struct fake_bus {
    using error_type = mcuhw::core::transfer_error<mcuhw::driver::i2c_error>;
    using result_type = std::expected<int, error_type>;

    result_type write_read(mcuhw::driver::i2c_address address,
                           std::span<const std::uint8_t> command,
                           std::span<std::uint8_t> data) noexcept {
        seen_address = address.value;
        seen_length = command.size();
        for (std::size_t i = 0; i < command.size(); ++i) {
            seen_command[i] = command[i];
        }
        if (!data.empty()) {
            data[0] = 0x5AU;
        }
        return 1;
    }

    std::array<std::uint8_t, 2> seen_command{};
    std::size_t seen_length{};
    std::uint8_t seen_address{};
};

TEST(i2c_device, same_fixed_storage_register_helper_is_bus_independent,
     "REQ-0018", "REQ-0026") {
    fake_bus bus;
    mcuhw::driver::i2c_device<fake_bus, 0x50U> device{bus};
    mcuhw::driver::i2c_register_request<4, 2> request;
    const auto result = device.read_register(
        0x1234U, request, 1U,
        mcuhw::driver::register_byte_order::most_significant_first);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(bus.seen_address, 0x50U);
    EXPECT_EQ(bus.seen_length, 2U);
    EXPECT_EQ(bus.seen_command[0], 0x12U);
    EXPECT_EQ(bus.seen_command[1], 0x34U);
    EXPECT_EQ(request.data[0], 0x5AU);

    mmio_type mmio{0x4001'E000U};
    execution_type execution;
    backend_type backend{mmio, execution};
    mcuhw::driver::i2c_controller controller{backend};
    ASSERT_TRUE(controller.initialize(*timing_100khz).has_value());
    mcuhw::driver::i2c_device<decltype(controller), 0x50U> real_device{
        controller};
    mcuhw::driver::i2c_register_request<2, 1> real_request;
    auto real_started = real_device.read_register(0x20U, real_request, 1U);
    ASSERT_TRUE(real_started.has_value());
    auto real_transfer = std::move(*real_started);
    ASSERT_TRUE(real_transfer.cancel().has_value());
    EXPECT_TRUE(real_transfer.take().error().kind
                == mcuhw::core::transfer_error_kind::cancelled);
    ASSERT_TRUE(controller.shutdown().has_value());
}

} // namespace
