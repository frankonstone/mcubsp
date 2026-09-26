#pragma once

// @req-file REQ-0014 REQ-0018 REQ-0019 REQ-0045 REQ-0046
#include <chrono>
#include <cstdint>
#include <span>
#include <utility>
#include <mcuhw/io/core.hpp>

namespace mcuhw::io {

// A move-only claim on one unshifted seven-bit address. The bus and board must
// outlive the endpoint. No transfer state points at this movable object.
template<class Bus>
class i2c_device {
public:
    using error_type = error;
    using deadline_type = execution::deadline;
    using transfer_operation = typename Bus::transfer_operation;
    using transfer_result = driver::i2c_transfer_result;

    i2c_device(const i2c_device&) = delete;
    i2c_device& operator=(const i2c_device&) = delete;
    i2c_device(i2c_device&& other) noexcept
        : bus_(std::exchange(other.bus_, nullptr)), address_(other.address_) {}
    i2c_device& operator=(i2c_device&&) = delete;
    ~i2c_device() {
        if (bus_ && !release()) bus_->ownership_violation();
    }

    [[nodiscard]] bool valid() const noexcept { return bus_ != nullptr; }
    [[nodiscard]] std::uint8_t address() const noexcept { return address_.value; }
    // Conservatively retains the lease while any bus operation is outstanding,
    // including a completed result that has not yet been consumed.
    [[nodiscard]] result<void> release() noexcept {
        if (!bus_) return {};
        const auto released = bus_->release_device(address_);
        if (released) bus_ = nullptr;
        return released;
    }

    template<class Duration>
    [[nodiscard]] result<deadline_type> deadline_after(Duration timeout) noexcept {
        if (!bus_) return std::unexpected(error{errc::not_started});
        return bus_->deadline_after(timeout);
    }
    [[nodiscard]] execution::tick now() const noexcept {
        if (!bus_) __builtin_trap();
        return bus_->now();
    }

    [[nodiscard]] result<transfer_operation> write_async(
        std::span<const std::uint8_t> bytes) noexcept {
        if (!bus_) return std::unexpected(error{errc::not_started});
        return bus_->submit_write(address_, bytes, true);
    }
    [[nodiscard]] result<transfer_operation> read_async(
        std::span<std::uint8_t> bytes) noexcept {
        if (!bus_) return std::unexpected(error{errc::not_started});
        return bus_->submit_read(address_, bytes, true);
    }
    [[nodiscard]] result<transfer_operation> write_read_async(
        std::span<const std::uint8_t> output,
        std::span<std::uint8_t> input) noexcept {
        if (!bus_) return std::unexpected(error{errc::not_started});
        return bus_->submit_write_read(address_, output, input, true);
    }

    [[nodiscard]] result<transfer_result> write_until(
        std::span<const std::uint8_t> bytes, deadline_type limit) noexcept {
        const auto checked = check_deadline(limit);
        if (!checked) return std::unexpected(checked.error());
        auto started = write_async(bytes);
        if (!started) return std::unexpected(started.error());
        return started->finish_until(limit);
    }
    [[nodiscard]] result<transfer_result> read_until(
        std::span<std::uint8_t> bytes, deadline_type limit) noexcept {
        const auto checked = check_deadline(limit);
        if (!checked) return std::unexpected(checked.error());
        auto started = read_async(bytes);
        if (!started) return std::unexpected(started.error());
        return started->finish_until(limit);
    }
    [[nodiscard]] result<transfer_result> write_read_until(
        std::span<const std::uint8_t> output, std::span<std::uint8_t> input,
        deadline_type limit) noexcept {
        const auto checked = check_deadline(limit);
        if (!checked) return std::unexpected(checked.error());
        auto started = write_read_async(output, input);
        if (!started) return std::unexpected(started.error());
        return started->finish_until(limit);
    }

    template<class Duration = std::chrono::milliseconds>
    [[nodiscard]] result<transfer_result> write(
        std::span<const std::uint8_t> bytes,
        Duration timeout = std::chrono::milliseconds{1000}) noexcept {
        const auto limit = deadline_after(timeout);
        if (!limit) return std::unexpected(limit.error());
        return write_until(bytes, *limit);
    }
    template<class Duration = std::chrono::milliseconds>
    [[nodiscard]] result<transfer_result> read(
        std::span<std::uint8_t> bytes,
        Duration timeout = std::chrono::milliseconds{1000}) noexcept {
        const auto limit = deadline_after(timeout);
        if (!limit) return std::unexpected(limit.error());
        return read_until(bytes, *limit);
    }
    template<class Duration = std::chrono::milliseconds>
    [[nodiscard]] result<transfer_result> write_read(
        std::span<const std::uint8_t> output, std::span<std::uint8_t> input,
        Duration timeout = std::chrono::milliseconds{1000}) noexcept {
        const auto limit = deadline_after(timeout);
        if (!limit) return std::unexpected(limit.error());
        return write_read_until(output, input, *limit);
    }

private:
    friend Bus;
    i2c_device(Bus& bus, driver::i2c_address address) noexcept
        : bus_(&bus), address_(address) {}
    [[nodiscard]] result<void> check_deadline(deadline_type limit) noexcept {
        if (!bus_) return std::unexpected(error{errc::not_started});
        return bus_->check_deadline(limit);
    }
    Bus* bus_;
    driver::i2c_address address_;
};

} // namespace mcuhw::io
