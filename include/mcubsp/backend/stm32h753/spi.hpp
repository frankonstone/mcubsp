#pragma once

#include <cstddef>
#include <cstdint>
#include <expected>
#include <optional>
#include <span>

#include <mcubsp/backend/stm32h753/detail.hpp>
#include <mcubsp/core/transfer.hpp>
#include <mcubsp/driver/spi.hpp>

namespace mcubsp::backend::stm32h753 {

struct spi_timing {
    std::uint32_t clock_hz;
    std::uint32_t target_hz;
    std::uint32_t achieved_hz;
    std::uint32_t error_ppm;

    std::uint32_t prescaler_encoding;
    [[nodiscard]] constexpr operator driver::spi_timing() const noexcept {
        return {clock_hz, target_hz, achieved_hz, error_ppm};
    }
    friend constexpr bool operator==(spi_timing, spi_timing) = default;
};

namespace detail {
template<std::uint8_t Instance>
struct spi_owner { static inline bool claimed{}; };

template<bool Enabled>
struct spi_select_handler {};

template<>
struct spi_select_handler<true> {
    bool (*set)(void*, bool) noexcept{};
    void* context{};
};
} // namespace detail

// STM32 SPI MBR offers power-of-two divisors from 2 through 256.
// @req REQ-0011 REQ-0028 REQ-0033
[[nodiscard]] constexpr std::expected<spi_timing, driver::spi_error>
solve_stm32_spi_timing(driver::spi_timing_request request) noexcept {
    if (request.clock_hz == 0U) return std::unexpected(driver::spi_error::invalid_clock);
    if (request.target_hz == 0U || request.target_hz > request.clock_hz / 2U) {
        return std::unexpected(driver::spi_error::invalid_rate);
    }
    for (std::uint8_t encoding = 0U; encoding < 8U; ++encoding) {
        const auto divider = static_cast<std::uint32_t>(2U << encoding);
        const auto achieved = request.clock_hz / divider;
        if (achieved > request.target_hz) continue;
        const auto error = static_cast<std::uint32_t>(
            (static_cast<std::uint64_t>(request.target_hz - achieved)
                 * 1'000'000ULL + request.target_hz / 2U)
            / request.target_hz);
        if (error > request.maximum_error_ppm) {
            return std::unexpected(driver::spi_error::unattainable_tolerance);
        }
        return spi_timing{
            .clock_hz = request.clock_hz,
            .target_hz = request.target_hz,
            .achieved_hz = achieved,
            .error_ppm = error,
            .prescaler_encoding = encoding,
        };
    }
    return std::unexpected(driver::spi_error::unattainable_tolerance);
}

template<std::uint8_t Instance, core::pin_id MisoPin, core::pin_id MosiPin,
         core::pin_id ClockPin, core::pin_id SelectPin>
struct spi_binding {
    static_assert(Instance == 1U,
                  "source-backed initial Nucleo route covers SPI1");
    static_assert(core::route_exists(mcu::stm32h753::pin_routes, MisoPin,
        mcu::stm32h753::signal_id(mcu::stm32h753::signal::spi1_miso)));
    static_assert(core::route_exists(mcu::stm32h753::pin_routes, MosiPin,
        mcu::stm32h753::signal_id(mcu::stm32h753::signal::spi1_mosi)));
    static_assert(core::route_exists(mcu::stm32h753::pin_routes, ClockPin,
        mcu::stm32h753::signal_id(mcu::stm32h753::signal::spi1_sck)));
    static_assert(mcu::stm32h753::package_contains(
        mcu::stm32h753::package_variant::lqfp144, SelectPin,
        mcu::stm32h753::package_memberships));
    static constexpr std::uint8_t instance = Instance;
    static constexpr core::pin_id miso_pin = MisoPin;
    static constexpr core::pin_id mosi_pin = MosiPin;
    static constexpr core::pin_id clock_pin = ClockPin;
    static constexpr core::pin_id select_pin = SelectPin;
    static constexpr auto metadata = detail::instance(
        mcu::stm32h753::peripheral_kind::spi, Instance);
    static constexpr auto af = detail::alternate_function(
        ClockPin, mcu::stm32h753::signal::spi1_sck);
};

// Software-controlled CS stays asserted over every caller segment. EOT is the
// sole success publication point, after the final frame has left the wire.
template<typename Binding, typename MmioPolicy, typename ExecutionPolicy,
         bool ExternalSelect = false>
// @req REQ-0003 REQ-0009 REQ-0014 REQ-0017 REQ-0019 REQ-0028 REQ-0033
class spi_master_backend {
public:
    using pool_type = core::operation_pool<driver::spi_transfer_result,
                                           driver::spi_error, 1,
                                           ExecutionPolicy>;
    using transfer_type = typename pool_type::transfer_type;
    using start_result = std::expected<transfer_type,
                                       typename pool_type::error_type>;

