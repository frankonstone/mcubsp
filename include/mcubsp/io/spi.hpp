#pragma once

// @req-file REQ-0014 REQ-0018 REQ-0019 REQ-0028 REQ-0045 REQ-0046
#include <array>
#include <chrono>
#include <cstdint>
#include <optional>
#include <span>
#include <mcubsp/driver/spi.hpp>
#include <mcubsp/io/core.hpp>
#include <mcubsp/io/operation.hpp>
#include <mcubsp/io/spi_device.hpp>

namespace mcubsp::io {

// One controller, multiple device leases, no hidden queue. All calls are from
// one foreground context. Each operation owns the bus through wire idle/CS
// deassertion; retained result handles also prevent device release/reconfigure.
template<class Context>
class spi_bus {
    using board_type = typename Context::board_type;
public:
    using backend_type = typename board_type::template spi_backend<
        typename Context::mmio_type, typename Context::execution_type>;
    using timing_type = typename decltype(board_type::solve_spi(0U))::value_type;
    using gpio_backend_type = typename board_type::template gpio_backend<typename Context::mmio_type>;
    using transfer_operation = operation<typename backend_type::transfer_type, spi_bus>;
    using device_type = spi_device<spi_bus>;

    explicit spi_bus(Context& context) noexcept
        : context_(context), backend_(detail::make_backend<backend_type>(context)), gpio_(context.mmio()) {}
    spi_bus(const spi_bus&) = delete;
    spi_bus& operator=(const spi_bus&) = delete;
    spi_bus(spi_bus&&) = delete;
    spi_bus& operator=(spi_bus&&) = delete;
    ~spi_bus() { if (initialized_ && !shutdown()) lifetime_violation(); }

    [[nodiscard]] result<void> begin(std::uint32_t hz = 1'000'000U,
                                    driver::spi_configuration configuration = {}) noexcept {
        if (context_.execution().in_isr()) return std::unexpected(error{errc::invalid_context});
        if (!context_.started()) return failure(driver::spi_error::not_initialized);
        if (initialized_) return failure(driver::spi_error::already_owned);
        if (!valid(configuration)) return failure(driver::spi_error::invalid_frame);
        const auto timing = board_type::solve_spi(hz);
        if (!timing) return failure(timing.error());
        const auto claimed = context_.claim(bus_pins());
        if (!claimed) return claimed;
        const auto handler = backend_.set_select_handler(&select, this);
        if (!handler) {
            context_.release(bus_pins());
            return failure(handler.error());
        }
        const auto initialized = backend_.initialize(*timing, configuration);
        if (!initialized) {
            if constexpr (requires { backend_.needs_cleanup(); })
                if (backend_.needs_cleanup()) {
                    initialized_ = true;
                    context_.retain();
                    return failure(initialized.error());
                }
            context_.release(bus_pins());
            return failure(initialized.error());
        }
        default_timing_ = *timing;
        default_configuration_ = configuration;
        initialized_ = true;
        context_.retain();
        return {};
    }
    [[nodiscard]] result<void> shutdown() noexcept {
        if (context_.execution().in_isr())
            return std::unexpected(error{errc::invalid_context});
        if (!initialized_)
            return {};
        if (devices_ || operations_ || backend_.busy() || selected_) return std::unexpected(error{errc::busy});
        shutdown_pending_ = true;
        if (default_claimed_) {
            auto released = release_select(board_type::spi_binding::select_pin);
            if (!released) return released;
            default_claimed_ = false;
        }
        auto stopped = backend_.shutdown();
        if (!stopped) return failure(stopped.error());
        initialized_ = false;
        shutdown_pending_ = false;
        configured_ = false;
        context_.release(bus_pins());
        context_.drop();
        return {};
    }
    void poll() noexcept {
        if (initialized_ && !context_.execution().in_isr()) board_type::poll_spi(context_.mmio(), backend_);
    }
    [[nodiscard]] result<device_type> device(spi_device_configuration configuration) noexcept {
        if (auto checked = check_context(); !checked) return std::unexpected(checked.error());
        const driver::spi_configuration native{
            configuration.mode, configuration.bit_order, configuration.frame_width, configuration.chip_select};
        if (!valid(native)) return std::unexpected(to_error(driver::spi_error::invalid_frame));
        auto timing = board_type::solve_spi(configuration.frequency);
        if (!timing) return std::unexpected(to_error(timing.error()));
        // GPIO setup cannot alter a running controller or change its clock pins.
        auto claimed = claim_select(configuration.cs, native);
        if (!claimed) return std::unexpected(claimed.error());
        ++devices_;
        return device_type{*this, configuration.cs, *timing, native};
    }

