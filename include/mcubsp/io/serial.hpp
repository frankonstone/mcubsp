#pragma once

// @req-file REQ-0014 REQ-0018 REQ-0019 REQ-0045
#include <chrono>
#include <cstdint>
#include <span>
#include <string_view>
#include <utility>

#include <mcubsp/driver/uart.hpp>
#include <mcubsp/io/core.hpp>
#include <mcubsp/io/operation.hpp>

namespace mcubsp::io {

// Single foreground owner; do not concurrently dispatch this polled backend
// from an ISR. This object/context outlive every operation, and asynchronous
// calls borrow the caller's buffer through completion.
template<class Context>
class serial_port {
    using board_type = typename Context::board_type;

public:
    using backend_type = typename board_type::template serial_backend<
        typename Context::mmio_type, typename Context::execution_type>;
    using write_operation = operation<typename backend_type::tx_transfer, Context>;
    using read_operation = operation<typename backend_type::rx_transfer, Context>;

    explicit serial_port(Context& context) noexcept
        : context_(context), backend_(detail::make_backend<backend_type>(context)) {}
    serial_port(const serial_port&) = delete;
    serial_port& operator=(const serial_port&) = delete;
    serial_port(serial_port&&) = delete;
    serial_port& operator=(serial_port&&) = delete;

    ~serial_port() {
        if (initialized_ && !shutdown()) {
            context_.execution().fail({"serial", "live operation or failed cleanup during destruction"});
            __builtin_trap();
        }
    }

