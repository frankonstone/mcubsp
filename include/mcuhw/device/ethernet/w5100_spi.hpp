#pragma once

// @req-file REQ-0044 REQ-0046
#include <array>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <span>
#include <utility>

#include <mcuhw/core/transfer.hpp>
#include <mcuhw/driver/spi.hpp>
#include <mcuhw/io/core.hpp>
#include <mcuhw/device/ethernet/w5100_types.hpp>

namespace mcuhw::device {

// Adapts the existing asynchronous SPI transaction boundary to the W5100's
// byte-register protocol. DeadlineSource must return a fresh absolute deadline
// from deadline(). The transfer is always consumed before this function
// returns, so the SPI backend has deasserted chip select and released its one
// operation slot on success, failure, and timeout.
// Direct controller adapter: call only from its foreground owner context.
// New compositions should use w5100_io_transport, whose endpoint checks context
// before submitting a transfer and shares deadlines across register accesses.
template<typename Spi, typename DeadlineSource>
class w5100_spi_transport {
public:
    w5100_spi_transport(Spi& spi, DeadlineSource& deadlines) noexcept
        : spi_(spi), deadlines_(deadlines) {}

    [[nodiscard]] std::expected<std::uint8_t, w5100_error>
    read(std::uint16_t address) noexcept {
        const std::array<std::uint16_t, 3U> header{
            read_opcode,
            static_cast<std::uint8_t>(address >> 8U),
            static_cast<std::uint8_t>(address),
        };
        std::array<std::uint16_t, 1U> response{};
        const std::array segments{
            driver::spi_segment{.transmit = header},
            driver::spi_segment{.receive = response, .dummy = 0U},
        };
        const auto transferred = transact(segments, 4U);
        if (!transferred) return std::unexpected(transferred.error());
        return static_cast<std::uint8_t>(response[0]);
    }

    [[nodiscard]] std::expected<void, w5100_error>
    write(std::uint16_t address, std::uint8_t value) noexcept {
        const std::array<std::uint16_t, 4U> command{
            write_opcode,
            static_cast<std::uint8_t>(address >> 8U),
            static_cast<std::uint8_t>(address),
            value,
        };
        const std::array segments{driver::spi_segment{.transmit = command}};
        return transact(segments, command.size());
    }

private:
    [[nodiscard]] std::expected<void, w5100_error>
    transact(std::span<const driver::spi_segment> segments,
             std::size_t expected_frames) noexcept {
        auto started = spi_.transaction(segments);
        if (!started) return std::unexpected(w5100_error::transport);
        auto transfer = std::move(*started);
        const auto waited = transfer.wait_until(deadlines_.deadline());
        if (!waited) {
            const auto kind = waited.error().kind;
            static_cast<void>(transfer.take());
            return std::unexpected(kind == core::transfer_error_kind::timeout
                ? w5100_error::timeout
                : w5100_error::transport);
        }
        const auto completed = transfer.take();
        if (!completed || completed->frames != expected_frames) {
            return std::unexpected(w5100_error::transport);
        }
        return {};
    }

    static constexpr std::uint16_t read_opcode = 0x0FU;
    static constexpr std::uint16_t write_opcode = 0xF0U;

