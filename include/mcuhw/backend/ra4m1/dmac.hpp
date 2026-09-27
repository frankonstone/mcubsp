#pragma once

#include <cstddef>
#include <cstdint>
#include <expected>
#include <limits>
#include <optional>

#include <mcuhw/arch/barrier.hpp>
#include <mcuhw/core/transfer.hpp>
#include <mcuhw/driver/dma.hpp>
#include <mcuhw/mcu/ra4m1/description.hpp>

namespace mcuhw::backend::ra4m1 {

namespace dmac_detail {

template<std::uint8_t Channel>
struct owner {
    static inline bool claimed{};
};

[[nodiscard]] constexpr mcu::ra4m1::instance_record metadata(
    std::uint8_t channel) noexcept {
    for (const auto& candidate : mcu::ra4m1::instances) {
        if (candidate.id.kind
                == static_cast<std::uint16_t>(
                    mcu::ra4m1::peripheral_kind::dmac)
            && candidate.id.instance == channel) {
            return candidate;
        }
    }
    return {};
}

[[nodiscard]] constexpr const core::dma_reachability* reachability_for(
    std::uint16_t region) noexcept {
    for (const auto& candidate : mcu::ra4m1::dma_reachability) {
        if (candidate.engine == core::dma_engine_kind::ra_dmac
            && candidate.region_id == region) {
            return &candidate;
        }
    }
    return nullptr;
}

} // namespace dmac_detail

// One fixed DMAC channel. This first PLN-0010 slice implements finite normal-
// mode copies activated by software; peripheral event activation is layered on
// the same channel backend without making ordinary IRQ drivers depend on DMA.
template<std::uint8_t Channel, typename MmioPolicy, typename ExecutionPolicy>
    requires(Channel < 4U)
// @req REQ-0013 REQ-0018 REQ-0019 REQ-0030
class dmac_channel_backend {
public:
    using pool_type = core::operation_pool<driver::dma_transfer_result,
                                           driver::dma_error, 1,
                                           ExecutionPolicy>;
    using transfer_type = typename pool_type::transfer_type;
    using start_result = std::expected<transfer_type,
                                       typename pool_type::error_type>;

    explicit dmac_channel_backend(MmioPolicy& mmio,
                                  ExecutionPolicy& execution) noexcept
        : mmio_(mmio), operations_(execution) {}

    [[nodiscard]] std::expected<void, driver::dma_error>
    initialize() noexcept {
        if (initialized_ || dmac_detail::owner<Channel>::claimed) {
            return std::unexpected(driver::dma_error::already_owned);
        }
        dmac_detail::owner<Channel>::claimed = true;
        owned_ = true;
        if (!release_module_stop()
            || !write<std::uint8_t>(base() + 0x1CU, 0U)
            || !write<std::uint8_t>(base() + 0x1DU, 0U)
            || !write<std::uint8_t>(base() + 0x13U, 0U)
            || !write<std::uint8_t>(base() + 0x1EU, 0U)
            || !write<std::uint32_t>(event_link_register(), 0U)) {
            release_owner();
            return mmio_failure();
        }
        initialized_ = true;
        return {};
    }

    [[nodiscard]] start_result
    copy(driver::dma_memory_copy request) noexcept {
        if (!initialized_) {
            return start_failure(driver::dma_error::not_initialized);
        }
        if (operation_id_) {
            return start_failure(driver::dma_error::busy);
        }
        const auto validation = validate(request);
        if (!validation) {
            return start_failure(validation.error());
        }

        auto started = operations_.try_start(
            {.quiesce = &cancel_operation, .context = this});
        if (!started) {
            return std::unexpected(started.error());
        }
        operation_id_ = started->id();
        transfer_count_ = request.count;
        transfer_width_ = request.width;

        const auto size = size_encoding(request.width);
        if (!program_memory_copy(request, size)) {
            static_cast<void>(finish_error(driver::dma_error::mmio));
            return started;
        }
        arch::data_memory_barrier();
        // CLRS keeps issuing software requests until the finite normal-mode
        // counter reaches zero and hardware clears DTE.
        if (!write<std::uint8_t>(base() + 0x1DU, 0x11U)) {
            static_cast<void>(finish_error(driver::dma_error::mmio));
        }
        return started;
    }

    [[nodiscard]] start_result
    peripheral(driver::dma_peripheral_transfer request) noexcept {
        if (!initialized_) {
            return start_failure(driver::dma_error::not_initialized);
        }
        if (operation_id_) {
            return start_failure(driver::dma_error::busy);
        }
        const auto validation = validate(request);
        if (!validation) {
            return start_failure(validation.error());
        }

        auto started = operations_.try_start(
            {.quiesce = &cancel_operation, .context = this});
        if (!started) {
            return std::unexpected(started.error());
        }
        operation_id_ = started->id();
        transfer_count_ = request.count;
        transfer_width_ = request.width;
        if (!program_peripheral_transfer(request, size_encoding(request.width))) {
            static_cast<void>(finish_error(driver::dma_error::mmio));
        }
        return started;
    }

