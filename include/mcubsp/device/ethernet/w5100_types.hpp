#pragma once

// @req-file REQ-0044 REQ-0046
#include <array>
#include <cstddef>
#include <cstdint>

namespace mcubsp::device {

// Register addresses and command values are from WIZnet W5100 Datasheet
// version 1.2.8 (23 May 2019). The original W5100 uses one separately framed
// 32-bit SPI stream for every byte; it does not have the burst framing used by
// later WIZnet parts.
namespace w5100_register {
inline constexpr std::uint16_t mode = 0x0000U;
inline constexpr std::uint16_t source_hardware_address = 0x0009U;
inline constexpr std::uint16_t rx_memory_size = 0x001AU;
inline constexpr std::uint16_t tx_memory_size = 0x001BU;
inline constexpr std::uint16_t socket0_mode = 0x0400U;
inline constexpr std::uint16_t socket0_command = 0x0401U;
inline constexpr std::uint16_t socket0_interrupt = 0x0402U;
inline constexpr std::uint16_t socket0_status = 0x0403U;
inline constexpr std::uint16_t socket0_tx_free_size = 0x0420U;
inline constexpr std::uint16_t socket0_tx_write_pointer = 0x0424U;
inline constexpr std::uint16_t socket0_rx_received_size = 0x0426U;
inline constexpr std::uint16_t socket0_rx_read_pointer = 0x0428U;
inline constexpr std::uint16_t socket0_tx_base = 0x4000U;
inline constexpr std::uint16_t socket0_rx_base = 0x6000U;
} // namespace w5100_register

enum class w5100_error : std::uint8_t {
    invalid_argument,
    transport,
    timeout,
    reset_failed,
    identity_mismatch,
    command_timeout,
    socket_state,
    unstable_register,
    malformed_frame,
    frame_too_large,
    cancelled,
};

struct w5100_mac_address {
    std::array<std::uint8_t, 6U> octets{};

    friend constexpr bool operator==(w5100_mac_address,
                                     w5100_mac_address) = default;
};

struct w5100_configuration {
    w5100_mac_address mac{};
    std::uint16_t command_poll_limit{256U};
    std::uint8_t stable_read_limit{4U};
    // Shared RX/TX payload-copy allowance for one poll. Register housekeeping
    // is separately bounded by command_poll_limit and stable_read_limit.
    std::uint16_t payload_bytes_per_poll{64U};
};

struct w5100_receive_completion {
    std::size_t size{};
    bool error{};
};

struct w5100_transmit_completion {
    bool error{};
};

struct w5100_metrics {
    std::uint32_t received_frames{};
    std::uint32_t transmitted_frames{};
    std::uint32_t receive_backpressure{};
    std::uint32_t malformed_frames{};
    std::uint32_t oversize_frames{};
    std::uint32_t transport_errors{};
    std::uint32_t timeouts{};
    std::uint32_t cancellations{};
    std::uint32_t recoveries{};
};

} // namespace mcubsp::device
