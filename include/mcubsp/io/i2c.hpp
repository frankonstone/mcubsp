#pragma once

// @req-file REQ-0014 REQ-0018 REQ-0019 REQ-0045 REQ-0046
#include <array>
#include <chrono>
#include <cstdint>
#include <span>

#include <mcubsp/driver/i2c.hpp>
#include <mcubsp/io/core.hpp>
#include <mcubsp/io/i2c_device.hpp>
#include <mcubsp/io/operation.hpp>

namespace mcubsp::io {

// Single foreground owner; no concurrent dispatch of the native backend in ISRs.
template<class Context>
class i2c_bus {
    using board_type = typename Context::board_type;

public:
    using backend_type = typename board_type::template i2c_backend<
        typename Context::mmio_type, typename Context::execution_type>;
    using transfer_operation = operation<typename backend_type::transfer_type, i2c_bus>;
    using device_type = i2c_device<i2c_bus>;
    using error_type = error;
    using deadline_type = execution::deadline;

    explicit i2c_bus(Context& context) noexcept
        : context_(context), backend_(detail::make_backend<backend_type>(context)) {}
    i2c_bus(const i2c_bus&) = delete;
    i2c_bus& operator=(const i2c_bus&) = delete;
    i2c_bus(i2c_bus&&) = delete;
    i2c_bus& operator=(i2c_bus&&) = delete;

    ~i2c_bus() {
        if (initialized_ && !shutdown()) {
            context_.execution().fail({"i2c", "live resources or failed cleanup during destruction"});
            __builtin_trap();
        }
    }

    [[nodiscard]] result<void> begin(std::uint32_t hz = 100'000U) noexcept {
        if (context_.execution().in_isr()) return std::unexpected(error{errc::invalid_context});
        if (!context_.started()) return failure(driver::i2c_error::not_initialized);
        if (initialized_) return failure(driver::i2c_error::already_owned);
        const auto timing = board_type::solve_i2c(hz);
        if (!timing) return failure(timing.error());
        const auto claimed = context_.claim(board_type::i2c_pins);
        if (!claimed)
            return claimed;
        const auto initialized = backend_.initialize(*timing);
        if (!initialized) {
            if constexpr (requires { backend_.needs_cleanup(); })
                if (backend_.needs_cleanup()) {
                    initialized_ = true;
                    context_.retain();
                    return failure(initialized.error());
                }
            context_.release(board_type::i2c_pins);
            return failure(initialized.error());
        }
        initialized_ = true;
        context_.retain();
        return {};
    }

    [[nodiscard]] result<void> shutdown() noexcept {
        if (context_.execution().in_isr())
            return std::unexpected(error{errc::invalid_context});
        if (!initialized_) return {};
        if (operations_ != 0U || has_devices()) return std::unexpected(error{errc::busy});
        const auto stopped = backend_.shutdown();
        if (!stopped) return failure(stopped.error());
        initialized_ = false;
        context_.release(board_type::i2c_pins);
        context_.drop();
        return {};
    }

    void poll() noexcept {
        if (initialized_ && !context_.execution().in_isr())
            board_type::poll_i2c(context_.mmio(), backend_);
    }

    [[nodiscard]] result<device_type> device(std::uint8_t address) noexcept {
        if (context_.execution().in_isr()) return std::unexpected(error{errc::invalid_context});
        if (!initialized_) return std::unexpected(error{errc::not_started});
        const auto validated = driver::make_i2c_address(address);
        if (!validated) return std::unexpected(to_error(validated.error()));
        if (operations_ != 0U || address_claimed(*validated))
            return std::unexpected(error{errc::busy});
        addresses_[address / 32U] |= std::uint32_t{1U} << (address % 32U);
        context_.retain();
        return device_type{*this, *validated};
    }

    template<class Duration>
    [[nodiscard]] result<deadline_type> deadline_after(Duration timeout) noexcept {
        if (!initialized_) return std::unexpected(error{errc::not_started});
        return context_.deadline_after(timeout);
    }
    [[nodiscard]] execution::tick now() const noexcept {
        return context_.execution().now();
    }

    // Addresses are unshifted seven-bit values. Buffers are borrowed until the
    // returned operation is terminal; write_read uses one repeated-START transfer.
    [[nodiscard]] result<transfer_operation> write_async(
        driver::i2c_address address, std::span<const std::uint8_t> bytes) noexcept {
        return submit_write(address, bytes, false);
    }
    [[nodiscard]] result<transfer_operation> read_async(
        driver::i2c_address address, std::span<std::uint8_t> bytes) noexcept {
        return submit_read(address, bytes, false);
    }
    [[nodiscard]] result<transfer_operation> write_read_async(
        driver::i2c_address address, std::span<const std::uint8_t> write_bytes,
        std::span<std::uint8_t> read_bytes) noexcept {
        return submit_write_read(address, write_bytes, read_bytes, false);
    }

    template<class Duration = std::chrono::milliseconds>
    [[nodiscard]] result<driver::i2c_transfer_result> write(
        driver::i2c_address address, std::span<const std::uint8_t> bytes,
        Duration timeout = std::chrono::milliseconds{1000}) noexcept {
        const auto limit = blocking_deadline(timeout);
        if (!limit) return std::unexpected(limit.error());
        auto started = write_async(address, bytes);
        if (!started) return std::unexpected(started.error());
        return started->finish_until(*limit);
    }

