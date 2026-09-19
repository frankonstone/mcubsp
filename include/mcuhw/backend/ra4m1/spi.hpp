#pragma once

#include <cstddef>
#include <cstdint>
#include <expected>
#include <optional>
#include <span>

#include <mcuhw/arch/barrier.hpp>
#include <mcuhw/core/metadata.hpp>
#include <mcuhw/core/transfer.hpp>
#include <mcuhw/driver/spi.hpp>
#include <mcuhw/mcu/ra4m1/description.hpp>

namespace mcuhw::backend::ra4m1 {

namespace spi_detail {

template<std::uint8_t Instance>
struct owner {
    static inline bool claimed{};
};

[[nodiscard]] constexpr mcu::ra4m1::signal miso_signal(
    std::uint8_t instance) noexcept {
    return instance == 0U ? mcu::ra4m1::signal::spi0_miso
                          : mcu::ra4m1::signal::spi1_miso;
}

[[nodiscard]] constexpr mcu::ra4m1::signal mosi_signal(
    std::uint8_t instance) noexcept {
    return instance == 0U ? mcu::ra4m1::signal::spi0_mosi
                          : mcu::ra4m1::signal::spi1_mosi;
}

[[nodiscard]] constexpr mcu::ra4m1::signal clock_signal(
    std::uint8_t instance) noexcept {
    return instance == 0U ? mcu::ra4m1::signal::spi0_rspck
                          : mcu::ra4m1::signal::spi1_rspck;
}

[[nodiscard]] constexpr mcu::ra4m1::signal select_signal(
    std::uint8_t instance) noexcept {
    return instance == 0U ? mcu::ra4m1::signal::spi0_ssl0
                          : mcu::ra4m1::signal::spi1_ssl0;
}

[[nodiscard]] constexpr mcu::ra4m1::instance_record metadata(
    std::uint8_t instance) noexcept {
    for (const auto& candidate : mcu::ra4m1::instances) {
        if (candidate.id.kind
                == static_cast<std::uint16_t>(
                    mcu::ra4m1::peripheral_kind::spi)
            && candidate.id.instance == instance) {
            return candidate;
        }
    }
    return {};
}

} // namespace spi_detail

template<std::uint8_t Instance, core::pin_id MisoPin, core::pin_id MosiPin,
         core::pin_id ClockPin, core::pin_id SelectPin>
struct spi_binding {
    static_assert(Instance < 2U);
    static_assert(core::route_exists(
        mcu::ra4m1::pin_routes, MisoPin,
        mcu::ra4m1::signal_id(spi_detail::miso_signal(Instance))));
    static_assert(core::route_exists(
        mcu::ra4m1::pin_routes, MosiPin,
        mcu::ra4m1::signal_id(spi_detail::mosi_signal(Instance))));
    static_assert(core::route_exists(
        mcu::ra4m1::pin_routes, ClockPin,
        mcu::ra4m1::signal_id(spi_detail::clock_signal(Instance))));
    static_assert(core::route_exists(
        mcu::ra4m1::pin_routes, SelectPin,
        mcu::ra4m1::signal_id(spi_detail::select_signal(Instance))));

    static constexpr std::uint8_t instance = Instance;
    static constexpr core::pin_id miso_pin = MisoPin;
    static constexpr core::pin_id mosi_pin = MosiPin;
    static constexpr core::pin_id clock_pin = ClockPin;
    static constexpr core::pin_id select_pin = SelectPin;
    static constexpr auto metadata = spi_detail::metadata(Instance);
};

// The hardware SSL pin is deliberately driven as GPIO. That makes one
// transaction the ownership boundary and keeps CS asserted across arbitrary
// caller-declared segments. SPII is the only completion point, so the final
// frame has left the wire before CS is released.
template<typename Binding, typename MmioPolicy, typename ExecutionPolicy>
// @req REQ-0014 REQ-0017 REQ-0019 REQ-0028
class spi_master_backend {
public:
    using pool_type = core::operation_pool<driver::spi_transfer_result,
                                           driver::spi_error, 1,
                                           ExecutionPolicy>;
    using transfer_type = typename pool_type::transfer_type;
    using start_result = std::expected<transfer_type,
                                       typename pool_type::error_type>;

    explicit spi_master_backend(MmioPolicy& mmio,
                                ExecutionPolicy& execution) noexcept
        : mmio_(mmio), operations_(execution) {}