    spi_master_backend(MmioPolicy& mmio, ExecutionPolicy& execution) noexcept
        : mmio_(mmio), operations_(execution) {}

    [[nodiscard]] std::expected<void, driver::spi_error> initialize(
        spi_timing timing,
        driver::spi_configuration configuration = {}) noexcept {
        if (initialized_ || detail::spi_owner<Binding::instance>::claimed) {
            return std::unexpected(driver::spi_error::already_owned);
        }
        if (!timing_is_valid(timing)) {
            return std::unexpected(driver::spi_error::invalid_rate);
        }
        if (!configuration_is_valid(configuration)) {
            return std::unexpected(driver::spi_error::invalid_frame);
        }
        detail::spi_owner<Binding::instance>::claimed = true; owned_ = true;
        timing_ = timing; configuration_ = configuration;
        detail::pinmux<MmioPolicy> pins{mmio_};
        const bool inactive = configuration.chip_select
                == driver::spi_chip_select_polarity::active_low;
        const auto miso_af = detail::alternate_function(
            Binding::miso_pin, mcu::stm32h753::signal::spi1_miso);
        const auto mosi_af = detail::alternate_function(
            Binding::mosi_pin, mcu::stm32h753::signal::spi1_mosi);
        if (!detail::enable_peripheral_clock(mmio_, Binding::metadata)
            || !pins.alternate(Binding::clock_pin, Binding::af)
            || !pins.alternate(Binding::miso_pin, miso_af)
            || !pins.alternate(Binding::mosi_pin, mosi_af)
            || !configure_select_pin(pins, inactive)
            || !program()) {
            release_owner(); return mmio_failure();
        }
        initialized_ = true; configuration_valid_ = true; return {};
    }

    [[nodiscard]] bool busy() const noexcept { return id_.has_value(); }

    [[nodiscard]] std::expected<void, driver::spi_error>
    set_select_handler(bool (*set)(void*, bool) noexcept,
                       void* context) noexcept requires(ExternalSelect) {
        if (busy()) return std::unexpected(driver::spi_error::busy);
        select_handler_ = {set, context};
        return {};
    }

    [[nodiscard]] std::expected<void, driver::spi_error> reconfigure(
        spi_timing timing,
        driver::spi_configuration configuration = {}) noexcept {
        if (!initialized_) return std::unexpected(driver::spi_error::not_initialized);
        if (busy()) return std::unexpected(driver::spi_error::busy);
        if (!timing_is_valid(timing)) return std::unexpected(driver::spi_error::invalid_rate);
        if (!configuration_is_valid(configuration)) return std::unexpected(driver::spi_error::invalid_frame);
        timing_ = timing;
        configuration_ = configuration;
        configuration_valid_ = program();
        if (!configuration_valid_) return mmio_failure();
        return {};
    }

    [[nodiscard]] start_result transaction(
        std::span<const driver::spi_segment> segments) noexcept {
        if (!initialized_) return failure(driver::spi_error::not_initialized);
        if (!configuration_valid_) return failure(driver::spi_error::mmio);
        if (id_) return failure(driver::spi_error::busy);
        const auto count = validate(segments);
        if (!count) return failure(count.error());
        auto started = operations_.try_start({.quiesce = &cancel, .context = this});
        if (!started) return std::unexpected(started.error());
        id_ = started->id(); segments_ = segments; total_ = *count;
        transmitted_ = 0U; received_ = 0U;
        if (!write(0x18U, 0x0FFFU) || !write(0x04U, static_cast<std::uint32_t>(total_))
            || !set_select(true) || !write(0x10U, interrupt_mask)
            || !write(0x00U, (1U << 12U) | 1U)
            || !write(0x00U, (1U << 12U) | (1U << 9U) | 1U)) {
            (void)finish(driver::spi_error::mmio);
        }
        return started;
    }

    [[nodiscard]] bool handle_interrupt() noexcept {
        if (!id_) return false;
        const auto status = read(0x14U);
        if (!status) return finish(driver::spi_error::mmio);
        if ((*status & (1U << 6U)) != 0U) return finish(driver::spi_error::overrun);
        if ((*status & (1U << 9U)) != 0U) return finish(driver::spi_error::mode_fault);
        if ((*status & 1U) != 0U && received_ < transmitted_) {
            const auto value = read_data();
            if (!value) return finish(driver::spi_error::mmio);
            store(received_++, static_cast<std::uint16_t>(*value));
            return true;
        }
        if ((*status & (1U << 1U)) != 0U && transmitted_ < total_) {
            if (!write_data(frame(transmitted_++))) {
                return finish(driver::spi_error::mmio);
            }
            return true;
        }
        if ((*status & (1U << 3U)) != 0U && received_ == total_) {
            const auto current = *id_; const auto frames = total_;
            const bool interrupts_stopped = write(0x10U, 0U);
            const bool cleared = write(0x18U, 1U << 3U);
            const bool controller_stopped = write(0x00U, 1U << 12U);
            const bool deselected = set_select(false);
            const bool stopped = interrupts_stopped && cleared
                && controller_stopped && deselected;
            reset();
            return stopped
                ? operations_.complete(current, driver::spi_transfer_result{frames})
                : operations_.fail(current, driver::spi_error::mmio);
        }
        return false;
    }
    [[nodiscard]] bool handle_transmit_empty_interrupt() noexcept { return handle_interrupt(); }
    [[nodiscard]] bool handle_receive_interrupt() noexcept { return handle_interrupt(); }
    [[nodiscard]] bool handle_idle_interrupt() noexcept { return handle_interrupt(); }
    [[nodiscard]] bool handle_error_interrupt() noexcept { return handle_interrupt(); }

