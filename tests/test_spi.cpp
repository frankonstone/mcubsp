#include <array>
#include <cstdint>
#include <span>
#include <utility>

#include <mcutest/mcutest.hpp>

#include <mcuhw/backend/ra4m1/spi.hpp>
#include <mcuhw/board/arduino_uno_r4_minima.hpp>
#include <mcuhw/driver/spi.hpp>
#include <mcuhw/execution/fake_policy.hpp>
#include <mcuhw/testing/recording_mmio.hpp>

namespace {

namespace board = mcuhw::board::arduino_uno_r4_minima;
using binding = mcuhw::backend::ra4m1::spi_binding<
    1, board::d12_miso, board::d11_mosi, board::d13_sck, board::d10_ssl>;
using mmio_type = mcuhw::testing::recording_mmio<0x60000, 512>;
using execution_type = mcuhw::execution::fake_policy;
using backend_type = mcuhw::backend::ra4m1::spi_master_backend<
    binding, mmio_type, execution_type>;

class fake_dma_backend {
public:
    using pool_type = mcuhw::core::operation_pool<
        mcuhw::driver::dma_transfer_result, mcuhw::driver::dma_error, 1,
        execution_type>;
    using transfer_type = typename pool_type::transfer_type;
    using start_result = std::expected<transfer_type,
                                       typename pool_type::error_type>;

    explicit fake_dma_backend(execution_type& execution) : pool_(execution) {}

    [[nodiscard]] start_result peripheral(
        mcuhw::driver::dma_peripheral_transfer request) noexcept {
        last_request = request;
        auto started = pool_.try_start({.quiesce = &quiesce, .context = this});
        if (started) operation_id_ = started->id();
        return started;
    }

    [[nodiscard]] bool complete() noexcept {
        if (!operation_id_) return false;
        const auto id = *operation_id_;
        operation_id_.reset();
        return pool_.complete(id, {
            .units = last_request.count,
            .width = last_request.width,
        });
    }

    [[nodiscard]] bool fail() noexcept {
        if (!operation_id_) return false;
        const auto id = *operation_id_;
        operation_id_.reset();
        return pool_.fail(id, mcuhw::driver::dma_error::transfer_escape);
    }

    mcuhw::driver::dma_peripheral_transfer last_request{};
    std::size_t cancellations{};

private:
    [[nodiscard]] static mcuhw::core::quiesce_result quiesce(
        void* context, mcuhw::core::operation_id id) noexcept {
        auto& self = *static_cast<fake_dma_backend*>(context);
        if (!self.operation_id_ || *self.operation_id_ != id) {
            return mcuhw::core::quiesce_result::reusable;
        }
        self.operation_id_.reset();
        ++self.cancellations;
        return mcuhw::core::quiesce_result::reusable;
    }