    [[nodiscard]] std::expected<void, driver::spi_error>
    initialize(driver::spi_timing timing,
               driver::spi_configuration configuration = {}) noexcept {
        if (initialized_ || spi_detail::owner<Binding::instance>::claimed) {
            return std::unexpected(driver::spi_error::already_owned);
        }
        if (!timing_is_valid(timing)) {
            return std::unexpected(driver::spi_error::invalid_rate);
        }
        spi_detail::owner<Binding::instance>::claimed = true;
        owned_ = true;
        timing_ = timing;
        configuration_ = configuration;
        if (!release_module_stop() || !configure_pins()
            || !program_controller()) {
            static_cast<void>(restore_pins());
            release_owner();
            return mmio_failure();
        }
        initialized_ = true;
        return {};
    }

    [[nodiscard]] start_result
    transaction(std::span<const driver::spi_segment> segments) noexcept {
        if (!initialized_) {
            return start_failure(driver::spi_error::not_initialized);
        }
        if (operation_id_) {
            return start_failure(driver::spi_error::busy);
        }
        const auto validation = validate_segments(segments);
        if (!validation) {
            return start_failure(validation.error());
        }
        auto started = operations_.try_start(
            {.quiesce = &cancel_operation, .context = this});
        if (!started) {
            return std::unexpected(started.error());
        }
        operation_id_ = started->id();
        segments_ = segments;
        total_frames_ = *validation;
        transmitted_frames_ = 0U;
        received_frames_ = 0U;

        if (!clear_errors() || !set_chip_select(true)
            || !write<std::uint8_t>(base() + 0x0FU, spcr2_auto_stop)
            || !write<std::uint8_t>(base() + 0x00U, spcr_active)) {
            static_cast<void>(finish_error(driver::spi_error::mmio));
        }
        return started;
    }

    [[nodiscard]] bool handle_transmit_empty_interrupt() noexcept {
        if (!operation_id_ || transmitted_frames_ >= total_frames_) {
            return false;
        }
        const auto value = transmit_frame(transmitted_frames_);
        const bool written = configuration_.frame_width
                == driver::spi_frame_width::bits8
            ? write<std::uint8_t>(base() + 0x04U,
                                  static_cast<std::uint8_t>(value))
            : write<std::uint16_t>(base() + 0x04U, value);
        if (!written) {
            return finish_error(driver::spi_error::mmio);
        }
        ++transmitted_frames_;
        if (transmitted_frames_ == total_frames_) {
            return write<std::uint8_t>(base() + 0x00U, spcr_receive_only);
        }
        return true;
    }

    [[nodiscard]] bool handle_receive_interrupt() noexcept {
        if (!operation_id_ || received_frames_ >= transmitted_frames_) {
            return false;
        }
        std::uint16_t value{};
        if (configuration_.frame_width == driver::spi_frame_width::bits8) {
            const auto received = mmio_.template read<std::uint8_t>(
                base() + 0x04U);
            if (!received) {
                return finish_error(driver::spi_error::mmio);
            }
            value = *received;
        } else {
            const auto received = mmio_.template read<std::uint16_t>(
                base() + 0x04U);
            if (!received) {
                return finish_error(driver::spi_error::mmio);
            }
            value = *received;
        }
        store_receive_frame(received_frames_, value);
        ++received_frames_;
        if (received_frames_ == total_frames_) {
            return write<std::uint8_t>(base() + 0x0FU,
                                       spcr2_auto_stop | spcr2_idle_irq);
        }
        return true;
    }

    [[nodiscard]] bool handle_idle_interrupt() noexcept {
        if (!operation_id_ || received_frames_ != total_frames_) {
            return false;
        }
        const auto status = mmio_.template read<std::uint8_t>(base() + 0x03U);
        if (!status) {
            return finish_error(driver::spi_error::mmio);
        }
        if ((*status & status_not_idle) != 0U) {
            return false;
        }
        const auto id = *operation_id_;
        const auto frames = total_frames_;
        const bool stopped = write<std::uint8_t>(base() + 0x00U, spcr_stopped)
            && write<std::uint8_t>(base() + 0x0FU, spcr2_auto_stop)
            && set_chip_select(false);
        reset_transaction();
        if (!stopped) {
            return operations_.fail(id, driver::spi_error::mmio);
        }
        return operations_.complete(id, driver::spi_transfer_result{frames});
    }

