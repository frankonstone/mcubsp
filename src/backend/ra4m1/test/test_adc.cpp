#include <array>
#include <cstdint>
#include <span>
#include <utility>

#include <mcutest/mcutest.hpp>

#include <mcubsp/backend/ra4m1/adc.hpp>
#include <mcubsp/board/arduino/uno_r4_minima/bindings.hpp>
#include <mcubsp/driver/adc.hpp>
#include <mcubsp/execution/fake_policy.hpp>
#include <mcubsp/testing/recording_mmio.hpp>

namespace {

using board = mcubsp::board::arduino::uno_r4_minima_bindings;
using binding = mcubsp::backend::ra4m1::adc14_binding<
    mcubsp::backend::ra4m1::adc_input<9, board::a0_an009>,
    mcubsp::backend::ra4m1::adc_input<0, board::a1_an000>,
    mcubsp::backend::ra4m1::adc_input<1, board::a2_an001>>;
using mmio_type = mcubsp::testing::recording_mmio<0x60000, 384>;
using execution_type = mcubsp::execution::fake_policy;
using backend_type = mcubsp::backend::ra4m1::adc14_backend<
    binding, mmio_type, execution_type>;

class fake_dma_backend {
public:
    using pool_type = mcubsp::core::operation_pool<
        mcubsp::driver::dma_transfer_result, mcubsp::driver::dma_error, 1,
        execution_type>;
    using transfer_type = typename pool_type::transfer_type;
    using start_result = std::expected<transfer_type,
                                       typename pool_type::error_type>;

    explicit fake_dma_backend(execution_type& execution) : pool_(execution) {}

    [[nodiscard]] start_result peripheral(
        mcubsp::driver::dma_peripheral_transfer request) noexcept {
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
        return pool_.fail(id, mcubsp::driver::dma_error::transfer_escape);
    }

    mcubsp::driver::dma_peripheral_transfer last_request{};
    std::size_t cancellations{};

private:
    [[nodiscard]] static mcubsp::core::quiesce_result quiesce(
        void* context, mcubsp::core::operation_id id) noexcept {
        auto& self = *static_cast<fake_dma_backend*>(context);
        if (!self.operation_id_ || *self.operation_id_ != id) {
            return mcubsp::core::quiesce_result::reusable;
        }
        self.operation_id_.reset();
        ++self.cancellations;
        return mcubsp::core::quiesce_result::reusable;
    }

