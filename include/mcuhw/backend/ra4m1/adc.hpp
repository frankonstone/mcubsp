#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <optional>
#include <span>

#include <mcuhw/arch/barrier.hpp>
#include <mcuhw/core/metadata.hpp>
#include <mcuhw/core/transfer.hpp>
#include <mcuhw/driver/adc.hpp>
#include <mcuhw/mcu/ra4m1/description.hpp>

namespace mcuhw::backend::ra4m1 {

namespace adc_detail {

struct owner {
    static inline bool claimed{};
};

[[nodiscard]] constexpr mcu::ra4m1::signal signal_for(
    std::uint8_t channel) noexcept {
    return static_cast<mcu::ra4m1::signal>(
        static_cast<std::uint16_t>(mcu::ra4m1::signal::adc_an000) + channel);
}

[[nodiscard]] constexpr mcu::ra4m1::instance_record metadata() noexcept {
    for (const auto& candidate : mcu::ra4m1::instances) {
        if (candidate.id.kind
            == static_cast<std::uint16_t>(
                mcu::ra4m1::peripheral_kind::adc14)) {
            return candidate;
        }
    }
    return {};
}

template<std::size_t N>
[[nodiscard]] consteval bool unique(const std::array<std::uint8_t, N>& values) {
    for (std::size_t first = 0U; first < N; ++first) {
        for (std::size_t second = first + 1U; second < N; ++second) {
            if (values[first] == values[second]) {
                return false;
            }
        }
    }
    return true;
}

} // namespace adc_detail

template<std::uint8_t Channel, core::pin_id Pin>
struct adc_input {
    static_assert(Channel <= 4U,
                  "this RA4M1 package description exposes AN000..AN004");
    static_assert(core::route_exists(
        mcu::ra4m1::pin_routes, Pin,
        mcu::ra4m1::signal_id(adc_detail::signal_for(Channel))));
    static constexpr std::uint8_t channel = Channel;
    static constexpr core::pin_id pin = Pin;
};

template<typename... Inputs>
struct adc14_binding {
    static_assert(sizeof...(Inputs) > 0U);
    static constexpr std::array<std::uint8_t, sizeof...(Inputs)> channels{
        Inputs::channel...};
    static constexpr std::array<core::pin_id, sizeof...(Inputs)> pins{
        Inputs::pin...};
    static_assert(adc_detail::unique(channels));
    static constexpr auto metadata = adc_detail::metadata();
};

template<typename Binding, typename MmioPolicy, typename ExecutionPolicy>
// @req REQ-0014 REQ-0017 REQ-0019 REQ-0029
class adc14_backend {
public:
    using pool_type = core::operation_pool<driver::adc_sequence_result,
                                           driver::adc_error, 1,
                                           ExecutionPolicy>;
    using transfer_type = typename pool_type::transfer_type;
    using start_result = std::expected<transfer_type,
                                       typename pool_type::error_type>;

    explicit adc14_backend(MmioPolicy& mmio,
                           ExecutionPolicy& execution) noexcept
        : mmio_(mmio), operations_(execution) {}

    [[nodiscard]] std::expected<void, driver::adc_error>
    initialize(driver::adc_configuration configuration = {}) noexcept {
        if (initialized_ || adc_detail::owner::claimed) {
            return std::unexpected(driver::adc_error::already_owned);
        }
        // RA4M1 requires more than five sampling states for the supported
        // PCLKB:PCLKC ratios. The reference and result-format enums expose
        // only the hardware mode this backend implements.
        if (configuration.sample_states < 6U) {
            return std::unexpected(driver::adc_error::invalid_sample_time);
        }
        adc_detail::owner::claimed = true;
        owned_ = true;
        configuration_ = configuration;
        if (!release_module_stop() || !configure_pins()
            || !program_converter()) {
            static_cast<void>(restore_pins());
            release_owner();
            return mmio_failure();
        }
        initialized_ = true;
        return {};
    }

