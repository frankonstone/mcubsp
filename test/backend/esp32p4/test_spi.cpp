#include <array>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <span>
#include <utility>

#include <mcutest/mcutest.hpp>

#include <mcubsp/backend/esp32p4/spi.hpp>
#include <mcubsp/core/mmio.hpp>
#include <mcubsp/execution/fake_policy.hpp>

namespace {

constexpr std::uintptr_t base = mcubsp::mcu::esp32p4::spi2_base;
constexpr std::uintptr_t command = base + 0x00U;
constexpr std::uintptr_t interrupt_clear = base + 0x38U;
constexpr std::uintptr_t interrupt_raw = base + 0x3CU;
constexpr std::uintptr_t buffer = base + 0x98U;

class sparse_mmio {
public:
    template<std::unsigned_integral Value>
        requires(sizeof(Value) == 1 || sizeof(Value) == 2 || sizeof(Value) == 4)
    [[nodiscard]] std::expected<Value, mcubsp::core::mmio_error>
    read(std::uintptr_t address) noexcept {
        if (address % alignof(Value) != 0U) {
            return std::unexpected(mcubsp::core::mmio_error::misaligned);
        }
        if (fail_) return std::unexpected(mcubsp::core::mmio_error::invalid_value);
        return static_cast<Value>(value(address));
    }