    [[nodiscard]] bool handle_error_interrupt() noexcept {
        if (!operation_id_) {
            return false;
        }
        const auto status = mmio_.template read<std::uint8_t>(base() + 0x03U);
        if (!status) {
            return finish_error(driver::spi_error::mmio);
        }
        const auto error = (*status & status_overrun) != 0U
            ? driver::spi_error::overrun
            : ((*status & status_mode_fault) != 0U
                   ? driver::spi_error::mode_fault
                   : ((*status & status_parity) != 0U
                          ? driver::spi_error::parity
                          : driver::spi_error::transport));
        static_cast<void>(clear_errors());
        return finish_error(error);
    }

    [[nodiscard]] std::expected<void, driver::spi_error> shutdown() noexcept {
        if (!initialized_) {
            return std::unexpected(driver::spi_error::not_initialized);
        }
        if (operation_id_) {
            return std::unexpected(driver::spi_error::busy);
        }
        if (!write<std::uint8_t>(base() + 0x00U, 0U)
            || !restore_pins()) {
            return mmio_failure();
        }
        initialized_ = false;
        release_owner();
        return {};
    }

private:
    static constexpr std::uint8_t status_overrun = 1U << 0U;
    static constexpr std::uint8_t status_not_idle = 1U << 1U;
    static constexpr std::uint8_t status_mode_fault = 1U << 2U;
    static constexpr std::uint8_t status_parity = 1U << 3U;
    static constexpr std::uint8_t spcr_stopped = 0x09U;
    static constexpr std::uint8_t spcr_active = 0xF9U;
    static constexpr std::uint8_t spcr_receive_only = 0xD9U;
    static constexpr std::uint8_t spcr2_idle_irq = 1U << 2U;
    static constexpr std::uint8_t spcr2_auto_stop = 1U << 4U;

    [[nodiscard]] static constexpr std::uintptr_t base() noexcept {
        return Binding::metadata.base;
    }

    [[nodiscard]] static constexpr bool
    timing_is_valid(driver::spi_timing timing) noexcept {
        if (timing.clock_hz == 0U || timing.target_hz == 0U
            || timing.brdv > 3U) {
            return false;
        }
        const auto divider = 2U * (static_cast<std::uint32_t>(timing.spbr) + 1U)
            * static_cast<std::uint32_t>(1U << timing.brdv);
        return timing.achieved_hz == timing.clock_hz / divider
            && timing.achieved_hz <= timing.target_hz;
    }

    [[nodiscard]] std::expected<std::size_t, driver::spi_error>
    validate_segments(std::span<const driver::spi_segment> segments) const noexcept {
        if (segments.empty()) {
            return std::unexpected(driver::spi_error::empty_transfer);
        }
        std::size_t total{};
        for (const auto& segment : segments) {
            if (segment.transmit.empty() && segment.receive.empty()) {
                return std::unexpected(driver::spi_error::empty_transfer);
            }
            if (!segment.transmit.empty() && !segment.receive.empty()
                && segment.transmit.size() != segment.receive.size()) {
                return std::unexpected(driver::spi_error::mismatched_segment);
            }
            const auto frames = segment.transmit.empty()
                ? segment.receive.size() : segment.transmit.size();
            if (frames > 65'535U - total) {
                return std::unexpected(driver::spi_error::excessive_length);
            }
            if (configuration_.frame_width == driver::spi_frame_width::bits8) {
                if (segment.transmit.empty() && segment.dummy > 0xFFU) {
                    return std::unexpected(driver::spi_error::invalid_frame);
                }
                for (const auto frame : segment.transmit) {
                    if (frame > 0xFFU) {
                        return std::unexpected(driver::spi_error::invalid_frame);
                    }
                }
            }
            total += frames;
        }
        return total;
    }

    [[nodiscard]] std::uint16_t transmit_frame(std::size_t flat) const noexcept {
        for (const auto& segment : segments_) {
            const auto count = segment.transmit.empty()
                ? segment.receive.size() : segment.transmit.size();
            if (flat < count) {
                return segment.transmit.empty() ? segment.dummy
                                                : segment.transmit[flat];
            }
            flat -= count;
        }
        return 0U;
    }

    void store_receive_frame(std::size_t flat, std::uint16_t value) noexcept {
        for (const auto& segment : segments_) {
            const auto count = segment.transmit.empty()
                ? segment.receive.size() : segment.transmit.size();
            if (flat < count) {
                if (!segment.receive.empty()) {
                    segment.receive[flat] = value;
                }
                return;
            }
            flat -= count;
        }
    }