    [[nodiscard]] result<void> begin(
        std::uint32_t baud = 115'200U, driver::uart_format format = {}) noexcept {
        if (context_.execution().in_isr()) return std::unexpected(error{errc::invalid_context});
        if (!context_.started()) return failure(driver::uart_error::not_initialized);
        if (initialized_) return failure(driver::uart_error::already_owned);
        const auto timing = board_type::solve_serial(baud);
        if (!timing) return failure(timing.error());
        const auto claimed = context_.claim(board_type::serial_pins);
        if (!claimed)
            return claimed;
        const auto initialized = backend_.initialize(*timing, format);
        if (!initialized) {
            if constexpr (requires { backend_.needs_cleanup(); })
                if (backend_.needs_cleanup()) {
                    initialized_ = true;
                    context_.retain();
                    return failure(initialized.error());
                }
            context_.release(board_type::serial_pins);
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
        context_.release(board_type::serial_pins);
        context_.drop();
        return {};
    }

    void poll() noexcept {
        if (initialized_ && !context_.execution().in_isr())
            board_type::poll_serial(context_.mmio(), backend_);
    }

    [[nodiscard]] result<write_operation>
    write_async(std::span<const std::uint8_t> bytes) noexcept {
        if (context_.execution().in_isr()) return std::unexpected(error{errc::invalid_context});
        return wrap_operation(backend_.transmit(bytes), context_);
    }

    [[nodiscard]] result<read_operation>
    read_async(std::span<std::uint8_t> bytes) noexcept {
        if (context_.execution().in_isr())
            return std::unexpected(error{errc::invalid_context});
        return wrap_operation(backend_.receive(bytes), context_);
    }

    template <class B = backend_type>
        requires requires(B &b,
                          std::span<const std::span<std::uint8_t>> buffers) {
            b.receive_stream(buffers, std::size_t{});
        }
    [[nodiscard]] result<read_operation>
    read_stream_async(std::span<const std::span<std::uint8_t>> buffers,
                      std::size_t maximum_bytes) noexcept {
        if (context_.execution().in_isr())
            return std::unexpected(error{errc::invalid_context});
        return wrap_operation(backend_.receive_stream(buffers, maximum_bytes),
                              context_);
    }
    template <class B = backend_type>
        requires requires(B &b, std::span<std::uint8_t> bytes) {
            b.queue_receive_buffer(bytes);
        }
    [[nodiscard]] result<void>
    queue_read_buffer(std::span<std::uint8_t> bytes) noexcept {
        if (context_.execution().in_isr())
            return std::unexpected(error{errc::invalid_context});
        auto r = backend_.queue_receive_buffer(bytes);
        if (!r)
            return failure(r.error());
        return {};
    }
    template <class B = backend_type>
        requires requires(B &b) { b.take_received_block(); }
    [[nodiscard]] auto take_read_block() noexcept {
        using R = decltype(backend_.take_received_block());
        if (context_.execution().in_isr())
            return R{};
        return backend_.take_received_block();
    }
    template <class B = backend_type>
        requires requires(B &b) { b.stop_receive(); }
    [[nodiscard]] result<void> stop_read() noexcept {
        if (context_.execution().in_isr())
            return std::unexpected(error{errc::invalid_context});
        auto r = backend_.stop_receive();
        if (!r)
            return failure(r.error());
        return {};
    }
    template <class Duration = std::chrono::milliseconds>
        requires requires(Duration value) {
            std::chrono::duration_cast<std::chrono::nanoseconds>(value);
        }
    [[nodiscard]] result<void>
    write(std::span<const std::uint8_t> bytes,
          Duration timeout = std::chrono::milliseconds{1000}) noexcept {
        const auto limit = blocking_deadline(timeout);
        if (!limit) return std::unexpected(limit.error());
        return write_until(bytes, *limit);
    }

    template<class Duration = std::chrono::milliseconds>
        requires requires(Duration value) {
            std::chrono::duration_cast<std::chrono::nanoseconds>(value);
        }
    [[nodiscard]] result<void> write(
        std::string_view text,
        Duration timeout = std::chrono::milliseconds{1000}) noexcept {
        return write(as_bytes(text), timeout);
    }

    template<class Duration = std::chrono::milliseconds>
        requires requires(Duration value) {
            std::chrono::duration_cast<std::chrono::nanoseconds>(value);
        }
    [[nodiscard]] result<std::size_t> read(
        std::span<std::uint8_t> bytes,
        Duration timeout = std::chrono::milliseconds{1000}) noexcept {
        const auto limit = blocking_deadline(timeout);
        if (!limit) return std::unexpected(limit.error());
        auto started = read_async(bytes);
        if (!started) return std::unexpected(started.error());
        return started->finish_until(*limit);
    }

    // Two writes share one overall deadline. There is no hidden line buffer
    // or atomic multi-writer guarantee; applications serialize shared writers.
    template<class Duration = std::chrono::milliseconds>
    [[nodiscard]] result<void> println(
        std::string_view text = {},
        Duration timeout = std::chrono::milliseconds{1000}) noexcept {
        const auto limit = blocking_deadline(timeout);
        if (!limit) return std::unexpected(limit.error());
        if (!text.empty()) {
            const auto written = write_until(as_bytes(text), *limit);
            if (!written) return written;
        }
        std::uint8_t newline[]{'\r', '\n'};
        return write_until(newline, *limit);
    }

    template<class DmaSelection>
        requires requires(backend_type& backend, DmaSelection selection,
                          std::span<const std::uint8_t> bytes) {
            backend.transmit_dma(bytes, selection.channel.backend());
        }
    [[nodiscard]] result<write_operation> write_async(
        std::span<const std::uint8_t> bytes, DmaSelection selection) noexcept {
        if (context_.execution().in_isr()) return std::unexpected(error{errc::invalid_context});
        if (!selection.channel.valid())
            return std::unexpected(error{errc::not_started});
        return wrap_operation(
            backend_.transmit_dma(bytes, selection.channel.backend()), context_);
    }

    template<class DmaSelection>
        requires requires(backend_type& backend, DmaSelection selection,
                          std::span<std::uint8_t> bytes) {
            backend.receive_dma(bytes, selection.channel.backend());
        }
    [[nodiscard]] result<read_operation> read_async(
        std::span<std::uint8_t> bytes, DmaSelection selection) noexcept {
        if (context_.execution().in_isr()) return std::unexpected(error{errc::invalid_context});
        if (!selection.channel.valid())
            return std::unexpected(error{errc::not_started});
        return wrap_operation(
            backend_.receive_dma(bytes, selection.channel.backend()), context_);
    }

    template<class DmaSelection, class Duration = std::chrono::milliseconds>
        requires requires(backend_type& backend, DmaSelection selection,
                          std::span<const std::uint8_t> bytes) {
            backend.transmit_dma(bytes, selection.channel.backend());
        }
    [[nodiscard]] result<void> write(
        std::span<const std::uint8_t> bytes, DmaSelection selection,
        Duration timeout = std::chrono::milliseconds{1000}) noexcept {
        const auto limit = blocking_deadline(timeout);
        if (!limit) return std::unexpected(limit.error());
        auto started = write_async(bytes, selection);
        if (!started) return std::unexpected(started.error());
        return started->finish_until(*limit);
    }

    template<class DmaSelection, class Duration = std::chrono::milliseconds>
        requires requires(backend_type& backend, DmaSelection selection,
                          std::span<std::uint8_t> bytes) {
            backend.receive_dma(bytes, selection.channel.backend());
        }
    [[nodiscard]] result<std::size_t> read(
        std::span<std::uint8_t> bytes, DmaSelection selection,
        Duration timeout = std::chrono::milliseconds{1000}) noexcept {
        const auto limit = blocking_deadline(timeout);
        if (!limit) return std::unexpected(limit.error());
        auto started = read_async(bytes, selection);
        if (!started) return std::unexpected(started.error());
        return started->finish_until(*limit);
    }

    [[nodiscard]] bool initialized() const noexcept { return initialized_; }

    [[nodiscard]] const backend_type& driver() const noexcept { return backend_; }
    // Borrowed expert escape hatch. Direct initialization, shutdown or pin
    // reconfiguration bypasses facade bookkeeping and its ownership guarantees.
    [[nodiscard]] backend_type& unsafe_driver() noexcept { return backend_; }

private:
    [[nodiscard]] static result<void> failure(driver::uart_error cause) noexcept {
        return std::unexpected(to_error(cause));
    }

    [[nodiscard]] static std::span<const std::uint8_t>
    as_bytes(std::string_view text) noexcept {
        return {reinterpret_cast<const std::uint8_t*>(text.data()), text.size()};
    }

    template<class Duration>
    [[nodiscard]] result<execution::deadline>
    blocking_deadline(Duration timeout) noexcept {
        if (context_.execution().in_isr()) {
            return std::unexpected(to_error(core::transfer_error<driver::uart_error>{
                .kind = core::transfer_error_kind::wait_from_isr}));
        }
        return context_.deadline_after(timeout);
    }

    [[nodiscard]] result<void> write_until(
        std::span<const std::uint8_t> bytes, execution::deadline limit) noexcept {
        auto started = write_async(bytes);
        if (!started) return std::unexpected(started.error());
        return started->finish_until(limit);
    }

    Context& context_;
    backend_type backend_;
    bool initialized_{};
};

} // namespace mcubsp::io
