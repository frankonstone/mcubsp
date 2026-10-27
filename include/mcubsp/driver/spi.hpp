#pragma once

#include <cstddef>
#include <cstdint>
#include <expected>
#include <span>

namespace mcubsp::driver {

enum class spi_error : std::uint8_t {
    invalid_clock,
    invalid_rate,
    unattainable_tolerance,
    invalid_frame,
    empty_transfer,
    mismatched_segment,
    excessive_length,
    busy,
    overrun,
    mode_fault,
    parity,
    transport,
    not_initialized,
    already_owned,
    dma,
    mmio,
    timeout,
};

enum class spi_mode : std::uint8_t { mode0, mode1, mode2, mode3 };
enum class spi_bit_order : std::uint8_t { most_significant_first,
                                           least_significant_first };
enum class spi_frame_width : std::uint8_t { bits8 = 8, bits16 = 16 };
enum class spi_chip_select_polarity : std::uint8_t { active_low, active_high };

struct spi_timing_request {
    std::uint32_t clock_hz;
    std::uint32_t target_hz;
    std::uint32_t maximum_error_ppm;
};

// Vendor-independent timing report. Native configuration is owned by the backend.
struct spi_timing {
    std::uint32_t clock_hz;
    std::uint32_t target_hz;
    std::uint32_t achieved_hz;
    std::uint32_t error_ppm;

    friend constexpr bool operator==(spi_timing, spi_timing) = default;
};

struct spi_configuration {
    spi_mode mode{spi_mode::mode0};
    spi_bit_order bit_order{spi_bit_order::most_significant_first};
    spi_frame_width frame_width{spi_frame_width::bits8};
    spi_chip_select_polarity chip_select{spi_chip_select_polarity::active_low};
};

// Each segment owns zero or more transmit and receive frames. When both are
// present their lengths must match. An empty transmit span generates dummy
// frames; an empty receive span discards received frames. Chip select remains
// asserted between every segment in one transaction.
struct spi_segment {
    std::span<const std::uint16_t> transmit{};
    std::span<std::uint16_t> receive{};
    std::uint16_t dummy{0xFFFFU};
    std::span<const std::uint8_t> transmit_bytes{};
    std::span<std::uint8_t> receive_bytes{};

    [[nodiscard]] std::size_t transmit_size() const noexcept {
        return transmit.empty() ? transmit_bytes.size() : transmit.size();
    }
    [[nodiscard]] std::size_t receive_size() const noexcept {
        return receive.empty() ? receive_bytes.size() : receive.size();
    }
    [[nodiscard]] std::size_t size() const noexcept {
        return transmit_size() ? transmit_size() : receive_size();
    }
    [[nodiscard]] std::uint16_t frame(std::size_t index) const noexcept {
        if (!transmit.empty()) return transmit[index];
        return transmit_bytes.empty() ? dummy : transmit_bytes[index];
    }
    void store(std::size_t index, std::uint16_t value) const noexcept {
        if (!receive.empty()) receive[index] = value;
        else if (!receive_bytes.empty()) receive_bytes[index] = static_cast<std::uint8_t>(value);
    }
    [[nodiscard]] std::expected<std::size_t, spi_error> validate(spi_frame_width width) const noexcept {
        const bool bytes = !transmit_bytes.empty() || !receive_bytes.empty();
        if (bytes && (width != spi_frame_width::bits8 || !transmit.empty() || !receive.empty()))
            return std::unexpected(spi_error::invalid_frame);
        if (size() == 0U) return std::unexpected(spi_error::empty_transfer);
        if (transmit_size() && receive_size() && transmit_size() != receive_size())
            return std::unexpected(spi_error::mismatched_segment);
        if (width == spi_frame_width::bits8) {
            if (!transmit_size() && dummy > 0xFFU) return std::unexpected(spi_error::invalid_frame);
            for (auto value : transmit) if (value > 0xFFU) return std::unexpected(spi_error::invalid_frame);
        }
        return size();
    }
};

[[nodiscard]] inline spi_segment byte_segment(std::span<const std::uint8_t> transmit,
                                             std::span<std::uint8_t> receive,
                                             std::uint8_t dummy = 0xFFU) noexcept {
    return {.dummy = dummy, .transmit_bytes = transmit, .receive_bytes = receive};
}

// Explicit full-duplex DMA mode. Both spans must have the same non-zero
// length, and the controller must be configured for 8-bit frames.
struct spi_dma_buffers {
    std::span<const std::uint8_t> transmit{};
    std::span<std::uint8_t> receive{};
};

struct spi_transfer_result {
    std::size_t frames;

    friend constexpr bool operator==(spi_transfer_result,
                                     spi_transfer_result) = default;
};

template<typename Backend>
class spi_controller {
public:
    explicit spi_controller(Backend& backend) noexcept : backend_(backend) {}

    template<typename Timing>
        requires requires(Backend& backend, Timing timing) { backend.initialize(timing, spi_configuration{}); }
    [[nodiscard]] std::expected<void, spi_error>
    initialize(Timing timing, spi_configuration configuration = {}) noexcept {
        return backend_.initialize(timing, configuration);
    }

    [[nodiscard]] auto transaction(std::span<const spi_segment> segments) noexcept {
        return backend_.transaction(segments);
    }

    template<typename ReceiveDma, typename TransmitDma>
    [[nodiscard]] auto transaction_dma(spi_dma_buffers buffers,
                                       ReceiveDma& receive_dma,
                                       TransmitDma& transmit_dma) noexcept {
        return backend_.transaction_dma(buffers, receive_dma, transmit_dma);
    }

    [[nodiscard]] std::expected<void, spi_error> shutdown() noexcept {
        return backend_.shutdown();
    }

private:
    Backend& backend_;
};

} // namespace mcubsp::driver
