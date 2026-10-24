#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <optional>
#include <span>

#include <mcubsp/arch/barrier.hpp>
#include <mcubsp/backend/stm32h753/detail.hpp>
#include <mcubsp/core/transfer.hpp>
#include <mcubsp/driver/adc.hpp>

namespace mcubsp::backend::stm32h753 {

namespace detail {
struct adc_owner { static inline bool claimed{}; };

template<std::size_t N>
[[nodiscard]] consteval bool adc_unique(const std::array<std::uint8_t, N>& values) {
    for (std::size_t first = 0U; first < N; ++first) {
        for (std::size_t second = first + 1U; second < N; ++second) {
            if (values[first] == values[second]) return false;
        }
    }
    return true;
}

[[nodiscard]] constexpr mcu::stm32h753::signal adc_signal(
    std::uint8_t channel) noexcept {
    return channel == 15U ? mcu::stm32h753::signal::adc1_inp15
                          : mcu::stm32h753::signal::adc1_inp16;
}
} // namespace detail

template<std::uint8_t Channel, core::pin_id Pin>
struct adc_input {
    static_assert(Channel == 15U || Channel == 16U,
                  "source-backed initial ADC1 routes cover INP15 and INP16");
    static_assert(core::route_exists(
        mcu::stm32h753::pin_routes, Pin,
        mcu::stm32h753::signal_id(detail::adc_signal(Channel))));
    static constexpr std::uint8_t channel = Channel;
    static constexpr core::pin_id pin = Pin;
};

template<typename... Inputs>
struct adc1_binding {
    static_assert(sizeof...(Inputs) > 0U);
    static constexpr std::array<std::uint8_t, sizeof...(Inputs)> channels{
        Inputs::channel...};
    static constexpr std::array<core::pin_id, sizeof...(Inputs)> pins{
        Inputs::pin...};
    static_assert(detail::adc_unique(channels));
    static constexpr auto metadata = detail::instance(
        mcu::stm32h753::peripheral_kind::adc, 1U);
};

// ADC1 regular-sequence backend. Initialization selects HSI as PER_CK,
// divides it to a legal 32 MHz ADC kernel clock, exits deep-power-down,
// calibrates, and waits with bounded polls for each hardware transition.
template<typename Binding, typename MmioPolicy, typename ExecutionPolicy>
// @req REQ-0003 REQ-0009 REQ-0014 REQ-0017 REQ-0019 REQ-0029 REQ-0033
class adc_backend {
public:
    using pool_type = core::operation_pool<driver::adc_sequence_result,
                                           driver::adc_error, 1,
                                           ExecutionPolicy>;
    using transfer_type = typename pool_type::transfer_type;
    using start_result = std::expected<transfer_type,
                                       typename pool_type::error_type>;

    adc_backend(MmioPolicy& mmio, ExecutionPolicy& execution) noexcept
        : mmio_(mmio), operations_(execution) {}

    [[nodiscard]] std::expected<void, driver::adc_error> initialize(
        driver::adc_configuration configuration = {}) noexcept {
        if (initialized_ || detail::adc_owner::claimed) {
            return std::unexpected(driver::adc_error::already_owned);
        }
        const auto sample = sample_encoding(configuration.sample_states);
        if (!sample) return std::unexpected(driver::adc_error::invalid_sample_time);
        detail::adc_owner::claimed = true; owned_ = true;
        configuration_ = configuration;
        detail::pinmux<MmioPolicy> pins{mmio_};
        for (const auto pin : Binding::pins) {
            if (!pins.analog(pin)) { release_owner(); return mmio_failure(); }
        }
        if (!detail::enable_peripheral_clock(mmio_, Binding::metadata)
            || !configure_kernel_clock() || !power_and_calibrate()
            || !program(*sample) || !enable_converter()) {
            release_owner(); return mmio_failure();
        }
        initialized_ = true; return {};
    }

    [[nodiscard]] start_result convert(
        std::span<const driver::adc_channel> channels,
        std::span<std::uint16_t> results) noexcept {
        if (!initialized_) return failure(driver::adc_error::not_initialized);
        if (id_) return failure(driver::adc_error::busy);
        if (channels.empty()) return failure(driver::adc_error::empty_sequence);
        if (channels.size() != results.size()) {
            return failure(driver::adc_error::result_size_mismatch);
        }
        if (channels.size() > 16U) return failure(driver::adc_error::excessive_length);
        std::uint32_t seen{};
        for (const auto channel : channels) {
            if (!bound(channel.value)) return failure(driver::adc_error::invalid_channel);
            const auto bit = 1U << channel.value;
            if ((seen & bit) != 0U) return failure(driver::adc_error::duplicate_channel);
            seen |= bit;
        }
        auto started = operations_.try_start({.quiesce = &cancel, .context = this});
        if (!started) return std::unexpected(started.error());
        id_ = started->id(); channels_ = channels; results_ = results; index_ = 0U;
        if (!program_sequence(channels) || !write(0x00U, 0x1FU)
            || !write(0x04U, (1U << 2U) | (1U << 3U) | (1U << 4U))
            || !update(0x08U, 1U << 2U, 0U)) {
            (void)finish_error(driver::adc_error::mmio);
        }
        return started;
    }