    [[nodiscard]] bool handle_transfer_end_interrupt() noexcept {
        if (!operation_id_) {
            return false;
        }
        const auto status = mmio_.template read<std::uint8_t>(base() + 0x1EU);
        if (!status) {
            return finish_error(driver::dma_error::mmio);
        }
        if ((*status & 0x80U) != 0U) {
            return false;
        }
        if ((*status & 0x01U) != 0U) {
            return finish_error(driver::dma_error::transfer_escape);
        }
        if ((*status & 0x10U) == 0U) {
            return false;
        }
        const auto id = *operation_id_;
        const auto result = driver::dma_transfer_result{
            .units = transfer_count_,
            .width = transfer_width_,
        };
        const bool cleared = write<std::uint32_t>(event_link_register(), 0U)
            && write<std::uint8_t>(base() + 0x1DU, 0U)
            && write<std::uint8_t>(base() + 0x1EU, 0U);
        reset_operation();
        if (!cleared) {
            return operations_.fail(id, driver::dma_error::mmio);
        }
        arch::data_memory_barrier();
        return operations_.complete(id, result);
    }

    // Foreground polling uses the same register dispatch as interrupt entry.
    void poll() noexcept {
        static_assert(Channel < 4U);
        const auto status = mmio_.template read<std::uint8_t>(
            dmac_detail::metadata(Channel).base + 0x1EU);
        if (status && (*status & 0x80U) == 0U
            && (*status & 0x11U) != 0U) {
            static_cast<void>(handle_transfer_end_interrupt());
        }
    }

    [[nodiscard]] std::expected<void, driver::dma_error> shutdown() noexcept {
        if (!initialized_) {
            return std::unexpected(driver::dma_error::not_initialized);
        }
        if (operation_id_) {
            return std::unexpected(driver::dma_error::busy);
        }
        if (!write<std::uint8_t>(base() + 0x1CU, 0U)
            || !write<std::uint8_t>(base() + 0x1DU, 0U)
            || !write<std::uint8_t>(base() + 0x13U, 0U)
            || !write<std::uint8_t>(base() + 0x1EU, 0U)
            || !write<std::uint32_t>(event_link_register(), 0U)) {
            return mmio_failure();
        }
        initialized_ = false;
        release_owner();
        return {};
    }

private:
    static constexpr auto channel_metadata = dmac_detail::metadata(Channel);
    static constexpr std::uint32_t normal_mode_max_count = 65'535U;
    static constexpr std::size_t quiesce_poll_limit = 256U;

    [[nodiscard]] static constexpr std::uintptr_t base() noexcept {
        return channel_metadata.base;
    }

    [[nodiscard]] static constexpr std::uintptr_t
    event_link_register() noexcept {
        return 0x4000'6280U + Channel * 4U;
    }

    [[nodiscard]] static constexpr std::uint16_t size_encoding(
        driver::dma_width width) noexcept {
        switch (width) {
        case driver::dma_width::bits8: return 0U;
        case driver::dma_width::bits16: return 1U;
        case driver::dma_width::bits32: return 2U;
        }
        return 3U;
    }

    [[nodiscard]] static std::expected<void, driver::dma_error>
    validate(driver::dma_memory_copy request) noexcept {
        const auto width = driver::bytes(request.width);
        if (size_encoding(request.width) == 3U) {
            return std::unexpected(driver::dma_error::invalid_width);
        }
        if (request.count == 0U || request.count > normal_mode_max_count
            || request.count
                > std::numeric_limits<std::size_t>::max() / width) {
            return std::unexpected(driver::dma_error::invalid_count);
        }
        if (request.source % width != 0U
            || request.destination % width != 0U) {
            return std::unexpected(driver::dma_error::invalid_alignment);
        }
        const auto byte_count = static_cast<std::size_t>(request.count) * width;
        if (!range_is_reachable(request.source, byte_count, false)
            || !range_is_reachable(request.destination, byte_count, true)) {
            return std::unexpected(driver::dma_error::inaccessible_memory);
        }
        return {};
    }

