#pragma once

// @req-file REQ-0054 REQ-0062
#include <array>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <optional>
#include <span>
#include <mcubsp/arch/barrier.hpp>
#include <mcubsp/backend/nrf52840/detail/serial_owner.hpp>
#include <mcubsp/core/transfer.hpp>
#include <mcubsp/driver/i2c.hpp>
#include <mcubsp/mcu/nrf52840/peripherals.hpp>
#include <mcubsp/mcu/nrf52840/twi.hpp>

namespace mcubsp::backend::nrf52840 {

struct i2c_timing : driver::i2c_timing {
    std::uint32_t frequency;
};

// Rates are nominal hardware presets; HFCLK accuracy and external rise/fall
// times remain board/application assumptions, not a measured timing claim.
[[nodiscard]] constexpr std::expected<i2c_timing, driver::i2c_error>
solve_i2c_timing(driver::i2c_timing_request request) noexcept {
    using enum driver::i2c_error;
    if (request.clock_hz != mcu::nrf52840::twi_clock_hz) return std::unexpected(invalid_clock);
    if (!request.target_hz || request.target_hz > 400'000U) return std::unexpected(invalid_rate);
    constexpr std::array rates{100'000U, 250'000U, 400'000U};
    constexpr std::array encodings{0x0198'0000U, 0x0400'0000U, 0x0668'0000U};
    std::optional<i2c_timing> best;
    bool electrical_candidate{};
    for (std::size_t i = 0; i < rates.size(); ++i) {
        const auto rise_limit = rates[i] <= 100'000U ? 1000U : 300U;
        if (request.rise_time_ns > rise_limit || request.fall_time_ns > 300U) continue;
        electrical_candidate = true;
        const auto difference = rates[i] > request.target_hz
            ? rates[i] - request.target_hz : request.target_hz - rates[i];
        const auto ppm = driver::i2c_detail::divide_rounded(
            static_cast<std::uint64_t>(difference) * 1'000'000U, request.target_hz);
        if (ppm <= request.maximum_error_ppm && (!best || ppm < best->error_ppm))
            best = i2c_timing{{request.clock_hz, request.target_hz, rates[i], static_cast<std::uint32_t>(ppm),
                request.rise_time_ns, request.fall_time_ns}, encodings[i]};
    }
    if (!best) return std::unexpected(electrical_candidate ? unattainable_tolerance : invalid_rise_fall);
    return *best;
}

template<unsigned Instance, core::pin_id Scl, core::pin_id Sda>
struct i2c_binding {
    static_assert(Instance < 2U, "nRF52840 TWI instance must be 0 or 1");
    static_assert(mcu::nrf52840::gpio_supported(Scl) && mcu::nrf52840::gpio_supported(Sda),
                  "TWI pins must be available GPIOs; reset/NFC/crystal pads are reserved");
    static_assert(Scl != Sda, "TWI SCL and SDA must use distinct pins");
    static constexpr unsigned instance = Instance;
    static constexpr auto scl_pin = Scl;
    static constexpr auto sda_pin = Sda;
    static constexpr auto base = mcu::nrf52840::twi_base(Instance);
};

// Foreground polling only. The native TWI (not TWIM/EasyDMA) accesses buffers
// through the CPU. RX BB_SUSPEND / BB_STOP prevents polling-latency overruns.
template<class Binding, class Mmio, class Execution>
class i2c_controller_backend {
    using reg = mcu::nrf52840::twi_registers;
public:
    using pool_type = core::operation_pool<driver::i2c_transfer_result, driver::i2c_error, 1, Execution>;
    using transfer_type = typename pool_type::transfer_type;
    using start_result = std::expected<transfer_type, typename pool_type::error_type>;
    static constexpr std::size_t stop_poll_limit = 1024U;

    i2c_controller_backend(Mmio& mmio, Execution& execution,
                            resource_registry& resources = default_resources) noexcept
        : mmio_(mmio), execution_(execution), operations_(execution), resources_(resources) {}
    i2c_controller_backend(const i2c_controller_backend&) = delete;
    i2c_controller_backend& operator=(const i2c_controller_backend&) = delete;

    [[nodiscard]] std::expected<void, driver::i2c_error> initialize(i2c_timing timing) noexcept {
        using enum driver::i2c_error;
        if (faulted_) return std::unexpected(transport);
        if (owned_ || initialized_) return std::unexpected(already_owned);
        const auto solved = solve_i2c_timing({timing.clock_hz, timing.target_hz,
            timing.error_ppm, timing.rise_time_ns, timing.fall_time_ns});
        if (!solved || solved->frequency != timing.frequency || solved->achieved_hz != timing.achieved_hz)
            return std::unexpected(solved ? invalid_rate : solved.error());
        const std::array pins{Binding::scl_pin, Binding::sda_pin};
        auto claim = resources_.claim(execution_, Binding::instance == 0 ? peripheral::twi0 : peripheral::twi1,
                                      this, pins);
        if (!claim) return std::unexpected(already_owned);
        resource_ = *claim;
        owned_ = true;
        const auto enabled = read<reg::enable>();
        if (!enabled || *enabled != 0U) {
            release_owner();
            return std::unexpected(enabled ? already_owned : mmio);
        }
        if (!write<reg::intenclr>(reg::intenclr::writable_mask)
            || !write<reg::shorts>(0U) || !clear_events()
            || !configure_pin(Binding::scl_pin) || !configure_pin(Binding::sda_pin)
            || !write<reg::psel_scl>(mcu::nrf52840::pin_select(Binding::scl_pin))
            || !write<reg::psel_sda>(mcu::nrf52840::pin_select(Binding::sda_pin))
            || !write<reg::frequency>(timing.frequency) || !write<reg::enable>(2U)) {
            faulted_ = true;
            // Retain ownership if cleanup cannot establish disabled hardware.
            (void)shutdown();
            return std::unexpected(mmio);
        }
        initialized_ = true;
        return {};
    }

    [[nodiscard]] start_result write(driver::i2c_address address, std::span<const std::uint8_t> bytes) noexcept {
        if (bytes.empty()) return failure(driver::i2c_error::empty_transfer);
        return start(address, bytes, {});
    }
    [[nodiscard]] start_result read(driver::i2c_address address, std::span<std::uint8_t> bytes) noexcept {
        if (bytes.empty()) return failure(driver::i2c_error::empty_transfer);
        return start(address, {}, bytes);
    }
    [[nodiscard]] start_result write_read(driver::i2c_address address,
        std::span<const std::uint8_t> output, std::span<std::uint8_t> input) noexcept {
        if (output.empty() || input.empty()) return failure(driver::i2c_error::empty_transfer);
        return start(address, output, input);
    }

    // At most one data byte is processed per call. STOPPED is checked after
    // RXDREADY so a simultaneous final byte / STOP cannot lose the last byte.
    [[nodiscard]] bool poll() noexcept {
        if (!id_ || cancelling_) return false;
        const auto error = read<reg::error>();
        if (!error) return fail_closed(driver::i2c_error::mmio);
        if (*error && !pending_error_) {
            const auto source = read<reg::errorsrc>();
            if (!source || !write<reg::error>(0U) || !write<reg::errorsrc>(source.value_or(0U) & 7U))
                return fail_closed(driver::i2c_error::mmio);
            pending_error_ = (*source & 2U) ? driver::i2c_error::address_nack
                : (*source & 4U) ? driver::i2c_error::data_nack : driver::i2c_error::transport;
            if (!request_stop()) return fail_closed(driver::i2c_error::mmio);
        }
        if (!pending_error_ && phase_ == phase::transmit) {
            const auto sent = read<reg::txdsent>();
            if (!sent) return fail_closed(driver::i2c_error::mmio);
            if (*sent) {
                if (!write<reg::txdsent>(0U)) return fail_closed(driver::i2c_error::mmio);
                ++written_;
                if (written_ < output_.size()) {
                    if (!write<reg::txd>(output_[written_])) return fail_closed(driver::i2c_error::mmio);
                } else if (!input_.empty()) {
                    phase_ = phase::restart;
                    if (!write<reg::suspended>(0U) || !write<reg::task_suspend>(1U))
                        return fail_closed(driver::i2c_error::mmio);
                } else if (!request_stop()) return fail_closed(driver::i2c_error::mmio);
            }
        } else if (!pending_error_ && phase_ == phase::restart) {
            const auto suspended = read<reg::suspended>();
            if (!suspended) return fail_closed(driver::i2c_error::mmio);
            if (*suspended && !start_receive()) return fail_closed(driver::i2c_error::mmio);
        } else if (!pending_error_ && phase_ == phase::receive) {
            const auto ready = read<reg::rxdready>();
            if (!ready) return fail_closed(driver::i2c_error::mmio);
            if (*ready) {
                const auto byte = read<reg::rxd>();
                if (!byte || received_ >= input_.size() || !write<reg::rxdready>(0U))
                    return fail_closed(driver::i2c_error::mmio);
                input_[received_++] = static_cast<std::uint8_t>(*byte);
                if (received_ < input_.size()) {
                    if (!write<reg::shorts>(input_.size() - received_ == 1U
                            ? mcu::nrf52840::twi_bb_stop : mcu::nrf52840::twi_bb_suspend)
                        || !write<reg::bb>(0U) || !write<reg::suspended>(0U)
                        || !write<reg::task_resume>(1U)) return fail_closed(driver::i2c_error::mmio);
                } else phase_ = phase::stop;
            }
        }
        const auto stopped = read<reg::stopped>();
        if (!stopped) return fail_closed(driver::i2c_error::mmio);
        if (!*stopped) return false;
        // RESUME above may have completed another RX byte before this STOPPED
        // read. Leave both events pending for the next bounded poll step.
        if (!pending_error_ && phase_ == phase::receive && received_ < input_.size()) {
            const auto final_byte = read<reg::rxdready>();
            if (!final_byte) return fail_closed(driver::i2c_error::mmio);
            if (*final_byte) return false;
        }
        const auto current = *id_;
        const auto counts = driver::i2c_transfer_result{written_, received_};
        auto cause = pending_error_;
        if (!cause && (written_ != output_.size() || received_ != input_.size()))
            return fail_closed(driver::i2c_error::transport);
        if (!write<reg::shorts>(0U) || !clear_events()) return fail_closed(driver::i2c_error::mmio);
        detach();
        return cause ? operations_.fail(current, *cause) : operations_.complete(current, counts);
    }

    [[nodiscard]] std::expected<void, driver::i2c_error> shutdown() noexcept {
        if (execution_.in_isr()) return std::unexpected(driver::i2c_error::busy);
        if (!owned_) return {};
        if (id_) return std::unexpected(driver::i2c_error::busy);
        if (!disable() || !write<reg::psel_scl>(0x8000'0000U)
            || !write<reg::psel_sda>(0x8000'0000U)
            || !restore_pin(Binding::scl_pin) || !restore_pin(Binding::sda_pin))
            return std::unexpected(driver::i2c_error::mmio);
        initialized_ = false;
        // A faulted operation pool deliberately retains its retired slot.
        // Shutdown releases hardware, but reconstruct the backend after a fault.
        if (!release_owner()) return std::unexpected(driver::i2c_error::busy);
        return {};
    }
    [[nodiscard]] bool faulted() const noexcept { return faulted_; }

private:
    enum class phase { transmit, restart, receive, stop };
    [[nodiscard]] start_result start(driver::i2c_address address,
        std::span<const std::uint8_t> output, std::span<std::uint8_t> input) noexcept {
        if (!initialized_) return failure(driver::i2c_error::not_initialized);
        if (faulted_ || id_) return failure(driver::i2c_error::busy);
        if (!driver::make_i2c_address(address.value)) return failure(driver::i2c_error::invalid_address);
        const auto idle = lines_idle();
        if (!idle) return failure(idle.error());
        if (!*idle) return failure(driver::i2c_error::bus_stuck);
        auto transfer = operations_.try_start({.quiesce = &cancel, .context = this});
        if (!transfer) return std::unexpected(transfer.error());
        id_ = transfer->id(); output_ = output; input_ = input;
        written_ = received_ = 0U; pending_error_.reset();
        if (!clear_events() || !write<reg::shorts>(0U) || !write<reg::address>(address.value))
            (void)fail_closed(driver::i2c_error::mmio);
        else if (output.empty()) {
            if (!start_receive()) (void)fail_closed(driver::i2c_error::mmio);
        } else {
            phase_ = phase::transmit;
            if (!write<reg::task_resume>(1U) || !write<reg::task_starttx>(1U)
                || !write<reg::txd>(output[0])) (void)fail_closed(driver::i2c_error::mmio);
        }
        return transfer;
    }
    [[nodiscard]] bool start_receive() noexcept {
        phase_ = phase::receive;
        return write<reg::suspended>(0U) && write<reg::bb>(0U)
            && write<reg::shorts>(input_.size() == 1U ? mcu::nrf52840::twi_bb_stop : mcu::nrf52840::twi_bb_suspend)
            && write<reg::task_resume>(1U) && write<reg::task_startrx>(1U);
    }
    [[nodiscard]] bool request_stop() noexcept {
        phase_ = phase::stop;
        return write<reg::shorts>(0U) && write<reg::task_resume>(1U) && write<reg::task_stop>(1U);
    }
    [[nodiscard]] bool clear_events() noexcept {
        return write<reg::stopped>(0U) && write<reg::rxdready>(0U)
            && write<reg::txdsent>(0U) && write<reg::error>(0U)
            && write<reg::bb>(0U) && write<reg::suspended>(0U) && write<reg::errorsrc>(7U);
    }
    [[nodiscard]] bool disable() noexcept {
        // Attempt disabling even if an earlier cleanup write failed.
        bool ok = write<reg::intenclr>(reg::intenclr::writable_mask);
        ok = write<reg::shorts>(0U) && ok;
        ok = write<reg::enable>(0U) && ok;
        const auto enabled = read<reg::enable>();
        return ok && enabled && *enabled == 0U;
    }
    [[nodiscard]] static core::quiesce_result cancel(void* context, core::operation_id id) noexcept {
        auto& self = *static_cast<i2c_controller_backend*>(context);
        if (!self.id_ || *self.id_ != id) return core::quiesce_result::reusable;
        self.cancelling_ = true;
        bool stopped{};
        const bool requested = self.request_stop();
        if (requested) for (std::size_t i = 0; i < stop_poll_limit; ++i) {
            const auto value = self.template read<reg::stopped>();
            if (!value) break;
            if (*value) { stopped = true; break; }
        }
        const bool disabled = self.disable();
        const bool cleared = self.clear_events();
        const auto idle = self.lines_idle();
        const bool reusable = requested && stopped && disabled && cleared && idle && *idle
            && self.template write<reg::enable>(2U);
        self.faulted_ = !reusable;
        self.detach();
        self.cancelling_ = false;
        return reusable ? core::quiesce_result::reusable : core::quiesce_result::faulted;
    }
    [[nodiscard]] bool fail_closed(driver::i2c_error error) noexcept {
        const auto current = *id_;
        (void)disable();
        faulted_ = true;
        // No DMA is used: detaching spans and preventing further dispatch
        // guarantees this backend will not touch caller buffers again.
        detach();
        return operations_.fail(current, error);
    }
    void detach() noexcept { id_.reset(); output_ = {}; input_ = {}; pending_error_.reset(); }
    bool release_owner() noexcept {
        if (resource_ && resources_.release(execution_, *resource_)) {
            resource_.reset(); owned_ = false; return true;
        }
        return false;
    }
    [[nodiscard]] std::expected<bool, driver::i2c_error> lines_idle() noexcept {
        const auto scl = mmio_.template read<std::uint32_t>(mcu::nrf52840::gpio_base(Binding::scl_pin) + 0x510U);
        const auto sda = mmio_.template read<std::uint32_t>(mcu::nrf52840::gpio_base(Binding::sda_pin) + 0x510U);
        if (!scl || !sda) return std::unexpected(driver::i2c_error::mmio);
        return (*scl & (1U << Binding::scl_pin.index)) && (*sda & (1U << Binding::sda_pin.index));
    }
    [[nodiscard]] bool configure_pin(core::pin_id pin) noexcept {
        // Connected input, external pull-ups, standard-drive open drain S0D1.
        return mmio_.template write<std::uint32_t>(mcu::nrf52840::pin_configuration(pin), 6U << 8U).has_value();
    }
    [[nodiscard]] bool restore_pin(core::pin_id pin) noexcept {
        return mmio_.template write<std::uint32_t>(mcu::nrf52840::pin_configuration(pin), 0U).has_value();
    }
    template<class R> [[nodiscard]] auto read() noexcept {
        return mmio_.template read<std::uint32_t>(Binding::base + R::offset);
    }
    template<class R> [[nodiscard]] bool write(std::uint32_t value) noexcept {
        const bool ok = mmio_.template write<std::uint32_t>(Binding::base + R::offset, value).has_value();
        arch::data_memory_barrier();
        return ok;
    }
    [[nodiscard]] static start_result failure(driver::i2c_error error) noexcept {
        return std::unexpected(typename pool_type::error_type{core::transfer_error_kind::failure, error});
    }
    Mmio& mmio_;
    Execution& execution_;
    pool_type operations_;
    resource_registry& resources_;
    std::optional<resource_token> resource_;
    std::optional<core::operation_id> id_;
    std::optional<driver::i2c_error> pending_error_;
    std::span<const std::uint8_t> output_;
    std::span<std::uint8_t> input_;
    std::size_t written_{}, received_{};
    phase phase_{};
    bool initialized_{}, owned_{}, faulted_{}, cancelling_{};
};

} // namespace mcubsp::backend::nrf52840