    // Direct bus methods retain the original default device convenience. Its
    // CS is claimed on first use; device(default_cs) can claim it beforehand.
    [[nodiscard]] result<transfer_operation> transaction_async(
        std::span<const driver::spi_segment> segments) noexcept {
        if (auto prepared = prepare_default(); !prepared) return std::unexpected(prepared.error());
        last_used_dma_ = false;
        return wrap_operation(backend_.transaction(segments), *this);
    }
    [[nodiscard]] result<driver::spi_transfer_result> transaction_until(
        std::span<const driver::spi_segment> segments, execution::deadline limit) noexcept {
        if (auto checked = check_deadline(limit); !checked) return std::unexpected(checked.error());
        auto started = transaction_async(segments);
        if (!started) return std::unexpected(started.error());
        return started->finish_until(limit);
    }
    template<class Duration = std::chrono::milliseconds>
    [[nodiscard]] result<driver::spi_transfer_result> transaction(
        std::span<const driver::spi_segment> segments,
        Duration timeout = std::chrono::milliseconds{1000}) noexcept {
        auto limit = deadline_after(timeout);
        if (!limit) return std::unexpected(limit.error());
        return transaction_until(segments, *limit);
    }
    template<class Duration = std::chrono::milliseconds>
    [[nodiscard]] result<driver::spi_transfer_result> transfer(
        std::span<const std::uint16_t> transmit, std::span<std::uint16_t> receive,
        Duration timeout = std::chrono::milliseconds{1000}) noexcept {
        const driver::spi_segment segment{transmit, receive};
        return transaction(std::span{&segment, 1U}, timeout);
    }
    template<class Duration = std::chrono::milliseconds>
    [[nodiscard]] result<driver::spi_transfer_result> transfer(
        std::span<const std::uint8_t> transmit, std::span<std::uint8_t> receive,
        Duration timeout = std::chrono::milliseconds{1000}) noexcept {
        const auto segment = driver::byte_segment(transmit, receive);
        return transaction(std::span{&segment, 1U}, timeout);
    }
    template<class Duration = std::chrono::milliseconds>
    [[nodiscard]] result<std::uint16_t> transfer(
        std::uint16_t value, Duration timeout = std::chrono::milliseconds{1000}) noexcept {
        std::uint16_t received{};
        auto result = transfer(std::span{&value, 1U}, std::span{&received, 1U}, timeout);
        if (!result) return std::unexpected(result.error());
        return received;
    }
    template<class Selection>
        requires requires(backend_type& backend, Selection selection, driver::spi_dma_buffers buffers) {
            backend.transaction_dma(buffers, selection.receive.backend(), selection.transmit.backend());
        }
    [[nodiscard]] result<transfer_operation> transfer_async(
        driver::spi_dma_buffers buffers, Selection selection) noexcept {
        if (!selection.receive.valid() || !selection.transmit.valid()) return std::unexpected(error{errc::not_started});
        if (auto prepared = prepare_default(); !prepared) return std::unexpected(prepared.error());
        last_used_dma_ = true;
        return wrap_operation(backend_.transaction_dma(
            buffers, selection.receive.backend(), selection.transmit.backend()), *this);
    }
    template<class Selection, class Duration = std::chrono::milliseconds>
        requires requires(backend_type& backend, Selection selection, driver::spi_dma_buffers buffers) {
            backend.transaction_dma(buffers, selection.receive.backend(), selection.transmit.backend());
        }
    [[nodiscard]] result<driver::spi_transfer_result> transfer(
        driver::spi_dma_buffers buffers, Selection selection,
        Duration timeout = std::chrono::milliseconds{1000}) noexcept {
        auto limit = deadline_after(timeout);
        if (!limit) return std::unexpected(limit.error());
        if (auto checked = check_deadline(*limit); !checked) return std::unexpected(checked.error());
        auto started = transfer_async(buffers, selection);
        if (!started) return std::unexpected(started.error());
        return started->finish_until(*limit);
    }
    template<class Selection>
        requires requires(backend_type& backend, Selection selection, driver::spi_dma_buffers buffers) {
            backend.transaction_dma(buffers, selection.receive.backend(), selection.transmit.backend());
        }
    [[nodiscard]] result<transfer_operation> transaction_async(
        driver::spi_dma_buffers buffers, Selection selection) noexcept { return transfer_async(buffers, selection); }
    template<class Selection, class Duration = std::chrono::milliseconds>
        requires requires(backend_type& backend, Selection selection, driver::spi_dma_buffers buffers) {
            backend.transaction_dma(buffers, selection.receive.backend(), selection.transmit.backend());
        }
    [[nodiscard]] result<driver::spi_transfer_result> transaction(
        driver::spi_dma_buffers buffers, Selection selection,
        Duration timeout = std::chrono::milliseconds{1000}) noexcept { return transfer(buffers, selection, timeout); }