    [[nodiscard]] bool handle_conversion_interrupt() noexcept {
        if (!id_) return false;
        const auto status = read(0x00U);
        if (!status) return finish_error(driver::adc_error::mmio);
        if ((*status & (1U << 4U)) != 0U) {
            (void)write(0x00U, 1U << 4U);
            return finish_error(driver::adc_error::conversion_overrun);
        }
        if ((*status & (1U << 2U)) != 0U && index_ < results_.size()) {
            const auto value = read(0x40U);
            if (!value) return finish_error(driver::adc_error::mmio);
            results_[index_++] = static_cast<std::uint16_t>(*value);
            (void)write(0x00U, 1U << 2U);
        }
        if ((*status & (1U << 3U)) != 0U && index_ == results_.size()) {
            const auto current = *id_; const auto count = index_;
            const bool stopped = write(0x04U, 0U) && write(0x00U, 1U << 3U);
            reset();
            return stopped
                ? operations_.complete(current, driver::adc_sequence_result{count})
                : operations_.fail(current, driver::adc_error::mmio);
        }
        return true;
    }
    [[nodiscard]] bool handle_scan_end_interrupt() noexcept {
        return handle_conversion_interrupt();
    }

    [[nodiscard]] std::expected<void, driver::adc_error> shutdown() noexcept {
        if (!initialized_) return std::unexpected(driver::adc_error::not_initialized);
        if (id_) return std::unexpected(driver::adc_error::busy);
        const auto control = read(0x08U);
        if (!control || !write(0x04U, 0U)
            || (((*control & 1U) != 0U) && !write(0x08U, *control | (1U << 1U)))) {
            return mmio_failure();
        }
        detail::pinmux<MmioPolicy> pins{mmio_};
        for (const auto pin : Binding::pins) {
            if (!pins.restore(pin)) return mmio_failure();
        }
        initialized_ = false; release_owner(); return {};
    }

private:
    static constexpr std::size_t maximum_polls = 100'000U;