    [[nodiscard]] static std::expected<void, driver::dma_error>
    validate(driver::dma_peripheral_transfer request) noexcept {
        const auto width = driver::bytes(request.width);
        if (size_encoding(request.width) == 3U) {
            return std::unexpected(driver::dma_error::invalid_width);
        }
        if (request.count == 0U || request.count > normal_mode_max_count
            || request.count
                > std::numeric_limits<std::size_t>::max() / width) {
            return std::unexpected(driver::dma_error::invalid_count);
        }
        if (request.memory % width != 0U
            || request.peripheral % width != 0U) {
            return std::unexpected(driver::dma_error::invalid_alignment);
        }
        if (request.peripheral < 0x4000'0000U
            || request.peripheral > std::numeric_limits<std::uint32_t>::max()) {
            return std::unexpected(driver::dma_error::invalid_peripheral);
        }
        if (!event_is_supported(request.event)) {
            return std::unexpected(driver::dma_error::unsupported_event);
        }
        const auto byte_count = static_cast<std::size_t>(request.count) * width;
        const bool memory_is_destination = request.direction
            == driver::dma_peripheral_direction::peripheral_to_memory;
        if (!range_is_reachable(
                request.memory, byte_count, memory_is_destination)) {
            return std::unexpected(driver::dma_error::inaccessible_memory);
        }
        return {};
    }

    [[nodiscard]] static constexpr bool event_is_supported(
        std::uint16_t event) noexcept {
        for (const auto& request : mcu::ra4m1::dma_requests) {
            if (request.request_id == event
                && request.constraints.engine
                    == core::dma_engine_kind::ra_dmac) {
                return true;
            }
        }
        return false;
    }

    [[nodiscard]] static bool range_is_reachable(
        std::uintptr_t address, std::size_t bytes, bool write_access) noexcept {
        if (address > std::numeric_limits<std::uint32_t>::max()) {
            return false;
        }
        for (std::size_t index = 0U;
             index < mcu::ra4m1::memory_regions.size(); ++index) {
            const auto& region = mcu::ra4m1::memory_regions[index];
            if (address < region.begin || bytes > region.size
                || address - region.begin > region.size - bytes) {
                continue;
            }
            const auto* reachability =
                dmac_detail::reachability_for(region.id);
            if (reachability == nullptr) {
                return false;
            }
            return write_access
                ? region.writable && reachability->can_write
                : region.readable && reachability->can_read;
        }
        return false;
    }

