#include <array>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <span>
#include <utility>

#include <mcutest/mcutest.hpp>

#include <mcuhw/backend/esp32p4/i2c.hpp>
#include <mcuhw/board/esp32p4_nano.hpp>
#include <mcuhw/core/mmio.hpp>
#include <mcuhw/execution/fake_policy.hpp>

namespace {

constexpr std::uintptr_t i2c_base = mcuhw::mcu::esp32p4::i2c0_base;
constexpr std::uintptr_t status_register = i2c_base + 0x08U;
constexpr std::uintptr_t data_register = i2c_base + 0x1CU;
constexpr std::uintptr_t interrupt_status = i2c_base + 0x2CU;
constexpr std::uintptr_t command0 = i2c_base + 0x58U;

class sparse_mmio {
public:
    template<std::unsigned_integral Value>
        requires(sizeof(Value) == 1 || sizeof(Value) == 2 || sizeof(Value) == 4)
    [[nodiscard]] std::expected<Value, mcuhw::core::mmio_error>
    read(std::uintptr_t address) noexcept {
        if (address % alignof(Value) != 0U) {
            return std::unexpected(mcuhw::core::mmio_error::misaligned);
        }
        if (address == data_register && rx_index_ < rx_size_) {
            return static_cast<Value>(received_[rx_index_++]);
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
        const auto word = static_cast<std::uint32_t>(next);
        if (address == data_register && tx_size_ < transmitted_.size()) {
            transmitted_[tx_size_++] = static_cast<std::uint8_t>(word);
        }
        if (address == i2c_base + 0x24U) {
            set(interrupt_status, value(interrupt_status) & ~word);
            return {};
        }
        if (address == i2c_base + 0x04U) {
            if ((word & (1U << 5U)) != 0U) started_ = true;
            set(address, word & ~((1U << 5U) | (1U << 10U)
                                  | (1U << 11U)));
            return {};
        }
        if (address == i2c_base + 0x80U && (word & 1U) != 0U) {
            set(address, word & ~1U);
            return {};
        }
        set(address, word);
        return {};
    }

    void set(std::uintptr_t address, std::uint32_t next) noexcept {
        for (std::size_t index = 0U; index < count_; ++index) {
            if (cells_[index].address == address) {
                cells_[index].value = next;
                return;
            }
        }
        if (count_ < cells_.size()) cells_[count_++] = {address, next};
    }

    [[nodiscard]] std::uint32_t value(std::uintptr_t address) const noexcept {
        for (std::size_t index = 0U; index < count_; ++index) {
            if (cells_[index].address == address) return cells_[index].value;
        }
        return 0U;
    }

    void queue_receive(std::span<const std::uint8_t> bytes) noexcept {
        rx_index_ = 0U;
        rx_size_ = bytes.size();
        for (std::size_t index = 0U; index < bytes.size(); ++index) {
            received_[index] = bytes[index];
        }
        set(status_register, static_cast<std::uint32_t>(bytes.size()) << 8U);
    }

    [[nodiscard]] std::span<const std::uint8_t> transmitted() const noexcept {
        return {transmitted_.data(), tx_size_};
    }
    [[nodiscard]] bool started() const noexcept { return started_; }

private:
    struct cell {
        std::uintptr_t address{};
        std::uint32_t value{};
    };
    std::array<cell, 128> cells_{};
    std::array<std::uint8_t, 64> transmitted_{};
    std::array<std::uint8_t, 32> received_{};
    std::size_t count_{};
    std::size_t tx_size_{};
    std::size_t rx_size_{};
    std::size_t rx_index_{};
    bool started_{};
};

using binding = mcuhw::backend::esp32p4::i2c_binding<
    0, mcuhw::board::esp32p4_nano::default_i2c_scl,
    mcuhw::board::esp32p4_nano::default_i2c_sda>;
using backend_type = mcuhw::backend::esp32p4::i2c_backend<
    binding, sparse_mmio, mcuhw::execution::fake_policy>;

constexpr auto timing_100k = mcuhw::backend::esp32p4::solve_i2c_timing({
    .clock_hz = 40'000'000U,
    .target_hz = 100'000U,
    .maximum_error_ppm = 1'000U,
    .rise_time_ns = 300U,
    .fall_time_ns = 300U,
});
static_assert(timing_100k.has_value());

[[nodiscard]] constexpr std::uint32_t opcode(std::uint32_t command) noexcept {
    return (command >> 11U) & 0x7U;
}

TEST(esp32p4_i2c, solves_standard_and_fast_mode_timing,
     "REQ-0011", "REQ-0024", "REQ-0034") {
    ASSERT_TRUE(timing_100k.has_value());
    EXPECT_EQ(timing_100k->achieved_hz, 100'000U);
    EXPECT_EQ(timing_100k->error_ppm, 0U);
    EXPECT_TRUE(timing_100k->low_count != 0U);
    EXPECT_TRUE(timing_100k->high_count >= 6U);

    constexpr auto timing_400k =
        mcuhw::backend::esp32p4::solve_i2c_timing({
            .clock_hz = 40'000'000U,
            .target_hz = 400'000U,
            .maximum_error_ppm = 1'000U,
            .rise_time_ns = 300U,
            .fall_time_ns = 300U,
        });
    ASSERT_TRUE(timing_400k.has_value());
    EXPECT_EQ(timing_400k->achieved_hz, 400'000U);
    EXPECT_TRUE(mcuhw::backend::esp32p4::solve_i2c_timing(
                    {80'000'000U, 100'000U, 1'000U, 300U, 300U}).error()
                == mcuhw::driver::i2c_error::invalid_clock);
    EXPECT_TRUE(mcuhw::backend::esp32p4::solve_i2c_timing(
                    {40'000'000U, 0U, 1'000U, 300U, 300U}).error()
                == mcuhw::driver::i2c_error::invalid_rate);
    EXPECT_TRUE(mcuhw::backend::esp32p4::solve_i2c_timing(
                    {40'000'000U, 400'000U, 1'000U, 301U, 300U}).error()
                == mcuhw::driver::i2c_error::invalid_rise_fall);
}

TEST(esp32p4_i2c, initializes_clock_timing_and_gpio_matrix,
     "REQ-0014", "REQ-0024", "REQ-0034") {
    sparse_mmio mmio;
    mcuhw::execution::fake_policy execution;
    backend_type backend{mmio, execution};
    ASSERT_TRUE(backend.initialize(*timing_100k).has_value());

    EXPECT_TRUE((mmio.value(binding::clocks.apb_gate_register)
                 & binding::clocks.apb_gate_mask) != 0U);
    EXPECT_TRUE((mmio.value(binding::clocks.source_register) & (1U << 1U))
                != 0U);
    EXPECT_EQ((mmio.value(binding::clocks.divider_register) >> 2U) & 0xFFU,
              timing_100k->source_divider - 1U);
    EXPECT_EQ(mmio.value(i2c_base + 0x00U), timing_100k->low_count - 1U);
    EXPECT_TRUE((mmio.value(i2c_base + 0x04U) & (1U << 4U)) != 0U);
    EXPECT_TRUE((mmio.value(i2c_base + 0x04U) & (1U << 9U)) != 0U);

    const auto scl_pad = mcuhw::mcu::esp32p4::io_mux_base + 0x04U
        + binding::scl_pin.index * 4U;
    const auto scl_out = mcuhw::mcu::esp32p4::gpio_matrix_base + 0x558U
        + binding::scl_pin.index * 4U;
    const auto scl_in = mcuhw::mcu::esp32p4::gpio_matrix_base + 0x158U
        + binding::scl_matrix_signal * 4U;
    EXPECT_EQ((mmio.value(scl_pad) >> 12U) & 0x7U, 1U);
    EXPECT_TRUE((mmio.value(scl_pad) & (1U << 9U)) != 0U);
    EXPECT_EQ(mmio.value(scl_out) & 0x1FFU,
              binding::scl_matrix_signal);
    EXPECT_EQ(mmio.value(scl_in) & 0x3FU, binding::scl_pin.index);
    EXPECT_TRUE((mmio.value(scl_in) & (1U << 7U)) != 0U);
    EXPECT_TRUE(backend.shutdown().has_value());
}

TEST(esp32p4_i2c, combined_transfer_uses_repeated_start_and_explicit_stop,
     "REQ-0014", "REQ-0017", "REQ-0019", "REQ-0024") {
    sparse_mmio mmio;
    mcuhw::execution::fake_policy execution;
    backend_type backend{mmio, execution};
    ASSERT_TRUE(backend.initialize(*timing_100k).has_value());
    constexpr std::array<std::uint8_t, 1> output{0x10U};
    std::array<std::uint8_t, 2> input{};
    const auto address = mcuhw::driver::make_i2c_address(0x50U);
    ASSERT_TRUE(address.has_value());
    auto started = backend.write_read(*address, output, input);
    ASSERT_TRUE(started.has_value());
    auto transfer = std::move(*started);

    ASSERT_EQ(mmio.transmitted().size(), 3U);
    EXPECT_EQ(mmio.transmitted()[0], 0xA0U);
    EXPECT_EQ(mmio.transmitted()[1], output[0]);
    EXPECT_EQ(mmio.transmitted()[2], 0xA1U);
    EXPECT_EQ(opcode(mmio.value(command0 + 0U * 4U)), 6U);
    EXPECT_EQ(opcode(mmio.value(command0 + 1U * 4U)), 1U);
    EXPECT_EQ(opcode(mmio.value(command0 + 2U * 4U)), 1U);
    EXPECT_EQ(opcode(mmio.value(command0 + 3U * 4U)), 6U);
    EXPECT_EQ(opcode(mmio.value(command0 + 4U * 4U)), 1U);
    EXPECT_EQ(opcode(mmio.value(command0 + 5U * 4U)), 3U);
    EXPECT_EQ(opcode(mmio.value(command0 + 6U * 4U)), 3U);
    EXPECT_TRUE((mmio.value(command0 + 6U * 4U) & (1U << 10U)) != 0U);
    EXPECT_EQ(opcode(mmio.value(command0 + 7U * 4U)), 2U);
    EXPECT_TRUE(mmio.started());
    EXPECT_FALSE(transfer.ready());

    constexpr std::array<std::uint8_t, 2> response{0xABU, 0xCDU};
    mmio.queue_receive(response);
    mmio.set(interrupt_status, 1U << 7U);
    EXPECT_TRUE(backend.handle_interrupt());
    const auto result = transfer.take();
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->bytes_written, output.size());
    EXPECT_EQ(result->bytes_read, input.size());
    EXPECT_EQ(input[0], response[0]);
    EXPECT_EQ(input[1], response[1]);
    EXPECT_TRUE(backend.shutdown().has_value());
}

TEST(esp32p4_i2c, reports_nack_arbitration_timeout_bounds_and_recovery,
     "REQ-0014", "REQ-0017", "REQ-0019", "REQ-0024", "REQ-0025") {
    sparse_mmio mmio;
    mcuhw::execution::fake_policy execution;
    backend_type backend{mmio, execution};
    ASSERT_TRUE(backend.initialize(*timing_100k).has_value());
    constexpr std::array<std::uint8_t, 1> byte{0x55U};
    const auto address = mcuhw::driver::make_i2c_address(0x50U);
    ASSERT_TRUE(address.has_value());

    auto address_nack_start = backend.write(*address, byte);
    ASSERT_TRUE(address_nack_start.has_value());
    auto address_nack = std::move(*address_nack_start);
    mmio.set(interrupt_status, 1U << 10U);
    EXPECT_TRUE(backend.handle_interrupt());
    const auto address_result = address_nack.take();
    ASSERT_FALSE(address_result.has_value());
    EXPECT_TRUE(address_result.error().cause
                == mcuhw::driver::i2c_error::address_nack);

    mmio.set(status_register, 0U);
    auto data_nack_start = backend.write(*address, byte);
    ASSERT_TRUE(data_nack_start.has_value());
    auto data_nack = std::move(*data_nack_start);
    mmio.set(command0 + 1U * 4U,
             mmio.value(command0 + 1U * 4U) | (1U << 31U));
    mmio.set(interrupt_status, 1U << 10U);
    EXPECT_TRUE(backend.handle_interrupt());
    const auto data_result = data_nack.take();
    ASSERT_FALSE(data_result.has_value());
    EXPECT_TRUE(data_result.error().cause
                == mcuhw::driver::i2c_error::data_nack);

    mmio.set(status_register, 0U);
    auto arbitration_start = backend.write(*address, byte);
    ASSERT_TRUE(arbitration_start.has_value());
    auto arbitration = std::move(*arbitration_start);
    mmio.set(interrupt_status, 1U << 5U);
    EXPECT_TRUE(backend.handle_interrupt());
    const auto arbitration_result = arbitration.take();
    ASSERT_FALSE(arbitration_result.has_value());
    EXPECT_TRUE(arbitration_result.error().cause
                == mcuhw::driver::i2c_error::arbitration_lost);

    mmio.set(status_register, 0U);
    auto timeout_start = backend.write(*address, byte);
    ASSERT_TRUE(timeout_start.has_value());
    auto timed_out = std::move(*timeout_start);
    mmio.set(interrupt_status, 1U << 8U);
    EXPECT_TRUE(backend.handle_interrupt());
    const auto timeout_result = timed_out.take();
    ASSERT_FALSE(timeout_result.has_value());
    EXPECT_TRUE(timeout_result.error().cause
                == mcuhw::driver::i2c_error::timeout);

    mmio.set(status_register, 0U);
    auto cancelled_start = backend.write(*address, byte);
    ASSERT_TRUE(cancelled_start.has_value());
    auto cancelled = std::move(*cancelled_start);
    const auto busy = backend.write(*address, byte);
    ASSERT_FALSE(busy.has_value());
    EXPECT_TRUE(busy.error().cause == mcuhw::driver::i2c_error::busy);
    const auto cancellation = cancelled.cancel();
    ASSERT_TRUE(cancellation.has_value());
    EXPECT_TRUE(*cancellation);
    const auto cancelled_result = cancelled.take();
    ASSERT_FALSE(cancelled_result.has_value());
    EXPECT_TRUE(cancelled_result.error().kind
                == mcuhw::core::transfer_error_kind::cancelled);

    std::array<std::uint8_t, 33> excessive{};
    const auto too_long = backend.write(*address, excessive);
    ASSERT_FALSE(too_long.has_value());
    EXPECT_TRUE(too_long.error().cause
                == mcuhw::driver::i2c_error::excessive_length);
    const auto empty = backend.write(*address, {});
    ASSERT_FALSE(empty.has_value());
    EXPECT_TRUE(empty.error().cause
                == mcuhw::driver::i2c_error::empty_transfer);

    mmio.set(status_register, 1U << 4U);
    const auto stuck = backend.recover_bus(
        mcuhw::backend::esp32p4::i2c_recovery_policy::exclusive_peripheral_reset);
    ASSERT_FALSE(stuck.has_value());
    EXPECT_TRUE(stuck.error() == mcuhw::driver::i2c_error::bus_stuck);
    mmio.set(status_register, 0U);
    EXPECT_TRUE(backend.recover_bus(
        mcuhw::backend::esp32p4::i2c_recovery_policy::exclusive_peripheral_reset)
                    .has_value());
    EXPECT_TRUE(backend.shutdown().has_value());
}

} // namespace
