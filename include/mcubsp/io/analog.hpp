#pragma once

// @req-file REQ-0014 REQ-0018 REQ-0019 REQ-0045
#include <chrono>
#include <cstdint>
#include <span>

#include <mcubsp/driver/adc.hpp>
#include <mcubsp/io/core.hpp>
#include <mcubsp/io/operation.hpp>

namespace mcubsp::io {

// The first board profile selects one physical analog input. A native scan
// samples that input once; explicit DMA sampling can fill a longer buffer.
// Single foreground owner; no concurrent dispatch of the native backend in ISRs.
template<class Context>
class analog_input {
    using board_type = typename Context::board_type;
    static_assert(board_type::adc_channels.size() == 1U);

public:
    using backend_type = typename board_type::template adc_backend<
        typename Context::mmio_type, typename Context::execution_type>;
    using sample_operation = operation<typename backend_type::transfer_type, Context>;

    explicit analog_input(Context& context) noexcept
        : context_(context), backend_(context.mmio(), context.execution()) {}
    analog_input(const analog_input&) = delete;
    analog_input& operator=(const analog_input&) = delete;
    analog_input(analog_input&&) = delete;
    analog_input& operator=(analog_input&&) = delete;

    ~analog_input() {
        if (initialized_ && !shutdown()) {
            context_.execution().active_transfer_destroyed();
            __builtin_trap();
        }
    }

    [[nodiscard]] result<void> begin(driver::adc_configuration configuration = {}) noexcept {
        if (context_.execution().in_isr()) return std::unexpected(error{errc::invalid_context});
        if (!context_.started()) return failure(driver::adc_error::not_initialized);
        if (initialized_) return failure(driver::adc_error::already_owned);
        const auto claimed = context_.claim(board_type::adc_pins);
        if (!claimed) return claimed;
        const auto initialized = backend_.initialize(configuration);
        if (!initialized) {
            context_.release(board_type::adc_pins);
            return failure(initialized.error());
        }
        initialized_ = true;
        context_.retain();
        return {};
    }

    [[nodiscard]] result<void> shutdown() noexcept {
        if (context_.execution().in_isr()) return std::unexpected(error{errc::invalid_context});
        if (!initialized_) return {};
        const auto stopped = backend_.shutdown();
        if (!stopped) return failure(stopped.error());
        initialized_ = false;
        context_.release(board_type::adc_pins);
        context_.drop();
        return {};
    }

    void poll() noexcept {
        if (initialized_ && !context_.execution().in_isr())
            board_type::poll_adc(context_.mmio(), backend_);
    }

    // The output span must have one element and remain alive until completion.
    [[nodiscard]] result<sample_operation>
    sample_async(std::span<std::uint16_t> samples) noexcept {
        if (context_.execution().in_isr()) return std::unexpected(error{errc::invalid_context});
        return wrap_operation(backend_.convert(board_type::adc_channels, samples), context_);
    }

    template<class Duration = std::chrono::milliseconds>
    [[nodiscard]] result<std::uint16_t> read(
        Duration timeout = std::chrono::milliseconds{1000}) noexcept {
        const auto limit = context_.deadline_after(timeout);
        if (!limit) return std::unexpected(limit.error());
        std::uint16_t value{};
        auto started = sample_async(std::span{&value, 1U});
        if (!started) return std::unexpected(started.error());
        const auto completed = started->finish_until(*limit);
        if (!completed) return std::unexpected(completed.error());
        return value;
    }

    template<class Duration = std::chrono::milliseconds>
        requires requires(Duration value) {
            std::chrono::duration_cast<std::chrono::nanoseconds>(value);
        }
    [[nodiscard]] result<driver::adc_sequence_result> sample(
        std::span<std::uint16_t> samples,
        Duration timeout = std::chrono::milliseconds{1000}) noexcept {
        const auto limit = context_.deadline_after(timeout);
        if (!limit) return std::unexpected(limit.error());
        auto started = sample_async(samples);
        if (!started) return std::unexpected(started.error());
        return started->finish_until(*limit);
    }

    template<class DmaSelection>
        requires requires(backend_type& backend, DmaSelection selection,
                          std::span<std::uint16_t> samples) {
            backend.sample_dma(board_type::adc_channels[0], samples,
                               selection.channel.backend());
        }
    [[nodiscard]] result<sample_operation> sample_async(
        std::span<std::uint16_t> samples, DmaSelection selection) noexcept {
        if (context_.execution().in_isr()) return std::unexpected(error{errc::invalid_context});
        if (!selection.channel.valid())
            return std::unexpected(error{errc::not_started});
        return wrap_operation(backend_.sample_dma(board_type::adc_channels[0],
                              samples, selection.channel.backend()), context_);
    }

    template<class DmaSelection, class Duration = std::chrono::milliseconds>
        requires requires(backend_type& backend, DmaSelection selection,
                          std::span<std::uint16_t> samples) {
            backend.sample_dma(board_type::adc_channels[0], samples,
                               selection.channel.backend());
        }
    [[nodiscard]] result<driver::adc_sequence_result> sample(
        std::span<std::uint16_t> samples, DmaSelection selection,
        Duration timeout = std::chrono::milliseconds{1000}) noexcept {
        const auto limit = context_.deadline_after(timeout);
        if (!limit) return std::unexpected(limit.error());
        auto started = sample_async(samples, selection);
        if (!started) return std::unexpected(started.error());
        return started->finish_until(*limit);
    }

    [[nodiscard]] bool initialized() const noexcept { return initialized_; }
    [[nodiscard]] const backend_type& driver() const noexcept { return backend_; }
    // Borrowed same-backend access; bypasses facade lifecycle guarantees.
    [[nodiscard]] backend_type& unsafe_driver() noexcept { return backend_; }

private:
    [[nodiscard]] static result<void> failure(driver::adc_error cause) noexcept {
        return std::unexpected(to_error(cause));
    }

    Context& context_;
    backend_type backend_;
    bool initialized_{};
};

} // namespace mcubsp::io