    [[nodiscard]] start_result
    convert(std::span<const driver::adc_channel> channels,
            std::span<std::uint16_t> results) noexcept {
        if (!initialized_) {
            return start_failure(driver::adc_error::not_initialized);
        }
        if (operation_id_) {
            return start_failure(driver::adc_error::busy);
        }
        if (channels.empty()) {
            return start_failure(driver::adc_error::empty_sequence);
        }
        if (channels.size() != results.size()) {
            return start_failure(driver::adc_error::result_size_mismatch);
        }
        if (channels.size() > 15U) {
            return start_failure(driver::adc_error::excessive_length);
        }
        std::uint16_t mask{};
        for (const auto channel : channels) {
            if (!is_bound(channel.value)) {
                return start_failure(driver::adc_error::invalid_channel);
            }
            const auto bit = static_cast<std::uint16_t>(1U << channel.value);
            if ((mask & bit) != 0U) {
                return start_failure(driver::adc_error::duplicate_channel);
            }
            mask = static_cast<std::uint16_t>(mask | bit);
        }

        auto started = operations_.try_start(
            {.quiesce = &cancel_operation, .context = this});
        if (!started) {
            return std::unexpected(started.error());
        }
        operation_id_ = started->id();
        channels_ = channels;
        results_ = results;
        if (!write<std::uint16_t>(base() + 0x04U, mask)
            || !write<std::uint16_t>(base() + 0x00U, adc_start)) {
            static_cast<void>(finish_error(driver::adc_error::mmio));
        }
        return started;
    }

    [[nodiscard]] bool handle_scan_end_interrupt() noexcept {
        if (!operation_id_) {
            return false;
        }
        const auto control = mmio_.template read<std::uint16_t>(base() + 0x00U);
        if (!control) {
            return finish_error(driver::adc_error::mmio);
        }
        if ((*control & adc_start) != 0U) {
            return false;
        }
        for (std::size_t index = 0U; index < channels_.size(); ++index) {
            const auto value = mmio_.template read<std::uint16_t>(
                base() + 0x20U + 2U * channels_[index].value);
            if (!value) {
                return finish_error(driver::adc_error::mmio);
            }
            results_[index] = *value;
        }
        const auto id = *operation_id_;
        const auto samples = channels_.size();
        if (!write<std::uint16_t>(base() + 0x04U, 0U)) {
            return finish_error(driver::adc_error::mmio);
        }
        reset_conversion();
        return operations_.complete(id, driver::adc_sequence_result{samples});
    }

    [[nodiscard]] std::expected<void, driver::adc_error> shutdown() noexcept {
        if (!initialized_) {
            return std::unexpected(driver::adc_error::not_initialized);
        }
        if (operation_id_) {
            return std::unexpected(driver::adc_error::busy);
        }
        if (!write<std::uint16_t>(base() + 0x00U, 0U)
            || !write<std::uint16_t>(base() + 0x04U, 0U)
            || !restore_pins()) {
            return mmio_failure();
        }
        initialized_ = false;
        release_owner();
        return {};
    }

private:
    static constexpr std::uint16_t adc_start = 1U << 15U;

    [[nodiscard]] static constexpr std::uintptr_t base() noexcept {
        return Binding::metadata.base;
    }

    [[nodiscard]] static constexpr bool is_bound(std::uint8_t channel) noexcept {
        for (const auto candidate : Binding::channels) {
            if (candidate == channel) {
                return true;
            }
        }
        return false;
    }

    [[nodiscard]] static core::quiesce_result cancel_operation(
        void* context, core::operation_id id) noexcept {
        auto& self = *static_cast<adc14_backend*>(context);
        if (!self.operation_id_ || *self.operation_id_ != id) {
            return core::quiesce_result::reusable;
        }
        // Clearing ADST is the manual's software stop operation. State and
        // spans are dropped synchronously, so a late ADI cannot touch them.
        const bool stopped = self.template write<std::uint16_t>(
                                 base() + 0x00U, 0U)
            && self.template write<std::uint16_t>(base() + 0x04U, 0U);
        self.reset_conversion();
        return stopped ? core::quiesce_result::reusable
                       : core::quiesce_result::faulted;
    }