    [[nodiscard]] static core::quiesce_result cancel_operation(
        void* context, core::operation_id id) noexcept {
        auto& self = *static_cast<spi_master_backend*>(context);
        if (!self.operation_id_ || *self.operation_id_ != id) {
            return core::quiesce_result::reusable;
        }
        const bool stopped = self.template write<std::uint8_t>(
                                 base() + 0x00U, spcr_stopped)
            && self.template write<std::uint8_t>(base() + 0x0FU,
                                                 spcr2_auto_stop)
            && self.set_chip_select(false);
        self.reset_transaction();
        return stopped ? core::quiesce_result::reusable
                       : core::quiesce_result::faulted;
    }

    [[nodiscard]] bool finish_error(driver::spi_error error) noexcept {
        if (!operation_id_) {
            return false;
        }
        const auto id = *operation_id_;
        static_cast<void>(write<std::uint8_t>(base() + 0x00U, spcr_stopped));
        static_cast<void>(write<std::uint8_t>(base() + 0x0FU,
                                              spcr2_auto_stop));
        static_cast<void>(set_chip_select(false));
        reset_transaction();
        return operations_.fail(id, error);
    }

    void reset_transaction() noexcept {
        operation_id_.reset();
        segments_ = {};
        total_frames_ = 0U;
        transmitted_frames_ = 0U;
        received_frames_ = 0U;
    }

    [[nodiscard]] bool clear_errors() noexcept {
        const auto status = mmio_.template read<std::uint8_t>(base() + 0x03U);
        return status && write<std::uint8_t>(
            base() + 0x03U,
            static_cast<std::uint8_t>((*status & ~0x1DU) | 0xA0U));
    }

    [[nodiscard]] bool program_controller() noexcept {
        const auto phase = configuration_.mode == driver::spi_mode::mode1
                || configuration_.mode == driver::spi_mode::mode3
            ? 1U : 0U;
        const auto polarity = configuration_.mode == driver::spi_mode::mode2
                || configuration_.mode == driver::spi_mode::mode3
            ? 1U : 0U;
        const auto bit_order = configuration_.bit_order
                == driver::spi_bit_order::least_significant_first
            ? 1U : 0U;
        const auto frame = configuration_.frame_width
                == driver::spi_frame_width::bits8
            ? 0x04U : 0x0FU;
        const auto command = static_cast<std::uint16_t>(
            phase | (polarity << 1U) | (timing_.brdv << 2U)
            | (frame << 8U) | (bit_order << 12U));
        const auto data_control = configuration_.frame_width
                == driver::spi_frame_width::bits8
            ? 0x40U : 0x00U;
        return write<std::uint8_t>(base() + 0x00U, 0U)
            && write<std::uint8_t>(base() + 0x01U, 0U)
            && write<std::uint8_t>(base() + 0x02U, 0U)
            && clear_errors()
            && write<std::uint8_t>(base() + 0x0AU, timing_.spbr)
            && write<std::uint8_t>(base() + 0x0BU, data_control)
            && write<std::uint8_t>(base() + 0x0FU, spcr2_auto_stop)
            && write<std::uint16_t>(base() + 0x10U, command)
            && write<std::uint8_t>(base() + 0x00U, spcr_stopped);
    }

    [[nodiscard]] bool release_module_stop() noexcept {
        constexpr std::uintptr_t prcr = 0x4001'E3FEU;
        const auto current = mmio_.template read<std::uint32_t>(
            Binding::metadata.module_stop_register);
        if (!current || !write<std::uint16_t>(prcr, 0xA502U)) {
            return false;
        }
        const bool enabled = write<std::uint32_t>(
            Binding::metadata.module_stop_register,
            *current & ~Binding::metadata.module_stop_mask);
        const bool locked = write<std::uint16_t>(prcr, 0xA500U);
        arch::data_memory_barrier();
        return enabled && locked;
    }

    [[nodiscard]] bool configure_pins() noexcept {
        return configure_peripheral_pin(Binding::miso_pin,
                                        spi_detail::miso_signal(Binding::instance))
            && configure_peripheral_pin(Binding::mosi_pin,
                                        spi_detail::mosi_signal(Binding::instance))
            && configure_peripheral_pin(Binding::clock_pin,
                                        spi_detail::clock_signal(Binding::instance))
            && configure_select_pin();
    }