    template<class Duration = std::chrono::milliseconds>
    [[nodiscard]] result<driver::i2c_transfer_result> read(
        driver::i2c_address address, std::span<std::uint8_t> bytes,
        Duration timeout = std::chrono::milliseconds{1000}) noexcept {
        const auto limit = blocking_deadline(timeout);
        if (!limit) return std::unexpected(limit.error());
        auto started = read_async(address, bytes);
        if (!started) return std::unexpected(started.error());
        return started->finish_until(*limit);
    }

    template<class Duration = std::chrono::milliseconds>
    [[nodiscard]] result<driver::i2c_transfer_result> write_read(
        driver::i2c_address address, std::span<const std::uint8_t> write_bytes,
        std::span<std::uint8_t> read_bytes,
        Duration timeout = std::chrono::milliseconds{1000}) noexcept {
        const auto limit = blocking_deadline(timeout);
        if (!limit) return std::unexpected(limit.error());
        auto started = write_read_async(address, write_bytes, read_bytes);
        if (!started) return std::unexpected(started.error());
        return started->finish_until(*limit);
    }

    [[nodiscard]] bool initialized() const noexcept { return initialized_; }
    [[nodiscard]] const backend_type& driver() const noexcept { return backend_; }
    // Borrowed same-backend access; bypasses facade lifecycle guarantees.
    [[nodiscard]] backend_type& unsafe_driver() noexcept { return backend_; }

private:
    friend device_type;
    template<class, class> friend class operation;

    // Operation handles point to this immovable bus, never to a movable endpoint.
    void retain() noexcept { ++operations_; context_.retain(); }
    void drop() noexcept { --operations_; context_.drop(); }
    auto& execution() noexcept { return context_.execution(); }
    const auto& execution() const noexcept { return context_.execution(); }
    [[noreturn]] void ownership_violation() noexcept {
        context_.execution().fail({"i2c", "live resources or failed cleanup during destruction"});
        __builtin_trap();
    }
    [[nodiscard]] bool has_devices() const noexcept {
        for (const auto word : addresses_) if (word != 0U) return true;
        return false;
    }
    [[nodiscard]] bool address_claimed(driver::i2c_address address) const noexcept {
        return (addresses_[address.value / 32U]
                & (std::uint32_t{1U} << (address.value % 32U))) != 0U;
    }
    [[nodiscard]] result<void> release_device(driver::i2c_address address) noexcept {
        if (context_.execution().in_isr()) return std::unexpected(error{errc::invalid_context});
        if (operations_ != 0U) return std::unexpected(error{errc::busy});
        addresses_[address.value / 32U] &= ~(std::uint32_t{1U} << (address.value % 32U));
        context_.drop();
        return {};
    }
    [[nodiscard]] result<void> authorize(driver::i2c_address address, bool leased) noexcept {
        if (context_.execution().in_isr()) return std::unexpected(error{errc::invalid_context});
        if (!initialized_) return std::unexpected(error{errc::not_started});
        const auto valid = driver::make_i2c_address(address.value);
        if (!valid) return std::unexpected(to_error(valid.error()));
        if (address_claimed(address) != leased) return std::unexpected(error{errc::busy});
        return {};
    }
    [[nodiscard]] result<transfer_operation> submit_write(
        driver::i2c_address address, std::span<const std::uint8_t> bytes,
        bool leased) noexcept {
        const auto authorized = authorize(address, leased);
        if (!authorized) return std::unexpected(authorized.error());
        return wrap_operation(backend_.write(address, bytes), *this);
    }
    [[nodiscard]] result<transfer_operation> submit_read(
        driver::i2c_address address, std::span<std::uint8_t> bytes,
        bool leased) noexcept {
        const auto authorized = authorize(address, leased);
        if (!authorized) return std::unexpected(authorized.error());
        return wrap_operation(backend_.read(address, bytes), *this);
    }
    [[nodiscard]] result<transfer_operation> submit_write_read(
        driver::i2c_address address, std::span<const std::uint8_t> output,
        std::span<std::uint8_t> input, bool leased) noexcept {
        const auto authorized = authorize(address, leased);
        if (!authorized) return std::unexpected(authorized.error());
        return wrap_operation(backend_.write_read(address, output, input), *this);
    }
    [[nodiscard]] result<void> check_deadline(deadline_type limit) noexcept {
        if (context_.execution().in_isr()) return std::unexpected(error{errc::invalid_context});
        if (!initialized_) return std::unexpected(error{errc::not_started});
        if (mcubsp::execution::reached(now(), limit)) return std::unexpected(error{errc::timeout});
        return {};
    }

    [[nodiscard]] static result<void> failure(driver::i2c_error cause) noexcept {
        return std::unexpected(to_error(cause));
    }

    template<class Duration>
    [[nodiscard]] result<execution::deadline>
    blocking_deadline(Duration timeout) noexcept {
        if (context_.execution().in_isr()) {
            return std::unexpected(to_error(core::transfer_error<driver::i2c_error>{
                .kind = core::transfer_error_kind::wait_from_isr}));
        }
        return context_.deadline_after(timeout);
    }

    Context& context_;
    backend_type backend_;
    std::array<std::uint32_t, 4U> addresses_{};
    std::uint16_t operations_{};
    bool initialized_{};
};

} // namespace mcubsp::io