    [[nodiscard]] bool program_memory_copy(
        driver::dma_memory_copy request, std::uint16_t size) noexcept {
        constexpr std::uint16_t increment_both =
            (2U << 14U) | (2U << 6U);
        if (!write<std::uint8_t>(base() + 0x1CU, 0U)) return false;
        if (!write<std::uint8_t>(base() + 0x1DU, 0U)) return false;
        if (!write<std::uint8_t>(base() + 0x1EU, 0U)) return false;
        if (!write<std::uint32_t>(
                base() + 0x00U,
                static_cast<std::uint32_t>(request.source))) {
            return false;
        }
        if (!write<std::uint32_t>(
                base() + 0x04U,
                static_cast<std::uint32_t>(request.destination))) {
            return false;
        }
        if (!write<std::uint32_t>(base() + 0x08U, request.count)) return false;
        if (!write<std::uint16_t>(base() + 0x0CU, 0U)) return false;
        if (!write<std::uint16_t>(
                base() + 0x10U, static_cast<std::uint16_t>(size << 8U))) {
            return false;
        }
        if (!write<std::uint8_t>(base() + 0x13U, 1U << 4U)) return false;
        if (!write<std::uint16_t>(base() + 0x14U, increment_both)) return false;
        if (!write<std::uint32_t>(base() + 0x18U, 0U)) return false;
        if (!write<std::uint8_t>(base() + 0x1CU, 1U)) return false;
        return write<std::uint8_t>(0x4000'5200U, 1U);
    }

    [[nodiscard]] bool program_peripheral_transfer(
        driver::dma_peripheral_transfer request,
        std::uint16_t size) noexcept {
        const bool memory_to_peripheral = request.direction
            == driver::dma_peripheral_direction::memory_to_peripheral;
        const auto source = memory_to_peripheral ? request.memory
                                                 : request.peripheral;
        const auto destination = memory_to_peripheral ? request.peripheral
                                                      : request.memory;
        constexpr std::uint16_t source_increment = 2U << 14U;
        constexpr std::uint16_t destination_increment = 2U << 6U;
        const auto address_mode = memory_to_peripheral
            ? source_increment : destination_increment;
        if (!write<std::uint32_t>(event_link_register(), 0U)) return false;
        if (!write<std::uint8_t>(base() + 0x1CU, 0U)) return false;
        if (!write<std::uint8_t>(base() + 0x1DU, 0U)) return false;
        if (!write<std::uint8_t>(base() + 0x1EU, 0U)) return false;
        if (!write<std::uint32_t>(base() + 0x00U,
                                  static_cast<std::uint32_t>(source))) {
            return false;
        }
        if (!write<std::uint32_t>(base() + 0x04U,
                                  static_cast<std::uint32_t>(destination))) {
            return false;
        }
        if (!write<std::uint32_t>(base() + 0x08U, request.count)) return false;
        if (!write<std::uint16_t>(base() + 0x0CU, 0U)) return false;
        if (!write<std::uint16_t>(
                base() + 0x10U,
                static_cast<std::uint16_t>((size << 8U) | 1U))) {
            return false;
        }
        if (!write<std::uint8_t>(base() + 0x13U, 1U << 4U)) return false;
        if (!write<std::uint16_t>(base() + 0x14U, address_mode)) return false;
        if (!write<std::uint32_t>(base() + 0x18U, 0U)) return false;
        if (!write<std::uint32_t>(event_link_register(), request.event)) {
            return false;
        }
        if (!write<std::uint8_t>(base() + 0x1CU, 1U)) return false;
        return write<std::uint8_t>(0x4000'5200U, 1U);
    }

    [[nodiscard]] static core::quiesce_result cancel_operation(
        void* context, core::operation_id id) noexcept {
        auto& self = *static_cast<dmac_channel_backend*>(context);
        if (!self.operation_id_ || *self.operation_id_ != id) {
            return core::quiesce_result::reusable;
        }
        bool stopped = self.template write<std::uint32_t>(
                           event_link_register(), 0U)
            && self.template write<std::uint8_t>(base() + 0x1CU, 0U)
            && self.template write<std::uint8_t>(base() + 0x1DU, 0U);
        arch::data_memory_barrier();
        bool idle = false;
        if (stopped) {
            for (std::size_t attempt = 0U; attempt < quiesce_poll_limit;
                 ++attempt) {
                const auto status = self.mmio_.template read<std::uint8_t>(
                    base() + 0x1EU);
                if (!status) {
                    stopped = false;
                    break;
                }
                if ((*status & 0x80U) == 0U) {
                    idle = true;
                    break;
                }
            }
        }
        stopped = stopped && idle
            && self.template write<std::uint8_t>(base() + 0x1EU, 0U);
        self.reset_operation();
        return stopped ? core::quiesce_result::reusable
                       : core::quiesce_result::faulted;
    }

    [[nodiscard]] bool finish_error(driver::dma_error error) noexcept {
        if (!operation_id_) {
            return false;
        }
        const auto id = *operation_id_;
        static_cast<void>(write<std::uint32_t>(event_link_register(), 0U));
        static_cast<void>(write<std::uint8_t>(base() + 0x1CU, 0U));
        static_cast<void>(write<std::uint8_t>(base() + 0x1DU, 0U));
        static_cast<void>(write<std::uint8_t>(base() + 0x1EU, 0U));
        reset_operation();
        return operations_.fail(id, error);
    }

    void reset_operation() noexcept {
        operation_id_.reset();
        transfer_count_ = 0U;
        transfer_width_ = driver::dma_width::bits8;
    }

    [[nodiscard]] bool release_module_stop() noexcept {
        constexpr std::uintptr_t prcr = 0x4001'E3FEU;
        const auto current = mmio_.template read<std::uint32_t>(
            channel_metadata.module_stop_register);
        if (!current || !write<std::uint16_t>(prcr, 0xA502U)) {
            return false;
        }
        const bool enabled = write<std::uint32_t>(
            channel_metadata.module_stop_register,
            *current & ~channel_metadata.module_stop_mask);
        const bool locked = write<std::uint16_t>(prcr, 0xA500U);
        arch::data_memory_barrier();
        return enabled && locked;
    }

    template<typename Value>
    [[nodiscard]] bool write(std::uintptr_t address, Value value) noexcept {
        return mmio_.template write<Value>(address, value).has_value();
    }

    [[nodiscard]] static start_result
    start_failure(driver::dma_error error) noexcept {
        return std::unexpected(typename pool_type::error_type{
            .kind = core::transfer_error_kind::failure, .cause = error});
    }

    [[nodiscard]] static std::expected<void, driver::dma_error>
    mmio_failure() noexcept {
        return std::unexpected(driver::dma_error::mmio);
    }

    void release_owner() noexcept {
        if (owned_) {
            dmac_detail::owner<Channel>::claimed = false;
            owned_ = false;
        }
    }

    MmioPolicy& mmio_;
    pool_type operations_;
    std::uint32_t transfer_count_{};
    driver::dma_width transfer_width_{driver::dma_width::bits8};
    std::optional<core::operation_id> operation_id_{};
    bool owned_{};
    bool initialized_{};
};

} // namespace mcuhw::backend::ra4m1
