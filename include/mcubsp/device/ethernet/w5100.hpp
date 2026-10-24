#pragma once

// @req-file REQ-0002 REQ-0035 REQ-0044 REQ-0046
#include <chrono>
#include <cstddef>
#include <expected>
#include <optional>
#include <span>
#include <utility>

#include <mcubsp/device/ethernet/w5100_macraw.hpp>
#include <mcubsp/device/ethernet/w5100_spi.hpp>

namespace mcubsp::device {

// An attached W5100 owns its SPI device endpoint; its SPI controller and board
// must outlive it. All calls use the board's foreground execution context.
// Endpoint, transport and protocol storage are fixed and never allocate.
template<typename SpiDevice, std::size_t RxDescriptors = 2U,
         std::size_t TxDescriptors = 2U>
class w5100 {
    using transport_type = w5100_io_transport<SpiDevice>;
    using protocol_type = w5100_macraw_device<transport_type,
                                            RxDescriptors, TxDescriptors>;
public:
    static constexpr std::size_t rx_descriptor_count = RxDescriptors;
    static constexpr std::size_t tx_descriptor_count = TxDescriptors;
    static constexpr std::size_t buffer_size = protocol_type::buffer_size;
    static constexpr std::size_t ethernet_header_size =
        protocol_type::ethernet_header_size;
    static constexpr auto default_timeout = std::chrono::milliseconds{1000};
    static constexpr auto default_poll_timeout = std::chrono::milliseconds{10};

    explicit w5100(SpiDevice spi) noexcept
        : spi_(std::move(spi)), transport_(spi_), protocol_(transport_) {}
    w5100(const w5100&) = delete;
    w5100& operator=(const w5100&) = delete;
    w5100(w5100&&) = delete;
    w5100& operator=(w5100&&) = delete;

    template<typename Rep = std::chrono::milliseconds::rep,
             typename Period = std::milli>
    [[nodiscard]] std::expected<void, w5100_error>
    begin(w5100_configuration config,
          std::chrono::duration<Rep, Period> timeout = default_timeout) noexcept {
        const auto limit = deadline_after(timeout);
        if (!limit) return std::unexpected(limit.error());
        return begin_until(config, *limit);
    }

    [[nodiscard]] std::expected<void, w5100_error>
    begin_until(w5100_configuration config, execution::deadline limit) noexcept {
        // Original W5100 datasheet v1.2.8, sections 6.3 and 7: mode 0,
        // MSB first, active-low SS, one 32-bit stream per register. The 70 ns
        // minimum clock period gives a conservative 14 MHz API ceiling.
        const auto& bus_config = spi_.configuration();
        if (bus_config.frame_width != driver::spi_frame_width::bits8
            || bus_config.bit_order != driver::spi_bit_order::most_significant_first
            || bus_config.chip_select != driver::spi_chip_select_polarity::active_low
            || bus_config.mode != driver::spi_mode::mode0
            || spi_.frequency() == 0U || spi_.frequency() > 14'000'000U) {
            return remember(std::unexpected(w5100_error::invalid_argument));
        }
        transport_.until(limit);
        return remember(protocol_.initialize(config));
    }

    template<typename Rep = std::chrono::milliseconds::rep,
             typename Period = std::milli>
    [[nodiscard]] std::expected<void, w5100_error>
    initialize(w5100_configuration config,
               std::chrono::duration<Rep, Period> timeout = default_timeout) noexcept {
        return begin(config, timeout);
    }

    [[nodiscard]] bool arm_receive(std::size_t index,
                                   std::span<std::byte> buffer) noexcept {
        return protocol_.arm_receive(index, buffer);
    }
    [[nodiscard]] bool start_transmit(
        std::size_t index, std::span<const std::byte> frame) noexcept {
        return protocol_.start_transmit(index, frame);
    }

    // A polling quantum copies at most config.payload_bytes_per_poll payload
    // bytes across RX and TX. Housekeeping is bounded by configuration and the
    // same absolute deadline. Frame buffers remain borrowed across quanta.
    void poll() noexcept {
        if (!initialized()) return;
        const auto limit = deadline_after(default_poll_timeout);
        if (limit) static_cast<void>(poll_until(*limit));
    }

    [[nodiscard]] std::expected<void, w5100_error>
    poll_until(execution::deadline limit) noexcept {
        transport_.until(limit);
        return remember(protocol_.poll_once());
    }

    [[nodiscard]] std::optional<w5100_receive_completion>
    take_receive(std::size_t index) noexcept {
        return protocol_.take_receive(index);
    }
    [[nodiscard]] std::optional<w5100_transmit_completion>
    take_transmit_result(std::size_t index) noexcept {
        return protocol_.take_transmit_result(index);
    }
    [[nodiscard]] bool take_transmit(std::size_t index) noexcept {
        return protocol_.take_transmit(index);
    }
    void cancel_all() noexcept { protocol_.cancel_all(); }

    template<typename Rep = std::chrono::milliseconds::rep,
             typename Period = std::milli>
    [[nodiscard]] std::expected<void, w5100_error>
    recover(std::chrono::duration<Rep, Period> timeout = default_timeout) noexcept {
        const auto limit = deadline_after(timeout);
        if (!limit) return std::unexpected(limit.error());
        return recover_until(*limit);
    }
    [[nodiscard]] std::expected<void, w5100_error>
    recover_until(execution::deadline limit) noexcept {
        transport_.until(limit);
        return remember(protocol_.recover());
    }

    [[nodiscard]] bool initialized() const noexcept {
        return protocol_.initialized();
    }
    [[nodiscard]] const w5100_metrics& metrics() const noexcept {
        return protocol_.metrics();
    }
    [[nodiscard]] std::optional<w5100_error> last_error() const noexcept {
        return local_error_ ? local_error_ : protocol_.last_error();
    }

private:
    template<typename Rep, typename Period>
    [[nodiscard]] std::expected<execution::deadline, w5100_error>
    deadline_after(std::chrono::duration<Rep, Period> timeout) noexcept {
        const auto limit = spi_.deadline_after(timeout);
        if (!limit) {
            local_error_ = limit.error().code == io::errc::invalid_timeout
                ? w5100_error::invalid_argument : w5100_error::transport;
            return std::unexpected(*local_error_);
        }
        return *limit;
    }

    [[nodiscard]] std::expected<void, w5100_error>
    remember(std::expected<void, w5100_error> result) noexcept {
        if (result) local_error_.reset();
        else local_error_ = result.error();
        return result;
    }

    SpiDevice spi_;
    transport_type transport_;
    protocol_type protocol_;
    std::optional<w5100_error> local_error_{};
};

} // namespace mcubsp::device