    template<class Rep, class Period>
    [[nodiscard]] result<execution::deadline> deadline_after(std::chrono::duration<Rep, Period> timeout) noexcept {
        if (auto checked = check_context(); !checked) return std::unexpected(checked.error());
        return context_.deadline_after(timeout);
    }
    [[nodiscard]] mcubsp::execution::tick now() const noexcept { return context_.execution().now(); }
    [[nodiscard]] bool initialized() const noexcept { return initialized_; }
    [[nodiscard]] const backend_type& driver() const noexcept { return backend_; }
    // Expert access must preserve ownership/configuration/CS invariants.
    [[nodiscard]] backend_type& unsafe_driver() noexcept { cache_valid_ = false; return backend_; }

    // Retry a failed CS release once all result handles have been consumed.
    // This does not recycle slots retired after failed hardware quiescence.
    [[nodiscard]] result<void> recover_select() noexcept {
        if (auto checked = check_context(); !checked) return checked;
        if (operations_ || backend_.busy()) return std::unexpected(error{errc::busy});
        if (!selected_)
            return {};
        // Controller idle cannot prove that a failed external DMA channel
        // quiesced. Such failures retain the claim and require target recovery.
        if (last_used_dma_)
            return std::unexpected(error{errc::quiesce_failed});
        cache_valid_ = false;
        auto stopped = [&] {
            if constexpr (requires { backend_.recover(); })
                return backend_.recover();
            else
                return backend_.reconfigure(current_timing_,
                                            current_configuration_);
        }();
        if (!stopped)
            return failure(stopped.error());
        if (!select(this, false))
            return failure(driver::spi_error::mmio);
        if constexpr (requires { backend_.recover(); }) {
            configured_ = false;
            cache_valid_ = false;
        } else
            cache_valid_ = true;
        return {};
    }