    pool_type pool_;
    std::optional<mcubsp::core::operation_id> operation_id_{};
};

[[nodiscard]] bool has_write(std::span<const mcubsp::testing::mmio_operation> ops,
                             std::uintptr_t address, std::uint64_t value) {
    for (const auto& operation : ops) {
        if (operation.kind == mcubsp::testing::mmio_operation_kind::write
            && operation.address == address && operation.value == value) {
            return true;
        }
    }
    return false;
}

TEST(adc_contract, validates_channels_and_sample_timing, "REQ-0029") {
    EXPECT_TRUE(mcubsp::driver::make_adc_channel(0U).has_value());
    EXPECT_TRUE(mcubsp::driver::make_adc_channel(14U).has_value());
    EXPECT_TRUE(mcubsp::driver::make_adc_channel(19U).has_value());
    EXPECT_TRUE(mcubsp::driver::make_adc_channel(20U).error()
                == mcubsp::driver::adc_error::invalid_channel);

    mmio_type mmio{0x4001'E000U};
    execution_type execution;
    backend_type backend{mmio, execution};
    const auto rejected = backend.initialize({.sample_states = 5U});
    ASSERT_FALSE(rejected.has_value());
    EXPECT_TRUE(rejected.error()
                == mcubsp::driver::adc_error::invalid_sample_time);
}

TEST(ra4m1_adc, configures_resolution_sampling_and_analog_pins,
     "REQ-0029") {
    mmio_type mmio{0x4001'E000U};
    execution_type execution;
    backend_type backend{mmio, execution};
    ASSERT_TRUE(backend.initialize({
        .resolution = mcubsp::driver::adc_resolution::bits14,
        .sample_states = 21U,
    }).has_value());
    EXPECT_TRUE(has_write(mmio.operations(), 0x4005'C00EU, 0x0006U));
    EXPECT_TRUE(has_write(mmio.operations(), 0x4005'C0E0U, 21U));
    EXPECT_TRUE(has_write(mmio.operations(), 0x4005'C0E1U, 21U));
    EXPECT_TRUE(has_write(mmio.operations(), 0x4005'C0E9U, 21U));
    EXPECT_TRUE(has_write(mmio.operations(), 0x4004'0800U, 1U << 15U));
    EXPECT_TRUE(has_write(mmio.operations(), 0x4004'0804U, 1U << 15U));
    EXPECT_TRUE(has_write(mmio.operations(), 0x4004'0838U, 1U << 15U));
    ASSERT_TRUE(backend.shutdown().has_value());
}

TEST(ra4m1_adc, finite_scan_returns_raw_results_in_requested_order,
     "REQ-0017", "REQ-0029") {
    mmio_type mmio{0x4001'E000U};
    execution_type execution;
    backend_type backend{mmio, execution};
    ASSERT_TRUE(backend.initialize().has_value());
    const std::array channels{
        mcubsp::driver::adc_channel{1U}, mcubsp::driver::adc_channel{0U}};
    std::array<std::uint16_t, 2> results{};
    auto started = backend.convert(channels, results);
    ASSERT_TRUE(started.has_value());
    auto transfer = std::move(*started);
    EXPECT_TRUE(has_write(mmio.operations(), 0x4005'C004U, 0x0003U));
    EXPECT_TRUE(has_write(mmio.operations(), 0x4005'C000U, 0x8000U));
    ASSERT_TRUE(mmio.write<std::uint16_t>(0x4005'C000U, 0U).has_value());
    ASSERT_TRUE(mmio.write<std::uint16_t>(0x4005'C020U, 0x0123U).has_value());
    ASSERT_TRUE(mmio.write<std::uint16_t>(0x4005'C022U, 0x3ABCU).has_value());
    EXPECT_TRUE(backend.handle_scan_end_interrupt());
    const auto complete = transfer.take();
    ASSERT_TRUE(complete.has_value());
    EXPECT_EQ(complete->samples, 2U);
    EXPECT_EQ(results[0], 0x3ABCU);
    EXPECT_EQ(results[1], 0x0123U);
    ASSERT_TRUE(backend.shutdown().has_value());
}

TEST(ra4m1_adc, continuous_single_channel_dma_stops_before_completion,
     "REQ-0017", "REQ-0018", "REQ-0029", "REQ-0030") {
    mmio_type mmio{0x4001'E000U};
    execution_type adc_execution;
    execution_type dma_execution;
    backend_type backend{mmio, adc_execution};
    fake_dma_backend dma{dma_execution};
    ASSERT_TRUE(backend.initialize().has_value());
    std::array<std::uint16_t, 8> samples{};
    auto started = backend.sample_dma(
        mcubsp::driver::adc_channel{9U}, samples, dma);
    ASSERT_TRUE(started.has_value());
    auto transfer = std::move(*started);
    EXPECT_EQ(dma.last_request.event, 0x29U);
    EXPECT_EQ(dma.last_request.peripheral, 0x4005'C032U);
    EXPECT_EQ(dma.last_request.count, samples.size());
    EXPECT_TRUE(dma.last_request.width == mcubsp::driver::dma_width::bits16);
    EXPECT_TRUE(dma.last_request.direction
                == mcubsp::driver::dma_peripheral_direction::peripheral_to_memory);
    EXPECT_TRUE(has_write(mmio.operations(), 0x4005'C004U, 1U << 9U));
    EXPECT_TRUE(has_write(mmio.operations(), 0x4005'C000U, 0x4000U));
    EXPECT_TRUE(has_write(mmio.operations(), 0x4005'C000U, 0xC000U));
    ASSERT_TRUE(dma.complete());
    EXPECT_TRUE(backend.handle_dma_completion());
    const auto result = transfer.take();
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->samples, samples.size());
    EXPECT_TRUE(has_write(mmio.operations(), 0x4005'C000U, 0U));
    EXPECT_TRUE(has_write(mmio.operations(), 0x4005'C004U, 0U));
    ASSERT_TRUE(backend.shutdown().has_value());
}

TEST(ra4m1_adc, dma_failure_and_timeout_stop_scan_and_quiesce_request,
     "REQ-0019", "REQ-0029", "REQ-0030") {
    mmio_type mmio{0x4001'E000U};
    execution_type adc_execution;
    execution_type dma_execution;
    backend_type backend{mmio, adc_execution};
    fake_dma_backend dma{dma_execution};
    ASSERT_TRUE(backend.initialize().has_value());
    std::array<std::uint16_t, 4> samples{};
    auto failed_start = backend.sample_dma(
        mcubsp::driver::adc_channel{9U}, samples, dma);
    ASSERT_TRUE(failed_start.has_value());
    auto failed_transfer = std::move(*failed_start);
    ASSERT_TRUE(dma.fail());
    EXPECT_TRUE(backend.handle_dma_completion());
    const auto failed = failed_transfer.take();
    ASSERT_FALSE(failed.has_value());
    EXPECT_TRUE(failed.error().cause == mcubsp::driver::adc_error::dma);

    auto timed_start = backend.sample_dma(
        mcubsp::driver::adc_channel{9U}, samples, dma);
    ASSERT_TRUE(timed_start.has_value());
    auto timed = std::move(*timed_start);
    const auto waited = timed.wait_until(mcubsp::execution::deadline{0U});
    ASSERT_FALSE(waited.has_value());
    EXPECT_TRUE(waited.error().kind
                == mcubsp::core::transfer_error_kind::timeout);
    EXPECT_EQ(dma.cancellations, 1U);
    EXPECT_TRUE(has_write(mmio.operations(), 0x4005'C000U, 0U));
    ASSERT_TRUE(backend.shutdown().has_value());
}

TEST(ra4m1_adc, rejects_bad_sequences_and_timeout_quiesces_converter,
     "REQ-0019", "REQ-0029") {
    mmio_type mmio{0x4001'E000U};
    execution_type execution;
    backend_type backend{mmio, execution};
    ASSERT_TRUE(backend.initialize().has_value());
    const std::array duplicate{
        mcubsp::driver::adc_channel{0U}, mcubsp::driver::adc_channel{0U}};
    std::array<std::uint16_t, 2> values{};
    const auto rejected = backend.convert(duplicate, values);
    ASSERT_FALSE(rejected.has_value());
    EXPECT_TRUE(rejected.error().cause
                == mcubsp::driver::adc_error::duplicate_channel);

    const std::array channels{mcubsp::driver::adc_channel{0U}};
    auto started = backend.convert(channels,
                                   std::span<std::uint16_t>{values}.first(1));
    ASSERT_TRUE(started.has_value());
    auto transfer = std::move(*started);
    const auto waited = transfer.wait_until(mcubsp::execution::deadline{0U});
    ASSERT_FALSE(waited.has_value());
    EXPECT_TRUE(waited.error().kind
                == mcubsp::core::transfer_error_kind::timeout);
    EXPECT_TRUE(has_write(mmio.operations(), 0x4005'C000U, 0U));
    EXPECT_TRUE(has_write(mmio.operations(), 0x4005'C004U, 0U));
    ASSERT_TRUE(backend.shutdown().has_value());
}

} // namespace