    [[nodiscard]] static constexpr std::optional<std::uint8_t> sample_encoding(
        std::uint8_t states) noexcept {
        if (states < 2U) return std::nullopt;
        if (states <= 2U) return 0U;
        if (states <= 3U) return 1U;
        if (states <= 9U) return 2U;
        if (states <= 17U) return 3U;
        if (states <= 33U) return 4U;
        if (states <= 65U) return 5U;
        return 6U;
    }
    [[nodiscard]] static constexpr bool bound(std::uint8_t channel) noexcept {
        for (const auto candidate : Binding::channels) {
            if (candidate == channel) return true;
        }
        return false;
    }
    [[nodiscard]] bool configure_kernel_clock() noexcept {
        constexpr auto rcc = mcu::stm32h753::rcc_base;
        const auto d1ccipr = mmio_.template read<std::uint32_t>(rcc + 0x4CU);
        const auto d3ccipr = mmio_.template read<std::uint32_t>(rcc + 0x58U);
        if (!d1ccipr || !d3ccipr) return false;
        // CKPERSEL=00 selects HSI; ADCSEL=10 selects PER_CK.
        if (!mmio_.template write<std::uint32_t>(rcc + 0x4CU,
                *d1ccipr & ~(3U << 28U))
            || !mmio_.template write<std::uint32_t>(rcc + 0x58U,
                (*d3ccipr & ~(3U << 16U)) | (2U << 16U))) return false;
        constexpr std::uintptr_t adc12_ccr = 0x4002'2308U;
        const auto common = mmio_.template read<std::uint32_t>(adc12_ccr);
        return common && mmio_.template write<std::uint32_t>(
            adc12_ccr, (*common & ~(0xFU << 18U)) | (1U << 18U)).has_value();
    }
    [[nodiscard]] bool power_and_calibrate() noexcept {
        const auto control = read(0x08U);
        if (!control || !write(0x08U,
            (*control & ~((1U << 29U) | (3U << 8U)))
                | (1U << 28U) | (3U << 8U))) return false;
        for (std::size_t delay = 0U; delay < 1024U; ++delay) {
            arch::data_memory_barrier();
        }
        if (!wait_set(0x00U, 1U << 12U)) return false; // LDORDY
        if (!update(0x08U, 1U << 31U, 0U)) return false;
        return wait_clear(0x08U, 1U << 31U);
    }
    [[nodiscard]] bool program(std::uint8_t sample) noexcept {
        const auto resolution = configuration_.resolution
                == driver::adc_resolution::bits14 ? 1U : 2U;
        std::uint32_t selected{};
        std::uint32_t smpr1{};
        std::uint32_t smpr2{};
        for (const auto channel : Binding::channels) {
            selected |= 1U << channel;
            if (channel < 10U) smpr1 |= static_cast<std::uint32_t>(sample)
                << (3U * channel);
            else smpr2 |= static_cast<std::uint32_t>(sample)
                << (3U * (channel - 10U));
        }
        return write(0x0CU, resolution << 2U) && write(0x10U, 0U)
            && write(0x14U, smpr1) && write(0x18U, smpr2)
            && write(0x1CU, selected);
    }
    [[nodiscard]] bool enable_converter() noexcept {
        if (!write(0x00U, 1U) || !update(0x08U, 1U, 0U)) return false;
        return wait_set(0x00U, 1U);
    }
    [[nodiscard]] bool program_sequence(
        std::span<const driver::adc_channel> channels) noexcept {
        std::array<std::uint32_t, 4> registers{};
        registers[0] = static_cast<std::uint32_t>(channels.size() - 1U);
        for (std::size_t index = 0U; index < channels.size(); ++index) {
            const auto slot = index + 1U;
            if (slot <= 4U) registers[0] |= static_cast<std::uint32_t>(channels[index].value)
                << (6U + 6U * index);
            else {
                const auto group = (slot - 5U) / 5U + 1U;
                const auto within = (slot - 5U) % 5U;
                registers[group] |= static_cast<std::uint32_t>(channels[index].value)
                    << (6U * within);
            }
        }
        return write(0x30U, registers[0]) && write(0x34U, registers[1])
            && write(0x38U, registers[2]) && write(0x3CU, registers[3]);
    }
    [[nodiscard]] bool wait_clear(std::uintptr_t offset, std::uint32_t bit) noexcept {
        for (std::size_t poll = 0U; poll < maximum_polls; ++poll) {
            const auto value = read(offset);
            if (!value) return false;
            if ((*value & bit) == 0U) return true;
        }
        return false;
    }
    [[nodiscard]] bool wait_set(std::uintptr_t offset, std::uint32_t bit) noexcept {
        for (std::size_t poll = 0U; poll < maximum_polls; ++poll) {
            const auto value = read(offset);
            if (!value) return false;
            if ((*value & bit) != 0U) return true;
        }
        return false;
    }
    [[nodiscard]] static core::quiesce_result cancel(
        void* context, core::operation_id id) noexcept {
        auto& self = *static_cast<adc_backend*>(context);
        if (!self.id_ || *self.id_ != id) return core::quiesce_result::reusable;
        const bool stopped = self.write(0x04U, 0U)
            && self.update(0x08U, 1U << 4U, 0U);
        self.reset();
        return stopped ? core::quiesce_result::reusable : core::quiesce_result::faulted;
    }
    [[nodiscard]] bool finish_error(driver::adc_error error) noexcept {
        if (!id_) return false;
        const auto current = *id_; (void)write(0x04U, 0U); reset();
        return operations_.fail(current, error);
    }
    void reset() noexcept { id_.reset(); channels_ = {}; results_ = {}; index_ = 0U; }
    [[nodiscard]] bool update(std::uintptr_t offset, std::uint32_t set,
                              std::uint32_t clear) noexcept {
        const auto value = read(offset);
        return value && write(offset, (*value | set) & ~clear);
    }
    [[nodiscard]] auto read(std::uintptr_t offset) noexcept {
        return mmio_.template read<std::uint32_t>(Binding::metadata.base + offset);
    }
    [[nodiscard]] bool write(std::uintptr_t offset, std::uint32_t value) noexcept {
        return mmio_.template write<std::uint32_t>(Binding::metadata.base + offset,
                                                   value).has_value();
    }
    void release_owner() noexcept {
        if (owned_) { detail::adc_owner::claimed = false; owned_ = false; }
    }
    [[nodiscard]] static start_result failure(driver::adc_error error) noexcept {
        return std::unexpected(typename pool_type::error_type{
            .kind = core::transfer_error_kind::invalid_operation, .cause = error});
    }
    [[nodiscard]] static std::expected<void, driver::adc_error>
    mmio_failure() noexcept { return std::unexpected(driver::adc_error::mmio); }

    MmioPolicy& mmio_;
    pool_type operations_;
    driver::adc_configuration configuration_{};
    std::span<const driver::adc_channel> channels_{};
    std::span<std::uint16_t> results_{};
    std::optional<core::operation_id> id_{};
    std::size_t index_{};
    bool initialized_{};
    bool owned_{};
};

} // namespace mcubsp::backend::stm32h753