    pool_type pool_;
    std::optional<mcuhw::core::operation_id> operation_id_{};
};

constexpr auto one_mhz = mcuhw::driver::solve_spi_timing({
    .clock_hz = 48'000'000U,
    .target_hz = 1'000'000U,
    .maximum_error_ppm = 0U,
});
static_assert(one_mhz.has_value());
static_assert(one_mhz->spbr == 23U);
static_assert(board::d10_ssl == mcuhw::core::pin_id{1U, 12U});

[[nodiscard]] bool has_write(std::span<const mcuhw::testing::mmio_operation> ops,
                             std::uintptr_t address, std::uint64_t value) {
    for (const auto& operation : ops) {
        if (operation.kind == mcuhw::testing::mmio_operation_kind::write
            && operation.address == address && operation.value == value) {
            return true;
        }
    }
    return false;
}

TEST(spi_solver, selects_non_overspeed_rate_and_rejects_bad_requests,
     "REQ-0028") {
    ASSERT_TRUE(one_mhz.has_value());
    EXPECT_EQ(one_mhz->achieved_hz, 1'000'000U);
    const auto odd = mcuhw::driver::solve_spi_timing(
        {48'000'000U, 7'000'000U, 150'000U});
    ASSERT_TRUE(odd.has_value());
    EXPECT_TRUE(odd->achieved_hz <= 7'000'000U);
    EXPECT_TRUE(mcuhw::driver::solve_spi_timing({0U, 1U, 1U}).error()
                == mcuhw::driver::spi_error::invalid_clock);
    EXPECT_TRUE(mcuhw::driver::solve_spi_timing(
                    {48'000'000U, 30'000'000U, 1U}).error()
                == mcuhw::driver::spi_error::invalid_rate);
    EXPECT_TRUE(mcuhw::driver::solve_spi_timing(
                    {48'000'000U, 7'000'000U, 0U}).error()
                == mcuhw::driver::spi_error::unattainable_tolerance);
}

TEST(ra4m1_spi, configures_mode_order_width_rate_and_uno_routes,
     "REQ-0028") {
    mmio_type mmio{0x4001'E000U};
    execution_type execution;
    backend_type backend{mmio, execution};
    ASSERT_TRUE(backend.initialize(*one_mhz, {
        .mode = mcuhw::driver::spi_mode::mode3,
        .bit_order = mcuhw::driver::spi_bit_order::least_significant_first,
        .frame_width = mcuhw::driver::spi_frame_width::bits16,
    }).has_value());
    EXPECT_TRUE(has_write(mmio.operations(), 0x4007'210AU, 23U));
    EXPECT_TRUE(has_write(mmio.operations(), 0x4007'210BU, 0U));
    EXPECT_TRUE(has_write(mmio.operations(), 0x4007'2110U, 0x1F03U));
    EXPECT_TRUE(has_write(mmio.operations(), 0x4004'0868U,
                          (0x06U << 24U) | (1U << 16U)));
    EXPECT_TRUE(has_write(mmio.operations(), 0x4004'0864U,
                          (0x06U << 24U) | (1U << 16U)));
    EXPECT_TRUE(has_write(mmio.operations(), 0x4004'086CU,
                          (0x06U << 24U) | (1U << 16U)));
    EXPECT_TRUE(has_write(mmio.operations(), 0x4004'0870U,
                          (1U << 2U) | 1U));
    ASSERT_TRUE(backend.shutdown().has_value());
}

TEST(ra4m1_spi, holds_cs_across_segments_and_finishes_only_when_idle,
     "REQ-0017", "REQ-0028") {
    mmio_type mmio{0x4001'E000U};
    execution_type execution;
    backend_type backend{mmio, execution};
    ASSERT_TRUE(backend.initialize(*one_mhz).has_value());
    const std::array<std::uint16_t, 2> command{0x11U, 0x22U};
    std::array<std::uint16_t, 2> response{};
    const std::array segments{
        mcuhw::driver::spi_segment{.transmit = command},
        mcuhw::driver::spi_segment{.receive = response, .dummy = 0xA5U},
    };
    auto started = backend.transaction(segments);
    ASSERT_TRUE(started.has_value());
    auto transfer = std::move(*started);
    constexpr std::array<std::uint8_t, 4> received{0x91U, 0x92U, 0x31U, 0x32U};
    for (const auto value : received) {
        EXPECT_TRUE(backend.handle_transmit_empty_interrupt());
        ASSERT_TRUE(mmio.write<std::uint8_t>(0x4007'2104U, value).has_value());
        EXPECT_TRUE(backend.handle_receive_interrupt());
    }
    EXPECT_FALSE(transfer.ready());
    ASSERT_TRUE(mmio.write<std::uint8_t>(0x4007'2103U, 0x22U).has_value());
    EXPECT_FALSE(backend.handle_idle_interrupt());
    ASSERT_TRUE(mmio.write<std::uint8_t>(0x4007'2103U, 0x20U).has_value());
    EXPECT_TRUE(backend.handle_idle_interrupt());
    const auto result = transfer.take();
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->frames, 4U);
    EXPECT_EQ(response[0], 0x31U);
    EXPECT_EQ(response[1], 0x32U);
    EXPECT_TRUE(has_write(mmio.operations(), 0x4004'0028U, 0x1000'0000U));
    EXPECT_TRUE(has_write(mmio.operations(), 0x4004'0028U, 0x0000'1000U));
    EXPECT_TRUE(has_write(mmio.operations(), 0x4007'2104U, 0xA5U));
    ASSERT_TRUE(backend.shutdown().has_value());
}

TEST(ra4m1_spi, dma_full_duplex_waits_for_both_channels_and_wire_idle,
     "REQ-0017", "REQ-0018", "REQ-0028", "REQ-0030") {
    mmio_type mmio{0x4001'E000U};
    execution_type spi_execution;
    execution_type receive_execution;
    execution_type transmit_execution;
    backend_type backend{mmio, spi_execution};
    fake_dma_backend receive_dma{receive_execution};
    fake_dma_backend transmit_dma{transmit_execution};
    ASSERT_TRUE(backend.initialize(*one_mhz).has_value());
    const std::array<std::uint8_t, 4> command{0x0FU, 0x00U, 0x17U, 0x00U};
    std::array<std::uint8_t, 4> response{};
    auto started = backend.transaction_dma({command, response},
                                           receive_dma, transmit_dma);
    ASSERT_TRUE(started.has_value());
    auto transfer = std::move(*started);
    EXPECT_EQ(receive_dma.last_request.event, 0xB2U);
    EXPECT_EQ(transmit_dma.last_request.event, 0xB3U);
    EXPECT_TRUE(receive_dma.last_request.direction
                == mcuhw::driver::dma_peripheral_direction::peripheral_to_memory);
    EXPECT_TRUE(transmit_dma.last_request.direction
                == mcuhw::driver::dma_peripheral_direction::memory_to_peripheral);
    EXPECT_EQ(receive_dma.last_request.peripheral, 0x4007'2104U);
    EXPECT_FALSE(transfer.ready());
    ASSERT_TRUE(transmit_dma.complete());
    EXPECT_TRUE(backend.handle_dma_transmit_completion());
    EXPECT_FALSE(transfer.ready());
    ASSERT_TRUE(receive_dma.complete());
    EXPECT_TRUE(backend.handle_dma_receive_completion());
    EXPECT_FALSE(transfer.ready());
    EXPECT_TRUE(has_write(mmio.operations(), 0x4007'2100U, 0x59U));
    ASSERT_TRUE(mmio.write<std::uint8_t>(0x4007'2103U, 0x22U).has_value());
    EXPECT_FALSE(backend.handle_idle_interrupt());
    ASSERT_TRUE(mmio.write<std::uint8_t>(0x4007'2103U, 0x20U).has_value());
    EXPECT_TRUE(backend.handle_idle_interrupt());
    const auto result = transfer.take();
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->frames, command.size());
    ASSERT_TRUE(backend.shutdown().has_value());
}

TEST(ra4m1_spi, dma_failure_cancels_peer_and_releases_chip_select,
     "REQ-0019", "REQ-0028", "REQ-0030") {
    mmio_type mmio{0x4001'E000U};
    execution_type spi_execution;
    execution_type receive_execution;
    execution_type transmit_execution;
    backend_type backend{mmio, spi_execution};
    fake_dma_backend receive_dma{receive_execution};
    fake_dma_backend transmit_dma{transmit_execution};
    ASSERT_TRUE(backend.initialize(*one_mhz).has_value());
    const std::array<std::uint8_t, 1> command{0x00U};
    std::array<std::uint8_t, 1> response{};
    auto started = backend.transaction_dma({command, response},
                                           receive_dma, transmit_dma);
    ASSERT_TRUE(started.has_value());
    auto transfer = std::move(*started);
    ASSERT_TRUE(receive_dma.fail());
    EXPECT_TRUE(backend.handle_dma_receive_completion());
    const auto result = transfer.take();
    ASSERT_FALSE(result.has_value());
    EXPECT_TRUE(result.error().cause == mcuhw::driver::spi_error::dma);
    EXPECT_EQ(transmit_dma.cancellations, 1U);
    EXPECT_TRUE(has_write(mmio.operations(), 0x4004'0028U, 0x0000'1000U));
    ASSERT_TRUE(backend.shutdown().has_value());
}

TEST(ra4m1_spi, rejects_invalid_segments_and_cleans_up_errors_and_timeout,
     "REQ-0019", "REQ-0028") {
    mmio_type mmio{0x4001'E000U};
    execution_type execution;
    backend_type backend{mmio, execution};
    ASSERT_TRUE(backend.initialize(*one_mhz).has_value());
    const std::array<std::uint16_t, 2> tx{1U, 2U};
    std::array<std::uint16_t, 1> rx{};
    const std::array bad{mcuhw::driver::spi_segment{tx, rx, 0U}};
    const auto rejected = backend.transaction(bad);
    ASSERT_FALSE(rejected.has_value());
    EXPECT_TRUE(rejected.error().cause
                == mcuhw::driver::spi_error::mismatched_segment);

    const std::array good{mcuhw::driver::spi_segment{.transmit = tx}};
    auto errored_start = backend.transaction(good);
    ASSERT_TRUE(errored_start.has_value());
    auto errored = std::move(*errored_start);
    ASSERT_TRUE(mmio.write<std::uint8_t>(0x4007'2103U, 1U).has_value());
    EXPECT_TRUE(backend.handle_error_interrupt());
    EXPECT_TRUE(errored.take().error().cause
                == mcuhw::driver::spi_error::overrun);

    auto timed_start = backend.transaction(good);
    ASSERT_TRUE(timed_start.has_value());
    auto timed = std::move(*timed_start);
    const auto waited = timed.wait_until(mcuhw::execution::deadline{0U});
    ASSERT_FALSE(waited.has_value());
    EXPECT_TRUE(waited.error().kind
                == mcuhw::core::transfer_error_kind::timeout);
    EXPECT_TRUE(has_write(mmio.operations(), 0x4004'0028U, 0x0000'1000U));
    ASSERT_TRUE(backend.shutdown().has_value());
}

} // namespace