    Spi& spi_;
    DeadlineSource& deadlines_;
};

// Original-W5100 optimized transport. Every call still produces exactly one
// separately chip-selected 32-bit command as required by the W5100 protocol;
// the optimization is confined to using a bounded synchronous controller path
// instead of creating and waiting on an asynchronous operation for each byte.
template<typename Spi>
class w5100_spi_immediate_transport {
public:
    explicit w5100_spi_immediate_transport(
        Spi& spi, std::uint32_t poll_limit = 100'000U) noexcept
        : spi_(spi), poll_limit_(poll_limit) {}

    [[nodiscard]] std::expected<std::uint8_t, w5100_error>
    read(std::uint16_t address) noexcept {
        const std::array<std::uint8_t, 4U> command{
            read_opcode,
            static_cast<std::uint8_t>(address >> 8U),
            static_cast<std::uint8_t>(address),
            0U,
        };
        std::array<std::uint8_t, 4U> response{};
        const auto transferred = spi_.transaction_immediate(
            command, response, poll_limit_);
        if (!transferred || *transferred != command.size()) {
            return std::unexpected(map_error(transferred));
        }
        return response[3];
    }

    [[nodiscard]] std::expected<void, w5100_error>
    write(std::uint16_t address, std::uint8_t value) noexcept {
        const std::array<std::uint8_t, 4U> command{
            write_opcode,
            static_cast<std::uint8_t>(address >> 8U),
            static_cast<std::uint8_t>(address),
            value,
        };
        const auto transferred = spi_.transaction_immediate(
            command, {}, poll_limit_);
        if (!transferred || *transferred != command.size()) {
            return std::unexpected(map_error(transferred));
        }
        return {};
    }

private:
    [[nodiscard]] static w5100_error map_error(
        const std::expected<std::size_t, driver::spi_error>& result) noexcept {
        return !result && result.error() == driver::spi_error::timeout
            ? w5100_error::timeout : w5100_error::transport;
    }

    static constexpr std::uint8_t read_opcode = 0x0FU;
    static constexpr std::uint8_t write_opcode = 0xF0U;

    Spi& spi_;
    std::uint32_t poll_limit_;
};

// Register transport for an io SPI device (or a compatible test transport).
// The caller sets one absolute deadline for a whole initialization, recovery,
// or polling step. Every original-W5100 register uses a separate CS assertion.
// On supported controllers the byte-oriented immediate path avoids creating
// one asynchronous operation for each register, while retaining io checks.
template<typename SpiDevice>
class w5100_io_transport {
public:
    explicit w5100_io_transport(SpiDevice& spi) noexcept : spi_(spi) {}

    void until(execution::deadline limit) noexcept { deadline_ = limit; }

    [[nodiscard]] std::expected<std::uint8_t, w5100_error>
    read(std::uint16_t address) noexcept {
        const std::array<std::uint8_t, 4U> command{
            0x0FU, static_cast<std::uint8_t>(address >> 8U),
            static_cast<std::uint8_t>(address), 0U};
        std::array<std::uint8_t, 4U> response{};
        const auto completed = transact(command, response);
        if (!completed) return std::unexpected(completed.error());
        return response[3];
    }

    [[nodiscard]] std::expected<void, w5100_error>
    write(std::uint16_t address, std::uint8_t value) noexcept {
        const std::array<std::uint8_t, 4U> command{
            0xF0U, static_cast<std::uint8_t>(address >> 8U),
            static_cast<std::uint8_t>(address), value};
        return transact(command, {});
    }

private:
    [[nodiscard]] static w5100_error map_error(io::error error) noexcept {
        if (error.code == io::errc::timeout) return w5100_error::timeout;
        if (error.code == io::errc::cancelled) return w5100_error::cancelled;
        return w5100_error::transport;
    }

    [[nodiscard]] std::expected<void, w5100_error>
    transact(std::span<const std::uint8_t, 4U> command,
             std::span<std::uint8_t> response) noexcept {
        if constexpr (requires { spi_.valid(); }) {
            if (!spi_.valid()) return std::unexpected(w5100_error::transport);
        }
        if (execution::reached(spi_.now(), deadline_)) {
            return std::unexpected(w5100_error::timeout);
        }
        if constexpr (requires {
            spi_.transaction_immediate(command, response, deadline_);
        }) {
            const auto completed = spi_.transaction_immediate(
                command, response, deadline_);
            if (!completed) return std::unexpected(map_error(completed.error()));
            if (*completed != command.size()) {
                return std::unexpected(w5100_error::transport);
            }
        } else {
            const std::array<std::uint16_t, 4U> frames{
                command[0], command[1], command[2], command[3]};
            std::array<std::uint16_t, 4U> incoming{};
            const auto rx = response.empty() ? std::span<std::uint16_t>{}
                                             : std::span{incoming};
            const std::array segments{
                driver::spi_segment{.transmit = frames, .receive = rx}};
            const auto completed = spi_.transaction_until(segments, deadline_);
            if (!completed) return std::unexpected(map_error(completed.error()));
            if (completed->frames != frames.size()) {
                return std::unexpected(w5100_error::transport);
            }
            for (std::size_t index = 0U; index < response.size(); ++index) {
                response[index] = static_cast<std::uint8_t>(incoming[index]);
            }
        }
        return {};
    }

    SpiDevice& spi_;
    execution::deadline deadline_{};
};

} // namespace mcuhw::device