    [[nodiscard]] bool finish_error(driver::adc_error error) noexcept {
        if (!operation_id_) {
            return false;
        }
        const auto id = *operation_id_;
        static_cast<void>(write<std::uint16_t>(base() + 0x00U, 0U));
        static_cast<void>(write<std::uint16_t>(base() + 0x04U, 0U));
        reset_conversion();
        return operations_.fail(id, error);
    }

    void reset_conversion() noexcept {
        operation_id_.reset();
        channels_ = {};
        results_ = {};
    }

    [[nodiscard]] bool program_converter() noexcept {
        const auto extended = configuration_.resolution
                == driver::adc_resolution::bits14
            ? static_cast<std::uint16_t>(0x0006U)
            : static_cast<std::uint16_t>(0x0000U);
        if (!write<std::uint16_t>(base() + 0x00U, 0U)
            || !write<std::uint16_t>(base() + 0x04U, 0U)
            || !write<std::uint16_t>(base() + 0x06U, 0U)
            || !write<std::uint8_t>(base() + 0x0CU, 0U)
            || !write<std::uint16_t>(base() + 0x0EU, extended)
            || !write<std::uint16_t>(base() + 0x10U, 0U)
            || !write<std::uint16_t>(base() + 0x12U, 0U)) {
            return false;
        }
        for (const auto channel : Binding::channels) {
            if (!write<std::uint8_t>(base() + 0xE0U + channel,
                                     configuration_.sample_states)) {
                return false;
            }
        }
        return true;
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
        for (const auto pin : Binding::pins) {
            if (!configure_pin(pin)) {
                return false;
            }
        }
        return true;
    }

    [[nodiscard]] bool restore_pins() noexcept {
        for (const auto pin : Binding::pins) {
            if (!update_pfs(pin, false)) {
                return false;
            }
        }
        return true;
    }

    [[nodiscard]] bool configure_pin(core::pin_id pin) noexcept {
        return update_pfs(pin, true);
    }

    [[nodiscard]] bool update_pfs(core::pin_id pin, bool analog) noexcept {
        constexpr std::uintptr_t pwpr = 0x4004'0D03U;
        constexpr std::uint32_t controlled =
            (0x1FU << 24U) | (1U << 16U) | (1U << 15U) | (1U << 4U)
            | (1U << 2U) | 1U;
        const auto address = mcu::ra4m1::pfs_address(pin);
        const auto current = mmio_.template read<std::uint32_t>(address);
        if (!current || !write<std::uint8_t>(pwpr, 0U)
            || !write<std::uint8_t>(pwpr, 0x40U)) {
            return false;
        }
        const auto selected = analog ? static_cast<std::uint32_t>(1U << 15U)
                                     : 0U;
        const bool configured = write<std::uint32_t>(
            address, (*current & ~controlled) | selected);
        const bool locked = write<std::uint8_t>(pwpr, 0U)
            && write<std::uint8_t>(pwpr, 0x80U);
        arch::data_memory_barrier();
        return configured && locked;
    }

    template<typename Value>
    [[nodiscard]] bool write(std::uintptr_t address, Value value) noexcept {
        return mmio_.template write<Value>(address, value).has_value();
    }

    [[nodiscard]] static start_result
    start_failure(driver::adc_error error) noexcept {
        return std::unexpected(typename pool_type::error_type{
            .kind = core::transfer_error_kind::failure, .cause = error});
    }

    [[nodiscard]] static std::expected<void, driver::adc_error>
    mmio_failure() noexcept {
        return std::unexpected(driver::adc_error::mmio);
    }

    void release_owner() noexcept {
        if (owned_) {
            adc_detail::owner::claimed = false;
            owned_ = false;
        }
    }

    MmioPolicy& mmio_;
    pool_type operations_;
    driver::adc_configuration configuration_{};
    std::span<const driver::adc_channel> channels_{};
    std::span<std::uint16_t> results_{};
    std::optional<core::operation_id> operation_id_{};
    bool owned_{};
    bool initialized_{};
};

} // namespace mcuhw::backend::ra4m1
