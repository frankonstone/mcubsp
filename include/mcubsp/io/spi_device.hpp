#pragma once

// @req-file REQ-0014 REQ-0018 REQ-0028 REQ-0046
#include <chrono>
#include <cstdint>
#include <span>
#include <utility>
#include <mcubsp/io/core.hpp>

namespace mcubsp::io {
using spi_mode = driver::spi_mode;

struct spi_device_configuration {
    core::pin_id cs{};
    std::uint32_t frequency{1'000'000U};
    spi_mode mode{spi_mode::mode0};
    driver::spi_bit_order bit_order{driver::spi_bit_order::most_significant_first};
    driver::spi_frame_width frame_width{driver::spi_frame_width::bits8};
    driver::spi_chip_select_polarity chip_select{driver::spi_chip_select_polarity::active_low};
};

// Owns a CS claim and borrows a stable controller. Completion storage remains
// in that controller, so moving this handle never moves hardware-owned state.
template<class Bus>
class spi_device {
public:
    using transfer_operation = typename Bus::transfer_operation;
    using backend_type = typename Bus::backend_type;
    using timing_type = typename Bus::timing_type;
    spi_device(const spi_device&) = delete;
    spi_device& operator=(const spi_device&) = delete;
    spi_device(spi_device&& other) noexcept
        : bus_(std::exchange(other.bus_, nullptr)), cs_(other.cs_),
          timing_(other.timing_), configuration_(other.configuration_) {}
    spi_device& operator=(spi_device&&) = delete;
    ~spi_device() { if (bus_ && !release()) bus_->lifetime_violation(); }

    [[nodiscard]] bool valid() const noexcept { return bus_ != nullptr; }
    [[nodiscard]] core::pin_id chip_select() const noexcept { return cs_; }
    [[nodiscard]] std::uint32_t frequency() const noexcept { return timing_.achieved_hz; }
    [[nodiscard]] driver::spi_configuration configuration() const noexcept { return configuration_; }
    [[nodiscard]] result<void> release() noexcept {
        if (!bus_) return {};
        auto result = bus_->release_device(cs_);
        if (result) bus_ = nullptr;
        return result;
    }
    template<class Rep, class Period>
    [[nodiscard]] result<execution::deadline> deadline_after(
        std::chrono::duration<Rep, Period> timeout) noexcept {
        if (!bus_) return std::unexpected(error{errc::not_started});
        return bus_->deadline_after(timeout);
    }
    [[nodiscard]] execution::tick now() const noexcept {
        if (!bus_) __builtin_trap();
        return bus_->now();
    }
    [[nodiscard]] result<transfer_operation> transaction_async(
        std::span<const driver::spi_segment> segments) noexcept {
        if (!bus_) return std::unexpected(error{errc::not_started});
        return bus_->start_device(*this, segments);
    }
    [[nodiscard]] result<driver::spi_transfer_result> transaction_until(
        std::span<const driver::spi_segment> segments, execution::deadline limit) noexcept {
        if (!bus_) return std::unexpected(error{errc::not_started});
        if (auto checked = bus_->check_deadline(limit); !checked)
            return std::unexpected(checked.error());
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
    // A checked fast path, available only when the controller supports it.
    // The deadline is checked inside status waits as well as before starting.
    [[nodiscard]] result<std::size_t> transaction_immediate(
        std::span<const std::uint8_t> transmit, std::span<std::uint8_t> receive,
        execution::deadline limit, std::uint32_t poll_limit = 100'000U) noexcept
        requires requires(backend_type& backend) {
            backend.transaction_immediate(transmit, receive, poll_limit);
        } {
        if (!bus_) return std::unexpected(error{errc::not_started});
        return bus_->immediate_device(*this, transmit, receive, limit, poll_limit);
    }
    template<class Selection>
        requires requires(backend_type& backend, Selection selection, driver::spi_dma_buffers buffers) {
            backend.transaction_dma(buffers, selection.receive.backend(), selection.transmit.backend());
        }
    [[nodiscard]] result<transfer_operation> transfer_async(
        driver::spi_dma_buffers buffers, Selection selection) noexcept {
        if (!bus_) return std::unexpected(error{errc::not_started});
        return bus_->start_device_dma(*this, buffers, selection);
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
        if (auto checked = bus_->check_deadline(*limit); !checked)
            return std::unexpected(checked.error());
        auto started = transfer_async(buffers, selection);
        if (!started) return std::unexpected(started.error());
        return started->finish_until(*limit);
    }
private:
    friend Bus;
    spi_device(Bus& bus, core::pin_id cs, timing_type timing,
               driver::spi_configuration configuration) noexcept
        : bus_(&bus), cs_(cs), timing_(timing), configuration_(configuration) {}
    Bus* bus_;
    core::pin_id cs_;
    timing_type timing_;
    driver::spi_configuration configuration_;
};
} // namespace mcubsp::io
