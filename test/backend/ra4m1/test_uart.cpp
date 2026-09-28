#include <array>
#include <cstdint>
#include <span>
#include <utility>

#include <mcutest/mcutest.hpp>

#include <mcuhw/backend/ra4m1/sci_uart.hpp>
#include <mcuhw/board/arduino_uno_r4_minima.hpp>
#include <mcuhw/driver/uart.hpp>
#include <mcuhw/execution/fake_policy.hpp>
#include <mcuhw/testing/recording_mmio.hpp>

namespace {

using binding = mcuhw::backend::ra4m1::sci_uart_binding<
    2, mcuhw::board::arduino_uno_r4_minima::d0_rx,
    mcuhw::board::arduino_uno_r4_minima::d1_tx>;
using mmio_type = mcuhw::testing::recording_mmio<0x60000, 256>;
using execution_type = mcuhw::execution::fake_policy;
using backend_type = mcuhw::backend::ra4m1::sci_uart_backend<
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

constexpr auto baud_115200 = mcuhw::backend::ra4m1::solve_uart_baud({
    .clock_hz = 48'000'000U,
    .target_baud = 115'200U,
    .maximum_error_ppm = 2'000U,
});
static_assert(baud_115200.has_value());

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

[[nodiscard]] bool has_read(std::span<const mcuhw::testing::mmio_operation> ops,
                            std::uintptr_t address) {
    for (const auto& operation : ops) {
        if (operation.kind == mcuhw::testing::mmio_operation_kind::read
            && operation.address == address) {
            return true;
        }
    }
    return false;
}

TEST(uart_solver, reports_achieved_baud_and_rejects_bad_requests,
     "REQ-0027") {
    ASSERT_TRUE(baud_115200.has_value());
    EXPECT_EQ(baud_115200->achieved_baud, 115'385U);
    EXPECT_TRUE(baud_115200->error_ppm <= 2'000U);
    EXPECT_TRUE(mcuhw::backend::ra4m1::solve_uart_baud({0U, 115'200U, 10'000U})
                    .error()
                == mcuhw::driver::uart_error::invalid_clock);
    EXPECT_TRUE(mcuhw::backend::ra4m1::solve_uart_baud({48'000'000U, 0U, 10'000U})
                    .error()
                == mcuhw::driver::uart_error::invalid_baud);
    EXPECT_TRUE(mcuhw::backend::ra4m1::solve_uart_baud({48'000'000U, 123'457U, 0U})
                    .error()
                == mcuhw::driver::uart_error::unattainable_tolerance);
}

TEST(ra4m1_uart, configures_sci2_format_baud_and_real_d0_d1_routes,
     "REQ-0027") {
    mmio_type mmio{0x4001'E000U};
    execution_type execution;
    backend_type backend{mmio, execution};
    ASSERT_TRUE(backend.initialize(*baud_115200, {
        .data_bits = mcuhw::driver::uart_data_bits::seven,
        .parity = mcuhw::driver::uart_parity::odd,
        .stop_bits = mcuhw::driver::uart_stop_bits::two,
    }).has_value());
    EXPECT_TRUE(has_write(mmio.operations(), 0x4007'0040U,
                          static_cast<std::uint8_t>(
                              baud_115200->clock_select | 0x78U)));
    EXPECT_TRUE(has_write(mmio.operations(), 0x4007'0041U,
                          baud_115200->brr));
    EXPECT_TRUE(has_write(mmio.operations(), 0x4004'08C4U,
                          (0x04U << 24U) | (1U << 16U)));
    EXPECT_TRUE(has_write(mmio.operations(), 0x4004'08C8U,
                          (0x04U << 24U) | (1U << 16U)));
    ASSERT_TRUE(backend.shutdown().has_value());
}

TEST(ra4m1_uart, initialization_discards_bootloader_receive_state,
     "REQ-0027") {
    mmio_type mmio{0x4001'E000U};
    execution_type execution;
    backend_type backend{mmio, execution};
    ASSERT_TRUE(mmio.write<std::uint8_t>(0x4007'0044U, 0xFCU).has_value());
    ASSERT_TRUE(mmio.write<std::uint8_t>(0x4007'0045U, 0xE0U).has_value());
    mmio.clear_operations();

    ASSERT_TRUE(backend.initialize(*baud_115200, {}).has_value());
    EXPECT_TRUE(has_read(mmio.operations(), 0x4007'0045U));
    EXPECT_TRUE(has_write(mmio.operations(), 0x4007'0044U, 0x84U));
    EXPECT_TRUE(has_write(mmio.operations(), 0x4007'0046U, 0xF2U));
    EXPECT_TRUE(has_write(mmio.operations(), 0x4007'0049U, 0U));
    EXPECT_TRUE(has_write(mmio.operations(), 0x4007'004DU, 0U));
    EXPECT_TRUE(has_write(mmio.operations(), 0x4007'005CU, 0x06U));
    ASSERT_TRUE(backend.shutdown().has_value());
}

TEST(ra4m1_uart, transmit_completes_only_after_final_wire_interrupt,
     "REQ-0017", "REQ-0027") {
    mmio_type mmio{0x4001'E000U};
    execution_type execution;
    backend_type backend{mmio, execution};
    ASSERT_TRUE(backend.initialize(*baud_115200, {}).has_value());
    const std::array<std::uint8_t, 3> data{0x11U, 0x22U, 0x33U};
    auto started = backend.transmit(data);
    ASSERT_TRUE(started.has_value());
    auto transfer = std::move(*started);
    EXPECT_FALSE(transfer.ready());
    EXPECT_TRUE(backend.handle_transmit_data_empty_interrupt());
    EXPECT_TRUE(backend.handle_transmit_data_empty_interrupt());
    EXPECT_TRUE(backend.handle_transmit_data_empty_interrupt());
    EXPECT_FALSE(transfer.ready());
    ASSERT_TRUE(mmio.write<std::uint8_t>(0x4007'0044U, 1U << 2U)
                    .has_value());
    EXPECT_TRUE(backend.handle_transmit_end_interrupt());
    EXPECT_TRUE(transfer.ready());
    EXPECT_TRUE(transfer.take().has_value());
    EXPECT_TRUE(has_write(mmio.operations(), 0x4007'0043U, 0x11U));
    EXPECT_TRUE(has_write(mmio.operations(), 0x4007'0043U, 0x22U));
    EXPECT_TRUE(has_write(mmio.operations(), 0x4007'0043U, 0x33U));
    ASSERT_TRUE(backend.shutdown().has_value());
}

TEST(ra4m1_uart, dma_rx_and_tx_use_sci2_events_and_tx_waits_for_tei,
     "REQ-0017", "REQ-0018", "REQ-0027", "REQ-0030") {
    mmio_type mmio{0x4001'E000U};
    execution_type uart_execution;
    execution_type rx_execution;
    execution_type tx_execution;
    backend_type backend{mmio, uart_execution};
    fake_dma_backend rx_dma{rx_execution};
    fake_dma_backend tx_dma{tx_execution};
    ASSERT_TRUE(backend.initialize(*baud_115200, {}).has_value());
    const std::array<std::uint8_t, 3> tx_data{0x31U, 0x32U, 0x33U};
    std::array<std::uint8_t, 3> rx_data{};
    auto rx_started = backend.receive_dma(rx_data, rx_dma);
    auto tx_started = backend.transmit_dma(tx_data, tx_dma);
    ASSERT_TRUE(rx_started.has_value());
    ASSERT_TRUE(tx_started.has_value());
    auto receive = std::move(*rx_started);
    auto transmit = std::move(*tx_started);
    EXPECT_EQ(rx_dma.last_request.event, 0xA3U);
    EXPECT_EQ(tx_dma.last_request.event, 0xA4U);
    EXPECT_EQ(rx_dma.last_request.peripheral, 0x4007'0045U);
    EXPECT_EQ(tx_dma.last_request.peripheral, 0x4007'0043U);
    EXPECT_TRUE(rx_dma.last_request.direction
                == mcuhw::driver::dma_peripheral_direction::peripheral_to_memory);
    EXPECT_TRUE(tx_dma.last_request.direction
                == mcuhw::driver::dma_peripheral_direction::memory_to_peripheral);

    ASSERT_TRUE(rx_dma.complete());
    EXPECT_TRUE(backend.handle_dma_receive_completion());
    const auto received = receive.take();
    ASSERT_TRUE(received.has_value());
    EXPECT_EQ(*received, rx_data.size());

    ASSERT_TRUE(tx_dma.complete());
    EXPECT_TRUE(backend.handle_dma_transmit_completion());
    EXPECT_FALSE(transmit.ready());
    EXPECT_TRUE(has_write(mmio.operations(), 0x4007'0042U, 0x24U));
    ASSERT_TRUE(mmio.write<std::uint8_t>(0x4007'0044U, 1U << 2U)
                    .has_value());
    EXPECT_TRUE(backend.handle_transmit_end_interrupt());
    EXPECT_TRUE(transmit.take().has_value());
    ASSERT_TRUE(backend.shutdown().has_value());
}

TEST(ra4m1_uart, dma_failure_and_timeout_quiesce_requests,
     "REQ-0019", "REQ-0027", "REQ-0030") {
    mmio_type mmio{0x4001'E000U};
    execution_type uart_execution;
    execution_type rx_execution;
    execution_type tx_execution;
    backend_type backend{mmio, uart_execution};
    fake_dma_backend rx_dma{rx_execution};
    fake_dma_backend tx_dma{tx_execution};
    ASSERT_TRUE(backend.initialize(*baud_115200, {}).has_value());
    std::array<std::uint8_t, 2> rx_data{};
    const std::array<std::uint8_t, 2> tx_data{1U, 2U};
    auto rx_started = backend.receive_dma(rx_data, rx_dma);
    ASSERT_TRUE(rx_started.has_value());
    auto receive = std::move(*rx_started);
    ASSERT_TRUE(rx_dma.fail());
    EXPECT_TRUE(backend.handle_dma_receive_completion());
    const auto failed = receive.take();
    ASSERT_FALSE(failed.has_value());
    EXPECT_TRUE(failed.error().cause == mcuhw::driver::uart_error::dma);

    auto tx_started = backend.transmit_dma(tx_data, tx_dma);
    ASSERT_TRUE(tx_started.has_value());
    auto transmit = std::move(*tx_started);
    const auto waited = transmit.wait_until(mcuhw::execution::deadline{0U});
    ASSERT_FALSE(waited.has_value());
    EXPECT_TRUE(waited.error().kind
                == mcuhw::core::transfer_error_kind::timeout);
    EXPECT_EQ(tx_dma.cancellations, 1U);
    ASSERT_TRUE(backend.shutdown().has_value());
}

TEST(ra4m1_uart, finite_receive_reports_count_and_never_overwrites,
     "REQ-0027") {
    mmio_type mmio{0x4001'E000U};
    execution_type execution;
    backend_type backend{mmio, execution};
    ASSERT_TRUE(backend.initialize(*baud_115200, {}).has_value());
    std::array<std::uint8_t, 2> data{};
    auto started = backend.receive(data);
    ASSERT_TRUE(started.has_value());
    auto transfer = std::move(*started);
    EXPECT_FALSE(backend.handle_receive_interrupt());
    EXPECT_FALSE(transfer.ready());
    ASSERT_TRUE(mmio.write<std::uint8_t>(0x4007'0045U, 0xA5U)
                    .has_value());
    ASSERT_TRUE(mmio.write<std::uint8_t>(0x4007'0044U, 1U << 6U)
                    .has_value());
    EXPECT_TRUE(backend.handle_receive_interrupt());
    ASSERT_TRUE(mmio.write<std::uint8_t>(0x4007'0045U, 0x5AU)
                    .has_value());
    ASSERT_TRUE(mmio.write<std::uint8_t>(0x4007'0044U, 1U << 6U)
                    .has_value());
    EXPECT_TRUE(backend.handle_receive_interrupt());
    ASSERT_TRUE(transfer.ready());
    const auto result = transfer.take();
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, 2U);
    EXPECT_EQ(data[0], 0xA5U);
    EXPECT_EQ(data[1], 0x5AU);
    EXPECT_FALSE(backend.handle_receive_interrupt());
    ASSERT_TRUE(backend.shutdown().has_value());
}

TEST(ra4m1_uart, receive_errors_timeout_and_slot_reuse_are_explicit,
     "REQ-0019", "REQ-0027") {
    mmio_type mmio{0x4001'E000U};
    execution_type execution;
    backend_type backend{mmio, execution};
    ASSERT_TRUE(backend.initialize(*baud_115200, {}).has_value());
    std::array<std::uint8_t, 1> data{};
    auto errored_start = backend.receive(data);
    ASSERT_TRUE(errored_start.has_value());
    auto errored = std::move(*errored_start);
    ASSERT_TRUE(mmio.write<std::uint8_t>(0x4007'0044U, 1U << 5U)
                    .has_value());
    EXPECT_TRUE(backend.handle_receive_error_interrupt());
    const auto error = errored.take();
    ASSERT_FALSE(error.has_value());
    EXPECT_TRUE(error.error().cause == mcuhw::driver::uart_error::overrun);

    auto timed_start = backend.receive(data);
    ASSERT_TRUE(timed_start.has_value());
    auto timed = std::move(*timed_start);
    const auto waited = timed.wait_until(mcuhw::execution::deadline{0U});
    ASSERT_FALSE(waited.has_value());
    EXPECT_TRUE(waited.error().kind == mcuhw::core::transfer_error_kind::timeout);
    EXPECT_TRUE(timed.take().error().kind
                == mcuhw::core::transfer_error_kind::timeout);

    auto reused_start = backend.receive(data);
    ASSERT_TRUE(reused_start.has_value());
    auto reused = std::move(*reused_start);
    ASSERT_TRUE(mmio.write<std::uint8_t>(0x4007'0044U, 1U << 6U)
                    .has_value());
    ASSERT_TRUE(mmio.write<std::uint8_t>(0x4007'0045U, 0x3CU).has_value());
    EXPECT_TRUE(backend.handle_receive_interrupt());
    EXPECT_TRUE(reused.take().has_value());
    ASSERT_TRUE(backend.shutdown().has_value());
}


TEST(ra4m1_uart, polling_keeps_old_tend_separate_from_the_last_fifo_write,
     "REQ-0003", "REQ-0017", "REQ-0027") {
    mmio_type mmio{0x4001'E000U};
    execution_type execution;
    backend_type backend{mmio, execution};
    ASSERT_TRUE(backend.initialize(*baud_115200, {}).has_value());
    const std::array<std::uint8_t, 2U> bytes{0x11U, 0x22U};
    auto started = backend.transmit(bytes);
    ASSERT_TRUE(started.has_value());
    auto transfer = std::move(*started);
    ASSERT_TRUE(mmio.write<std::uint8_t>(0x4007'0044U, 0x84U).has_value());
    backend.poll();
    EXPECT_FALSE(transfer.ready());
    backend.poll();
    EXPECT_FALSE(transfer.ready());
    ASSERT_TRUE(mmio.write<std::uint8_t>(0x4007'0044U, 0x04U).has_value());
    backend.poll();
    EXPECT_TRUE(transfer.ready());
    EXPECT_TRUE(transfer.take().has_value());
    EXPECT_TRUE(backend.shutdown().has_value());
}

} // namespace