    [[nodiscard]] std::expected<void, driver::spi_error> shutdown() noexcept {
        if (!initialized_) return std::unexpected(driver::spi_error::not_initialized);
        if (id_) return std::unexpected(driver::spi_error::busy);
        detail::pinmux<MmioPolicy> pins{mmio_};
        if (!write(0x00U, 0U) || !pins.restore(Binding::clock_pin)
            || !pins.restore(Binding::miso_pin) || !pins.restore(Binding::mosi_pin)
            || !configure_select_pin(pins,
                configuration_.chip_select == driver::spi_chip_select_polarity::active_low)) {
            return mmio_failure();
        }
        initialized_ = false; release_owner(); return {};
    }

private:
    [[nodiscard]] static constexpr bool timing_is_valid(
        spi_timing timing) noexcept {
        return timing.prescaler_encoding <= 7U && timing.clock_hz != 0U
            && timing.achieved_hz != 0U
            && timing.achieved_hz <= timing.target_hz
            && timing.achieved_hz
                == timing.clock_hz / (2U << timing.prescaler_encoding);
    }

    [[nodiscard]] static constexpr bool configuration_is_valid(
        driver::spi_configuration configuration) noexcept {
        return static_cast<std::uint8_t>(configuration.mode) <= 3U
            && static_cast<std::uint8_t>(configuration.bit_order) <= 1U
            && (configuration.frame_width == driver::spi_frame_width::bits8
                || configuration.frame_width == driver::spi_frame_width::bits16)
            && static_cast<std::uint8_t>(configuration.chip_select) <= 1U;
    }

    [[nodiscard]] bool configure_select_pin(detail::pinmux<MmioPolicy>& pins,
                                            bool inactive) noexcept {
        if constexpr (ExternalSelect) return true;
        else return pins.output(Binding::select_pin, inactive);
    }

    static constexpr std::uint32_t interrupt_mask =
        (1U << 0U) | (1U << 1U) | (1U << 3U) | (1U << 6U) | (1U << 9U);