    template<std::unsigned_integral Value>
        requires(sizeof(Value) == 1 || sizeof(Value) == 2 || sizeof(Value) == 4)
    [[nodiscard]] std::expected<void, mcubsp::core::mmio_error>
    write(std::uintptr_t address, Value next) noexcept {
        if (address % alignof(Value) != 0U) {
            return std::unexpected(mcubsp::core::mmio_error::misaligned);
        }
        if (fail_) return std::unexpected(mcubsp::core::mmio_error::invalid_value);
        const auto word = static_cast<std::uint32_t>(next);
        if (address == interrupt_clear) {
            set(interrupt_raw, value(interrupt_raw) & ~word);
            return {};
        }
        if (address == command && (word & (1U << 23U)) != 0U) {
            set(address, word & ~(1U << 23U));
            return {};
        }
        if (address == command && (word & (1U << 24U)) != 0U) {
            ++starts_;
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

    void complete(std::span<const std::uint8_t> received) noexcept {
        for (std::size_t index = 0U; index < received.size(); ++index) {
            const auto address = buffer + (index / 4U) * 4U;
            const auto shift = (index % 4U) * 8U;
            set(address, (value(address) & ~(0xFFU << shift))
                             | (static_cast<std::uint32_t>(received[index])
                                << shift));
        }
        set(command, value(command) & ~(1U << 24U));
        set(interrupt_raw, 1U << 12U);
    }

    void fail(bool enabled = true) noexcept { fail_ = enabled; }
    [[nodiscard]] std::size_t starts() const noexcept { return starts_; }

private:
    struct cell {
        std::uintptr_t address{};
        std::uint32_t value{};
    };
    std::array<cell, 128> cells_{};
    std::size_t count_{};
    std::size_t starts_{};
    bool fail_{};
};

using binding = mcubsp::backend::esp32p4::spi_binding<
    2, mcubsp::core::pin_id{0, 20}, mcubsp::core::pin_id{0, 21},
    mcubsp::core::pin_id{0, 22}, mcubsp::core::pin_id{0, 23}>;
using backend_type = mcubsp::backend::esp32p4::spi_master_backend<
    binding, sparse_mmio, mcubsp::execution::fake_policy>;

constexpr auto timing_1m = mcubsp::backend::esp32p4::solve_spi_timing({
    .clock_hz = 40'000'000U,
    .target_hz = 1'000'000U,
    .maximum_error_ppm = 0U,
});
static_assert(timing_1m.has_value());

TEST(esp32p4_spi, solves_native_non_overspeed_clock,
     "REQ-0011", "REQ-0028", "REQ-0034") {
    ASSERT_TRUE(timing_1m.has_value());
    EXPECT_EQ(timing_1m->achieved_hz, 1'000'000U);
    EXPECT_EQ(timing_1m->error_ppm, 0U);
    EXPECT_TRUE((timing_1m->clock_register & (1U << 31U)) == 0U);

    constexpr auto direct = mcubsp::backend::esp32p4::solve_spi_timing(
        {40'000'000U, 40'000'000U, 0U});
    ASSERT_TRUE(direct.has_value());
    EXPECT_EQ(direct->achieved_hz, 40'000'000U);
    EXPECT_EQ(direct->clock_register, 1U << 31U);

    constexpr auto below = mcubsp::backend::esp32p4::solve_spi_timing(
        {40'000'000U, 7'000'000U, 200'000U});
    ASSERT_TRUE(below.has_value());
    EXPECT_TRUE(below->achieved_hz <= 7'000'000U);
    EXPECT_TRUE(mcubsp::backend::esp32p4::solve_spi_timing(
                    {0U, 1'000'000U, 0U}).error()
                == mcubsp::driver::spi_error::invalid_clock);
    EXPECT_TRUE(mcubsp::backend::esp32p4::solve_spi_timing(
                    {40'000'000U, 40'000'001U, 0U}).error()
                == mcubsp::driver::spi_error::invalid_rate);
    EXPECT_TRUE(mcubsp::backend::esp32p4::solve_spi_timing(
                    {40'000'000U, 7'000'000U, 1U}).error()
                == mcubsp::driver::spi_error::unattainable_tolerance);
}

TEST(esp32p4_spi, configures_mode_order_rate_and_gpio_matrix,
     "REQ-0010", "REQ-0011", "REQ-0014", "REQ-0028", "REQ-0034") {
    sparse_mmio mmio;
    mcubsp::execution::fake_policy execution;
    backend_type spi{mmio, execution};
    const mcubsp::driver::spi_configuration configuration{
        .mode = mcubsp::driver::spi_mode::mode3,
        .bit_order = mcubsp::driver::spi_bit_order::least_significant_first,
        .frame_width = mcubsp::driver::spi_frame_width::bits16,
        .chip_select = mcubsp::driver::spi_chip_select_polarity::active_high,
    };
    ASSERT_TRUE(spi.initialize(*timing_1m, configuration).has_value());
    EXPECT_TRUE((mmio.value(binding::clocks.system_gate_register)
                 & binding::clocks.system_gate_mask) != 0U);
    EXPECT_TRUE((mmio.value(binding::clocks.apb_gate_register)
                 & binding::clocks.apb_gate_mask) != 0U);
    EXPECT_EQ(mmio.value(base + 0x0CU), timing_1m->clock_register);
    EXPECT_TRUE((mmio.value(base + 0x08U) & (1U << 23U)) != 0U);
    EXPECT_TRUE((mmio.value(base + 0x08U) & (1U << 25U)) != 0U);
    EXPECT_TRUE((mmio.value(base + 0x10U) & (1U << 9U)) == 0U);
    EXPECT_TRUE((mmio.value(base + 0x20U) & (1U << 29U)) != 0U);
    EXPECT_TRUE((mmio.value(base + 0x20U) & (1U << 7U)) != 0U);
    EXPECT_EQ(mmio.value(mcubsp::mcu::esp32p4::gpio_matrix_base + 0x558U
                         + binding::mosi_pin.index * 4U) & 0x1FFU,
              binding::mosi_matrix_signal);
    EXPECT_EQ(mmio.value(mcubsp::mcu::esp32p4::gpio_matrix_base + 0x158U
                         + binding::miso_matrix_signal * 4U) & 0x3FU,
              binding::miso_pin.index);
    EXPECT_TRUE(spi.shutdown().has_value());
}

TEST(esp32p4_spi, holds_native_cs_for_segments_and_cleans_up_failures,
     "REQ-0014", "REQ-0017", "REQ-0019", "REQ-0028") {
    sparse_mmio mmio;
    mcubsp::execution::fake_policy execution;
    backend_type spi{mmio, execution};
    ASSERT_TRUE(spi.initialize(*timing_1m).has_value());

    constexpr std::array<std::uint16_t, 2> command_frames{0x9FU, 0x00U};
    std::array<std::uint16_t, 2> command_response{};
    std::array<std::uint16_t, 2> read_response{};
    const std::array segments{
        mcubsp::driver::spi_segment{command_frames, command_response, 0xFFU},
        mcubsp::driver::spi_segment{{}, read_response, 0xFFU},
    };
    auto started = spi.transaction(segments);
    ASSERT_TRUE(started.has_value());
    auto transfer = std::move(*started);
    EXPECT_EQ(mmio.starts(), 1U);
    EXPECT_FALSE(transfer.ready());
    EXPECT_EQ(mmio.value(buffer), 0xFFFF'009FU);
    EXPECT_EQ(mmio.value(base + 0x1CU), 31U);

    mmio.set(interrupt_raw, 1U << 12U);
    EXPECT_FALSE(spi.handle_interrupt());
    EXPECT_FALSE(transfer.ready());
    constexpr std::array<std::uint8_t, 4> received{0x11U, 0x22U, 0x33U, 0x44U};
    mmio.complete(received);
    EXPECT_TRUE(spi.handle_interrupt());
    const auto result = transfer.take();
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->frames, 4U);
    EXPECT_EQ(command_response[0], 0x11U);
    EXPECT_EQ(command_response[1], 0x22U);
    EXPECT_EQ(read_response[0], 0x33U);
    EXPECT_EQ(read_response[1], 0x44U);

    std::array<std::uint16_t, 65> excessive{};
    const std::array too_long{
        mcubsp::driver::spi_segment{{}, excessive, 0xFFU}};
    const auto rejected = spi.transaction(too_long);
    ASSERT_FALSE(rejected.has_value());
    EXPECT_TRUE(rejected.error().cause
                == mcubsp::driver::spi_error::excessive_length);

    std::array<std::uint16_t, 1> input{};
    const std::array one{
        mcubsp::driver::spi_segment{{}, input, 0xFFU}};
    auto cancelled_start = spi.transaction(one);
    ASSERT_TRUE(cancelled_start.has_value());
    auto cancelled = std::move(*cancelled_start);
    const auto cancellation = cancelled.cancel();
    ASSERT_TRUE(cancellation.has_value());
    EXPECT_TRUE(*cancellation);
    const auto cancelled_result = cancelled.take();
    ASSERT_FALSE(cancelled_result.has_value());
    EXPECT_TRUE(cancelled_result.error().kind
                == mcubsp::core::transfer_error_kind::cancelled);

    auto failed_start = spi.transaction(one);
    ASSERT_TRUE(failed_start.has_value());
    auto failed = std::move(*failed_start);
    mmio.set(interrupt_raw, 1U << 17U);
    EXPECT_TRUE(spi.handle_interrupt());
    const auto failed_result = failed.take();
    ASSERT_FALSE(failed_result.has_value());
    EXPECT_TRUE(failed_result.error().cause
                == mcubsp::driver::spi_error::overrun);
    EXPECT_TRUE(spi.shutdown().has_value());
}

} // namespace