    [[nodiscard]] bool restore_pins() noexcept {
        return restore_pin(Binding::miso_pin) && restore_pin(Binding::mosi_pin)
            && restore_pin(Binding::clock_pin) && restore_pin(Binding::select_pin);
    }

    [[nodiscard]] std::uint32_t psel_for(core::pin_id pin,
                                         mcu::ra4m1::signal signal) const noexcept {
        for (const auto& route : mcu::ra4m1::pin_routes) {
            if (route.pin == pin
                && route.signal == mcu::ra4m1::signal_id(signal)) {
                return route.vendor_encoding;
            }
        }
        return 0U;
    }

    [[nodiscard]] bool configure_peripheral_pin(
        core::pin_id pin, mcu::ra4m1::signal signal) noexcept {
        constexpr std::uint32_t controlled =
            (0x1FU << 24U) | (1U << 16U) | (1U << 15U) | (1U << 2U);
        return update_pfs(pin, controlled,
                          (psel_for(pin, signal) << 24U) | (1U << 16U));
    }

    [[nodiscard]] bool configure_select_pin() noexcept {
        if (!set_chip_select(false)) {
            return false;
        }
        constexpr std::uint32_t controlled =
            (0x1FU << 24U) | (1U << 16U) | (1U << 15U) | (1U << 2U)
            | 1U;
        const auto inactive_high = configuration_.chip_select
                == driver::spi_chip_select_polarity::active_low
            ? 1U : 0U;
        return update_pfs(Binding::select_pin, controlled,
                          (1U << 2U) | inactive_high);
    }

    [[nodiscard]] bool restore_pin(core::pin_id pin) noexcept {
        constexpr std::uint32_t controlled =
            (0x1FU << 24U) | (1U << 16U) | (1U << 15U) | (1U << 2U)
            | 1U;
        return update_pfs(pin, controlled, 0U);
    }

    [[nodiscard]] bool update_pfs(core::pin_id pin, std::uint32_t controlled,
                                  std::uint32_t selected) noexcept {
        constexpr std::uintptr_t pwpr = 0x4004'0D03U;
        const auto address = mcu::ra4m1::pfs_address(pin);
        const auto current = mmio_.template read<std::uint32_t>(address);
        if (!current || !write<std::uint8_t>(pwpr, 0U)
            || !write<std::uint8_t>(pwpr, 0x40U)) {
            return false;
        }
        const bool configured = write<std::uint32_t>(
            address, (*current & ~controlled) | selected);
        const bool locked = write<std::uint8_t>(pwpr, 0U)
            && write<std::uint8_t>(pwpr, 0x80U);
        arch::data_memory_barrier();
        return configured && locked;
    }

    [[nodiscard]] bool set_chip_select(bool active) noexcept {
        const auto mask = static_cast<std::uint32_t>(
            1U << Binding::select_pin.index);
        const bool high = active
            ? configuration_.chip_select
                  == driver::spi_chip_select_polarity::active_high
            : configuration_.chip_select
                  == driver::spi_chip_select_polarity::active_low;
        const auto command = high ? mask : static_cast<std::uint32_t>(mask << 16U);
        const bool result = write<std::uint32_t>(
            0x4004'0008U + Binding::select_pin.port * 0x20U, command);
        arch::data_memory_barrier();
        return result;
    }

    template<typename Value>
    [[nodiscard]] bool write(std::uintptr_t address, Value value) noexcept {
        return mmio_.template write<Value>(address, value).has_value();
    }

    [[nodiscard]] static start_result
    start_failure(driver::spi_error error) noexcept {
        return std::unexpected(typename pool_type::error_type{
            .kind = core::transfer_error_kind::failure, .cause = error});
    }

    [[nodiscard]] static std::expected<void, driver::spi_error>
    mmio_failure() noexcept {
        return std::unexpected(driver::spi_error::mmio);
    }

    void release_owner() noexcept {
        if (owned_) {
            spi_detail::owner<Binding::instance>::claimed = false;
            owned_ = false;
        }
    }

    MmioPolicy& mmio_;
    pool_type operations_;
    driver::spi_timing timing_{};
    driver::spi_configuration configuration_{};
    std::span<const driver::spi_segment> segments_{};
    std::optional<core::operation_id> operation_id_{};
    std::size_t total_frames_{};
    std::size_t transmitted_frames_{};
    std::size_t received_frames_{};
    bool owned_{};
    bool initialized_{};
};

} // namespace mcuhw::backend::ra4m1