    [[nodiscard]] bool program() noexcept {
        const auto width = static_cast<std::uint32_t>(configuration_.frame_width);
        std::uint32_t cfg1 = width - 1U;
        cfg1 |= timing_.prescaler_encoding << 28U;
        std::uint32_t cfg2 = (1U << 22U) | (1U << 26U) | (1U << 31U);
        if (configuration_.bit_order == driver::spi_bit_order::least_significant_first) {
            cfg2 |= 1U << 23U;
        }
        if (configuration_.mode == driver::spi_mode::mode1
            || configuration_.mode == driver::spi_mode::mode3) cfg2 |= 1U << 24U;
        if (configuration_.mode == driver::spi_mode::mode2
            || configuration_.mode == driver::spi_mode::mode3) cfg2 |= 1U << 25U;
        return write(0x00U, 0U) && write(0x08U, cfg1) && write(0x0CU, cfg2);
    }
    [[nodiscard]] std::expected<std::size_t, driver::spi_error> validate(
        std::span<const driver::spi_segment> segments) const noexcept {
        if (segments.empty()) return std::unexpected(driver::spi_error::empty_transfer);
        std::size_t total{};
        for (const auto& segment : segments) {
            if (segment.transmit.empty() && segment.receive.empty()) {
                return std::unexpected(driver::spi_error::empty_transfer);
            }
            if (!segment.transmit.empty() && !segment.receive.empty()
                && segment.transmit.size() != segment.receive.size()) {
                return std::unexpected(driver::spi_error::mismatched_segment);
            }
            const auto count = segment.transmit.empty()
                ? segment.receive.size() : segment.transmit.size();
            if (count > 65'535U - total) {
                return std::unexpected(driver::spi_error::excessive_length);
            }
            if (configuration_.frame_width == driver::spi_frame_width::bits8) {
                if (segment.transmit.empty() && segment.dummy > 0xFFU) {
                    return std::unexpected(driver::spi_error::invalid_frame);
                }
                for (const auto value : segment.transmit) {
                    if (value > 0xFFU) return std::unexpected(driver::spi_error::invalid_frame);
                }
            }
            total += count;
        }
        return total;
    }
    [[nodiscard]] std::uint16_t frame(std::size_t flat) const noexcept {
        for (const auto& segment : segments_) {
            const auto count = segment.transmit.empty()
                ? segment.receive.size() : segment.transmit.size();
            if (flat < count) return segment.transmit.empty()
                ? segment.dummy : segment.transmit[flat];
            flat -= count;
        }
        return 0U;
    }
    void store(std::size_t flat, std::uint16_t value) noexcept {
        for (const auto& segment : segments_) {
            const auto count = segment.transmit.empty()
                ? segment.receive.size() : segment.transmit.size();
            if (flat < count) {
                if (!segment.receive.empty()) segment.receive[flat] = value;
                return;
            }
            flat -= count;
        }
    }
    [[nodiscard]] bool set_select(bool asserted) noexcept {
        if constexpr (ExternalSelect) {
            return select_handler_.set
                && select_handler_.set(select_handler_.context, asserted);
        }
        const bool active_high = configuration_.chip_select
            == driver::spi_chip_select_polarity::active_high;
        detail::pinmux<MmioPolicy> pins{mmio_};
        return pins.set_output(Binding::select_pin, asserted == active_high);
    }
    [[nodiscard]] static core::quiesce_result cancel(
        void* context, core::operation_id id) noexcept {
        auto& self = *static_cast<spi_master_backend*>(context);
        if (!self.id_ || *self.id_ != id) return core::quiesce_result::reusable;
        const bool interrupts_stopped = self.write(0x10U, 0U);
        const bool controller_stopped = self.write(0x00U, 0U);
        const bool deselected = self.set_select(false);
        const bool stopped = interrupts_stopped && controller_stopped && deselected;
        self.reset();
        return stopped ? core::quiesce_result::reusable : core::quiesce_result::faulted;
    }
    [[nodiscard]] bool finish(driver::spi_error error) noexcept {
        if (!id_) return false;
        const auto current = *id_;
        (void)write(0x10U, 0U); (void)write(0x00U, 0U); (void)set_select(false);
        reset(); return operations_.fail(current, error);
    }
    void reset() noexcept {
        id_.reset(); segments_ = {}; total_ = 0U; transmitted_ = 0U; received_ = 0U;
    }
    [[nodiscard]] auto read(std::uintptr_t offset) noexcept {
        return mmio_.template read<std::uint32_t>(Binding::metadata.base + offset);
    }
    [[nodiscard]] std::expected<std::uint16_t, core::mmio_error>
    read_data() noexcept {
        if (configuration_.frame_width == driver::spi_frame_width::bits8) {
            const auto value = mmio_.template read<std::uint8_t>(
                Binding::metadata.base + 0x30U);
            if (!value) return std::unexpected(value.error());
            return *value;
        }
        return mmio_.template read<std::uint16_t>(
            Binding::metadata.base + 0x30U);
    }
    [[nodiscard]] bool write_data(std::uint16_t value) noexcept {
        if (configuration_.frame_width == driver::spi_frame_width::bits8) {
            return mmio_.template write<std::uint8_t>(
                Binding::metadata.base + 0x20U,
                static_cast<std::uint8_t>(value)).has_value();
        }
        return mmio_.template write<std::uint16_t>(
            Binding::metadata.base + 0x20U, value).has_value();
    }
    [[nodiscard]] bool write(std::uintptr_t offset, std::uint32_t value) noexcept {
        return mmio_.template write<std::uint32_t>(Binding::metadata.base + offset,
                                                   value).has_value();
    }
    void release_owner() noexcept {
        if (owned_) { detail::spi_owner<Binding::instance>::claimed = false; owned_ = false; }
    }
    [[nodiscard]] static start_result failure(driver::spi_error error) noexcept {
        return std::unexpected(typename pool_type::error_type{
            .kind = core::transfer_error_kind::invalid_operation, .cause = error});
    }
    [[nodiscard]] static std::expected<void, driver::spi_error>
    mmio_failure() noexcept { return std::unexpected(driver::spi_error::mmio); }

    MmioPolicy& mmio_;
    [[no_unique_address]] detail::spi_select_handler<ExternalSelect> select_handler_{};
    pool_type operations_;
    std::optional<core::operation_id> id_{};
    std::span<const driver::spi_segment> segments_{};
    spi_timing timing_{};
    driver::spi_configuration configuration_{};
    std::size_t total_{};
    std::size_t transmitted_{};
    std::size_t received_{};
    bool initialized_{};
    bool configuration_valid_{};
    bool owned_{};
};

} // namespace mcubsp::backend::stm32h753