  private:
    friend device_type;
    template <class, class> friend class operation;
    void retain() noexcept {
        ++operations_;
        context_.retain();
    }
    void drop() noexcept {
        --operations_;
        context_.drop();
    }
    typename Context::execution_type& execution() noexcept { return context_.execution(); }
    [[noreturn]] void lifetime_violation() noexcept {
        context_.execution().fail({"spi", "live resources or failed cleanup during destruction"}); __builtin_trap();
    }
    [[nodiscard]] static std::span<const core::pin_id> bus_pins() noexcept {
        return std::span{board_type::spi_pins}.first(3U);
    }
    [[nodiscard]] static bool valid(driver::spi_configuration config) noexcept {
        return static_cast<unsigned>(config.mode) <= 3U
            && static_cast<unsigned>(config.bit_order) <= 1U
            && (config.frame_width == driver::spi_frame_width::bits8 || config.frame_width == driver::spi_frame_width::bits16)
            && static_cast<unsigned>(config.chip_select) <= 1U;
    }
    [[nodiscard]] result<void> check_context() const noexcept {
        if (context_.execution().in_isr()) return std::unexpected(error{errc::invalid_context});
        if (!initialized_) return std::unexpected(error{errc::not_started});
        if (shutdown_pending_) return std::unexpected(error{errc::invalid_operation});
        return {};
    }
    [[nodiscard]] result<void> check_deadline(mcubsp::execution::deadline limit) const noexcept {
        if (auto checked = check_context(); !checked) return checked;
        if (mcubsp::execution::reached(now(), limit)) return std::unexpected(error{errc::timeout});
        return {};
    }
    [[nodiscard]] result<void> idle() const noexcept {
        if (auto checked = check_context(); !checked) return checked;
        if (operations_ || backend_.busy() || selected_) return std::unexpected(error{errc::busy});
        return {};
    }
    [[nodiscard]] result<void> claim_select(core::pin_id pin, driver::spi_configuration configuration) noexcept {
        const std::array pins{pin};
        if (auto claimed = context_.claim(pins); !claimed) return claimed;
        auto claimed = gpio_.claim(pin, driver::gpio_direction::output);
        if (!claimed) { context_.release(pins); return std::unexpected(to_error(claimed.error())); }
        auto configured = gpio_.configure(pin, {
            .direction = driver::gpio_direction::output,
            .initial_level = configuration.chip_select == driver::spi_chip_select_polarity::active_low
                ? driver::gpio_level::high : driver::gpio_level::low,
        });
        if (!configured) {
            gpio_.release(pin); context_.release(pins);
            return std::unexpected(to_error(configured.error()));
        }
        context_.retain();
        return {};
    }
    [[nodiscard]] result<void> release_select(core::pin_id pin) noexcept {
        auto restored = gpio_.configure(pin, driver::gpio_configuration{});
        if (!restored) return std::unexpected(to_error(restored.error()));
        gpio_.release(pin);
        const std::array pins{pin};
        context_.release(pins); context_.drop();
        return {};
    }
    [[nodiscard]] result<void> release_device(core::pin_id cs) noexcept {
        if (auto checked = idle(); !checked) return checked;
        if (auto released = release_select(cs); !released) return released;
        --devices_;
        if (configured_ && cs == current_cs_) configured_ = false;
        return {};
    }
    [[nodiscard]] result<void> prepare(core::pin_id cs, timing_type timing,
                                       driver::spi_configuration configuration) noexcept {
        if (auto checked = idle(); !checked) return checked;
        if (configured_ && cache_valid_ && cs == current_cs_ && timing == current_timing_
            && configuration.mode == current_configuration_.mode
            && configuration.bit_order == current_configuration_.bit_order
            && configuration.frame_width == current_configuration_.frame_width
            && configuration.chip_select == current_configuration_.chip_select) return {};
        configured_ = false;
        auto changed = backend_.reconfigure(timing, configuration);
        if (!changed) return failure(changed.error());
        current_cs_ = cs; current_timing_ = timing; current_configuration_ = configuration;
        configured_ = true;
        cache_valid_ = true;
        return {};
    }
    [[nodiscard]] result<void> prepare_default() noexcept {
        if (auto checked = idle(); !checked) return checked;
        if (!default_claimed_) {
            if (auto claimed = claim_select(board_type::spi_binding::select_pin, default_configuration_); !claimed) return claimed;
            default_claimed_ = true;
        }
        return prepare(board_type::spi_binding::select_pin, default_timing_, default_configuration_);
    }
    [[nodiscard]] result<transfer_operation> start_device(
        const device_type& device, std::span<const driver::spi_segment> segments) noexcept {
        if (auto prepared = prepare(device.cs_, device.timing_, device.configuration_); !prepared)
            return std::unexpected(prepared.error());
        last_used_dma_ = false;
        return wrap_operation(backend_.transaction(segments), *this);
    }
    template<class Selection>
    [[nodiscard]] result<transfer_operation> start_device_dma(
        const device_type& device, driver::spi_dma_buffers buffers, Selection selection) noexcept {
        if (!selection.receive.valid() || !selection.transmit.valid()) return std::unexpected(error{errc::not_started});
        if (auto prepared = prepare(device.cs_, device.timing_, device.configuration_); !prepared)
            return std::unexpected(prepared.error());
        last_used_dma_ = true;
        return wrap_operation(backend_.transaction_dma(
            buffers, selection.receive.backend(), selection.transmit.backend()), *this);
    }
    [[nodiscard]] result<std::size_t> immediate_device(
        const device_type& device, std::span<const std::uint8_t> transmit, std::span<std::uint8_t> receive,
        mcubsp::execution::deadline limit, std::uint32_t poll_limit) noexcept {
        if (auto checked = check_deadline(limit); !checked) return std::unexpected(checked.error());
        if (auto prepared = prepare(device.cs_, device.timing_, device.configuration_); !prepared)
            return std::unexpected(prepared.error());
        last_used_dma_ = false;
        auto transferred = backend_.transaction_immediate(transmit, receive, poll_limit,
            [&]() noexcept { return mcubsp::execution::reached(now(), limit); });
        if (!transferred) return std::unexpected(to_error(transferred.error()));
        return *transferred;
    }
    [[nodiscard]] static bool select(void* pointer, bool active) noexcept {
        auto& self = *static_cast<spi_bus*>(pointer);
        if (!self.configured_) return !active;
        const bool high = active == (self.current_configuration_.chip_select == driver::spi_chip_select_polarity::active_high);
        const auto result = high ? self.gpio_.set(self.current_cs_) : self.gpio_.clear(self.current_cs_);
        if (result) self.selected_ = active;
        return result.has_value();
    }
    [[nodiscard]] static result<void> failure(driver::spi_error error) noexcept { return std::unexpected(to_error(error)); }

    Context& context_;
    backend_type backend_;
    gpio_backend_type gpio_;
    timing_type default_timing_{};
    driver::spi_configuration default_configuration_{};
    timing_type current_timing_{};
    driver::spi_configuration current_configuration_{};
    core::pin_id current_cs_{};
    std::uint16_t devices_{};
    std::uint16_t operations_{};
    bool initialized_{};
    bool default_claimed_{};
    bool configured_{};
    bool cache_valid_{};
    bool selected_{};
    bool last_used_dma_{};
    bool shutdown_pending_{};
};
} // namespace mcubsp::io
