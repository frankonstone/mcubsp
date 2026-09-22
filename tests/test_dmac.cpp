#include <array>
#include <cstdint>
#include <span>
#include <utility>

#include <mcutest/mcutest.hpp>

#include <mcuhw/backend/ra4m1/dmac.hpp>
#include <mcuhw/execution/fake_policy.hpp>
#include <mcuhw/testing/recording_mmio.hpp>

namespace {

using mmio_type = mcuhw::testing::recording_mmio<0x20000, 1024>;
using execution_type = mcuhw::execution::fake_policy;
using backend_type = mcuhw::backend::ra4m1::dmac_channel_backend<
    0, mmio_type, execution_type>;

constexpr std::uintptr_t channel_base = 0x4000'5000U;

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

void seed_module_stop(mmio_type& mmio) {
    ASSERT_TRUE(mmio.write<std::uint32_t>(0x4001'E01CU,
                                          0xFFFF'FFFFU).has_value());
    mmio.clear_operations();
}

[[nodiscard]] mcuhw::driver::dma_memory_copy valid_copy() {
    return {
        .source = 0x2000'0100U,
        .destination = 0x2000'0200U,
        .count = 16U,
        .width = mcuhw::driver::dma_width::bits32,
    };
}

TEST(ra4m1_dmac, programs_finite_software_copy_and_completes_from_dtif,
     "REQ-0013", "REQ-0018", "REQ-0030") {
    mmio_type mmio{0x4000'0000U};
    seed_module_stop(mmio);
    execution_type execution;
    backend_type backend{mmio, execution};
    ASSERT_TRUE(backend.initialize().has_value());

    auto started = backend.copy(valid_copy());
    ASSERT_TRUE(started.has_value());
    auto transfer = std::move(*started);
    EXPECT_TRUE(has_write(mmio.operations(), channel_base + 0x00U,
                          0x2000'0100U));
    EXPECT_TRUE(has_write(mmio.operations(), channel_base + 0x04U,
                          0x2000'0200U));
    EXPECT_TRUE(has_write(mmio.operations(), channel_base + 0x08U, 16U));
    EXPECT_TRUE(has_write(mmio.operations(), channel_base + 0x10U, 0x0200U));
    EXPECT_TRUE(has_write(mmio.operations(), channel_base + 0x14U, 0x8080U));
    EXPECT_TRUE(has_write(mmio.operations(), channel_base + 0x13U, 0x10U));
    EXPECT_TRUE(has_write(mmio.operations(), channel_base + 0x1CU, 1U));
    EXPECT_TRUE(has_write(mmio.operations(), 0x4000'5200U, 1U));
    EXPECT_TRUE(has_write(mmio.operations(), channel_base + 0x1DU, 0x11U));

    ASSERT_TRUE(mmio.write<std::uint8_t>(channel_base + 0x1CU, 0U).has_value());
    ASSERT_TRUE(mmio.write<std::uint8_t>(channel_base + 0x1EU, 0x10U).has_value());
    EXPECT_TRUE(backend.handle_transfer_end_interrupt());
    const auto result = transfer.take();
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->units, 16U);
    EXPECT_TRUE(result->width == mcuhw::driver::dma_width::bits32);
    EXPECT_EQ(execution.notification_count(), 1U);
    ASSERT_TRUE(backend.shutdown().has_value());
}

TEST(ra4m1_dmac, programs_spi_event_transfers_and_disconnects_event_on_completion,
     "REQ-0013", "REQ-0018", "REQ-0028", "REQ-0030") {
    mmio_type mmio{0x4000'0000U};
    seed_module_stop(mmio);
    execution_type execution;
    backend_type backend{mmio, execution};
    ASSERT_TRUE(backend.initialize().has_value());

    auto started = backend.peripheral({
        .memory = 0x2000'0100U,
        .peripheral = 0x4007'2104U,
        .count = 4U,
        .width = mcuhw::driver::dma_width::bits8,
        .event = mcuhw::mcu::ra4m1::event_number(
            mcuhw::mcu::ra4m1::event::spi1_rxi),
        .direction = mcuhw::driver::dma_peripheral_direction::peripheral_to_memory,
    });
    ASSERT_TRUE(started.has_value());
    auto transfer = std::move(*started);
    EXPECT_TRUE(has_write(mmio.operations(), 0x4000'6280U, 0xB2U));
    EXPECT_TRUE(has_write(mmio.operations(), channel_base + 0x00U,
                          0x4007'2104U));
    EXPECT_TRUE(has_write(mmio.operations(), channel_base + 0x04U,
                          0x2000'0100U));
    EXPECT_TRUE(has_write(mmio.operations(), channel_base + 0x10U, 0x0001U));
    EXPECT_TRUE(has_write(mmio.operations(), channel_base + 0x14U, 0x0080U));
    ASSERT_TRUE(mmio.write<std::uint8_t>(channel_base + 0x1EU, 0x10U).has_value());
    EXPECT_TRUE(backend.handle_transfer_end_interrupt());
    const auto result = transfer.take();
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->units, 4U);
    EXPECT_TRUE(has_write(mmio.operations(), 0x4000'6280U, 0U));
    ASSERT_TRUE(backend.shutdown().has_value());
}

TEST(ra4m1_dmac, validates_peripheral_event_address_and_memory_direction,
     "REQ-0013", "REQ-0019", "REQ-0030") {
    mmio_type mmio{0x4000'0000U};
    seed_module_stop(mmio);
    execution_type execution;
    backend_type backend{mmio, execution};
    ASSERT_TRUE(backend.initialize().has_value());
    mcuhw::driver::dma_peripheral_transfer request{
        .memory = 0x2000'0100U,
        .peripheral = 0x4007'2104U,
        .count = 4U,
        .width = mcuhw::driver::dma_width::bits8,
        .event = mcuhw::mcu::ra4m1::event_number(
            mcuhw::mcu::ra4m1::event::spi1_txi),
        .direction = mcuhw::driver::dma_peripheral_direction::memory_to_peripheral,
    };
    request.event = mcuhw::mcu::ra4m1::event_number(
        mcuhw::mcu::ra4m1::event::spi1_idle);
    EXPECT_TRUE(backend.peripheral(request).error().cause
                == mcuhw::driver::dma_error::unsupported_event);
    request.event = mcuhw::mcu::ra4m1::event_number(
        mcuhw::mcu::ra4m1::event::spi1_txi);
    request.peripheral = 0x2000'0000U;
    EXPECT_TRUE(backend.peripheral(request).error().cause
                == mcuhw::driver::dma_error::invalid_peripheral);
    request.peripheral = 0x4007'2104U;
    request.memory = 0x0000'4000U;
    request.direction =
        mcuhw::driver::dma_peripheral_direction::peripheral_to_memory;
    EXPECT_TRUE(backend.peripheral(request).error().cause
                == mcuhw::driver::dma_error::inaccessible_memory);
    ASSERT_TRUE(backend.shutdown().has_value());
}

TEST(ra4m1_dmac, owns_each_hardware_channel_exclusively_until_shutdown,
     "REQ-0013", "REQ-0030") {
    mmio_type first_mmio{0x4000'0000U};
    mmio_type second_mmio{0x4000'0000U};
    seed_module_stop(first_mmio);
    seed_module_stop(second_mmio);
    execution_type first_execution;
    execution_type second_execution;
    backend_type first{first_mmio, first_execution};
    backend_type second{second_mmio, second_execution};

    ASSERT_TRUE(first.initialize().has_value());
    const auto conflicted = second.initialize();
    ASSERT_FALSE(conflicted.has_value());
    EXPECT_TRUE(conflicted.error() == mcuhw::driver::dma_error::already_owned);
    ASSERT_TRUE(first.shutdown().has_value());
    ASSERT_TRUE(second.initialize().has_value());
    ASSERT_TRUE(second.shutdown().has_value());
}

TEST(ra4m1_dmac, accepts_flash_source_and_rejects_invalid_ranges_and_shapes,
     "REQ-0013", "REQ-0030") {
    mmio_type mmio{0x4000'0000U};
    seed_module_stop(mmio);
    execution_type execution;
    backend_type backend{mmio, execution};
    ASSERT_TRUE(backend.initialize().has_value());

    auto bad = valid_copy();
    bad.count = 0U;
    EXPECT_TRUE(backend.copy(bad).error().cause
                == mcuhw::driver::dma_error::invalid_count);
    bad = valid_copy();
    bad.count = 65'536U;
    EXPECT_TRUE(backend.copy(bad).error().cause
                == mcuhw::driver::dma_error::invalid_count);
    bad = valid_copy();
    bad.width = static_cast<mcuhw::driver::dma_width>(3U);
    EXPECT_TRUE(backend.copy(bad).error().cause
                == mcuhw::driver::dma_error::invalid_width);
    bad = valid_copy();
    bad.source += 1U;
    EXPECT_TRUE(backend.copy(bad).error().cause
                == mcuhw::driver::dma_error::invalid_alignment);
    bad = valid_copy();
    bad.destination = 0x0000'1000U;
    EXPECT_TRUE(backend.copy(bad).error().cause
                == mcuhw::driver::dma_error::inaccessible_memory);
    bad = valid_copy();
    bad.source = 0x2000'7FF0U;
    EXPECT_TRUE(backend.copy(bad).error().cause
                == mcuhw::driver::dma_error::inaccessible_memory);

    auto flash = valid_copy();
    flash.source = 0x0000'4000U;
    auto accepted = backend.copy(flash);
    ASSERT_TRUE(accepted.has_value());
    auto transfer = std::move(*accepted);
    ASSERT_TRUE(transfer.cancel().has_value());
    EXPECT_TRUE(transfer.take().error().kind
                == mcuhw::core::transfer_error_kind::cancelled);
    ASSERT_TRUE(backend.shutdown().has_value());
}

TEST(ra4m1_dmac, cancellation_quiesces_and_late_interrupt_does_not_complete,
     "REQ-0018", "REQ-0019", "REQ-0030") {
    mmio_type mmio{0x4000'0000U};
    seed_module_stop(mmio);
    execution_type execution;
    backend_type backend{mmio, execution};
    ASSERT_TRUE(backend.initialize().has_value());
    auto started = backend.copy(valid_copy());
    ASSERT_TRUE(started.has_value());
    auto transfer = std::move(*started);
    const auto cancelled = transfer.cancel();
    ASSERT_TRUE(cancelled.has_value());
    EXPECT_TRUE(*cancelled);
    EXPECT_TRUE(transfer.take().error().kind
                == mcuhw::core::transfer_error_kind::cancelled);

    ASSERT_TRUE(mmio.write<std::uint8_t>(channel_base + 0x1EU, 0x10U).has_value());
    EXPECT_FALSE(backend.handle_transfer_end_interrupt());

    auto restarted = backend.copy(valid_copy());
    ASSERT_TRUE(restarted.has_value());
    auto second = std::move(*restarted);
    EXPECT_FALSE(backend.handle_transfer_end_interrupt());
    ASSERT_TRUE(mmio.write<std::uint8_t>(channel_base + 0x1EU, 0x10U).has_value());
    EXPECT_TRUE(backend.handle_transfer_end_interrupt());
    EXPECT_TRUE(second.take().has_value());
    ASSERT_TRUE(backend.shutdown().has_value());
}

TEST(ra4m1_dmac, timeout_with_stuck_active_flag_faults_and_retires_channel,
     "REQ-0018", "REQ-0019", "REQ-0030") {
    mmio_type mmio{0x4000'0000U};
    seed_module_stop(mmio);
    execution_type execution;
    backend_type backend{mmio, execution};
    ASSERT_TRUE(backend.initialize().has_value());
    auto started = backend.copy(valid_copy());
    ASSERT_TRUE(started.has_value());
    auto transfer = std::move(*started);
    ASSERT_TRUE(mmio.write<std::uint8_t>(channel_base + 0x1EU, 0x80U).has_value());
    const auto waited = transfer.wait_until(mcuhw::execution::deadline{0U});
    ASSERT_FALSE(waited.has_value());
    EXPECT_TRUE(waited.error().kind
                == mcuhw::core::transfer_error_kind::quiesce_failed);
    EXPECT_TRUE(transfer.take().error().kind
                == mcuhw::core::transfer_error_kind::quiesce_failed);
    const auto unavailable = backend.copy(valid_copy());
    ASSERT_FALSE(unavailable.has_value());
    EXPECT_TRUE(unavailable.error().kind
                == mcuhw::core::transfer_error_kind::resource_exhausted);
    ASSERT_TRUE(mmio.write<std::uint8_t>(channel_base + 0x1EU, 0U).has_value());
    ASSERT_TRUE(backend.shutdown().has_value());
}

TEST(ra4m1_dmac, escape_interrupt_is_typed_failure_and_channel_is_reusable,
     "REQ-0019", "REQ-0030") {
    mmio_type mmio{0x4000'0000U};
    seed_module_stop(mmio);
    execution_type execution;
    backend_type backend{mmio, execution};
    ASSERT_TRUE(backend.initialize().has_value());
    auto started = backend.copy(valid_copy());
    ASSERT_TRUE(started.has_value());
    auto transfer = std::move(*started);
    ASSERT_TRUE(mmio.write<std::uint8_t>(channel_base + 0x1EU, 0x01U).has_value());
    EXPECT_TRUE(backend.handle_transfer_end_interrupt());
    const auto failed = transfer.take();
    ASSERT_FALSE(failed.has_value());
    EXPECT_TRUE(failed.error().cause
                == mcuhw::driver::dma_error::transfer_escape);

    auto restarted = backend.copy(valid_copy());
    ASSERT_TRUE(restarted.has_value());
    auto second = std::move(*restarted);
    ASSERT_TRUE(second.cancel().has_value());
    static_cast<void>(second.take());
    ASSERT_TRUE(backend.shutdown().has_value());
}

} // namespace
