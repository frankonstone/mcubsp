// SPDX-License-Identifier: Apache-2.0 OR MIT

// @req-file REQ-0002 REQ-0003 REQ-0009 REQ-0015 REQ-0017 REQ-0019 REQ-0028 REQ-0030 REQ-0031 REQ-0035 REQ-0036 REQ-0037 REQ-0038 REQ-0041 REQ-0044

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <utility>

#include <mcuip/checksum.hpp>
#include <mcuip/ethernet.hpp>
#include <mcuip/icmp.hpp>
#include <mcuip/ipv4.hpp>
#include <mcuip/ipv4_host.hpp>
#include <mcutest/mcutest.hpp>

#include <mcuhw/backend/ra4m1/spi.hpp>
#include <mcuhw/board/arduino_uno_r4_minima.hpp>
#include <mcuhw/driver/gpio.hpp>
#include <mcuhw/device/ethernet/w5100_spi.hpp>
#include <mcuhw/integration/mcuip/w5100.hpp>

#include "support.hpp"

#ifndef MCUHW_SOURCE_ID
#define MCUHW_SOURCE_ID "unavailable"
#endif

#ifndef MCUHW_MCUIP_SOURCE_ID
#define MCUHW_MCUIP_SOURCE_ID "unavailable"
#endif

#ifndef MCUHW_ETHERNET_SPI_HZ
#define MCUHW_ETHERNET_SPI_HZ 6000000
#endif

namespace {

namespace board = mcuhw::board::arduino_uno_r4_minima;
using spi_binding = mcuhw::backend::ra4m1::spi_binding<
    1, board::d12_miso, board::d11_mosi, board::d13_sck, board::d10_ssl>;
using spi_type = mcuhw::backend::ra4m1::spi_master_backend<
    spi_binding, mcuhw::core::direct_mmio,
    mcuhw::board_test::execution_policy>;

#ifndef MCUHW_ETHERNET_FAST_SPI
struct deadline_source {
    [[nodiscard]] mcuhw::execution::deadline deadline() const noexcept {
        return {static_cast<std::uint32_t>(
            mcuhw::board_test::execution_hooks::now() + 4'800'000U)};
    }
};

using transport_type = mcuhw::device::w5100_spi_transport<
    spi_type, deadline_source>;
#else
using transport_type =
    mcuhw::device::w5100_spi_immediate_transport<spi_type>;
#endif
using device_type = mcuhw::integration::mcuip::
    w5100_macraw_network_device<transport_type>;
using host_type = ::mcuip::EthernetIpv4Host<4U>;

static_assert(::mcuip::NetworkDevice<device_type>);

mcuhw::board_test::execution_policy execution;
spi_type spi{mcuhw::board_test::mmio, execution};
#ifndef MCUHW_ETHERNET_FAST_SPI
deadline_source deadlines;
transport_type transport{spi, deadlines};
#else
transport_type transport{spi};
#endif
device_type device{transport};

constexpr mcuhw::core::pin_id sd_chip_select_pin{1U, 3U};
mcuhw::driver::gpio_pin sd_chip_select{
    mcuhw::board_test::gpio, sd_chip_select_pin};

constexpr auto spi_timing = mcuhw::driver::solve_spi_timing(
    {48'000'000U, MCUHW_ETHERNET_SPI_HZ, 0U});
static_assert(spi_timing.has_value());
static_assert(spi_timing->achieved_hz == MCUHW_ETHERNET_SPI_HZ);

inline constexpr auto mac_address =
    ::mcuip::MacAddress::from_octets(0x02U, 0x00U, 0x00U,
                                    0x04U, 0x51U, 0x00U);
inline constexpr ::mcuip::Ipv4InterfaceConfig ipv4_configuration{
    .mode = ::mcuip::AddressingMode::static_address,
    .address = ::mcuip::Ipv4Address::from_octets(192U, 168U, 188U, 240U),
    .netmask = ::mcuip::Ipv4Address::from_octets(255U, 255U, 255U, 0U),
};
inline constexpr mcuhw::device::w5100_configuration device_configuration{
    .mac = {{0x02U, 0x00U, 0x00U, 0x04U, 0x51U, 0x00U}},
    .command_poll_limit = 256U,
    .stable_read_limit = 4U,
};
inline constexpr auto peer_ip =
    ::mcuip::Ipv4Address::from_octets(192U, 168U, 188U, 1U);

std::array<std::array<std::byte, device_type::buffer_size>,
           device_type::rx_descriptor_count> receive_buffers{};
std::array<std::array<std::byte, device_type::buffer_size>,
           device_type::tx_descriptor_count> transmit_buffers{};
std::array<bool, device_type::tx_descriptor_count> transmit_busy{};

bool sd_deselected{};
bool spi_initialized{};
bool w5100_initialized{};
bool mcuip_initialized{};
std::uint32_t raw_frames{};
std::uint32_t arp_frames{};
std::uint32_t icmp_echo_requests{};
std::uint32_t icmp_echo_replies{};
std::uint32_t replies_submitted{};
std::uint32_t replies_completed{};
std::uint32_t receive_errors{};
std::uint32_t minimum_frame = 0xFFFF'FFFFU;
std::uint32_t maximum_frame{};
std::array<std::uint8_t, 24U> first_frame_prefix{};
std::size_t first_frame_size{};
std::array<std::byte, 8U> small_echo{};
std::array<std::byte, ::mcuip::ipv4_mtu - ::mcuip::ipv4_min_header_size>
    mtu_echo{};
bool arp_request_submitted{};
bool arp_resolved{};
bool small_echo_submitted{};
bool mtu_echo_submitted{};
bool recovery_attempted{};
bool recovery_completed{};
bool post_recovery_echo_submitted{};
inline constexpr std::uint32_t benchmark_echo_count = 64U;
// Four maximum-sized replies occupy about 6.1 KiB, safely below socket 0's
// 8 KiB RX allocation. A window of eight can overflow the W5100 before this
// polling driver has drained the first replies.
inline constexpr std::uint32_t benchmark_window = 4U;
inline constexpr std::uint16_t benchmark_sequence_base = 0x1000U;
inline constexpr std::uint32_t benchmark_reply_timeout_cycles = 96'000'000U;
#ifdef MCUHW_ETHERNET_METRICS
inline constexpr bool benchmark_enabled = true;
inline constexpr char peer_exchange_description[] =
    "firmware ARP plus initial small/full-MTU ICMP and post-socket-recovery echo; 64 full-MTU echoes benchmarked with window 4";
#else
inline constexpr bool benchmark_enabled = false;
#ifdef MCUHW_ETHERNET_LINK_RECOVERY
inline constexpr char peer_exchange_description[] =
    "firmware ARP plus initial small/full-MTU ICMP and post-socket-recovery echo; continuous small echoes detect manual cable loss and restored traffic";
#else
inline constexpr char peer_exchange_description[] =
    "firmware ARP plus 8-byte and 1480-byte ICMP echo to gateway 192.168.188.1; 8-byte echo repeated after W5100 socket recovery";
#endif
#endif
#ifdef MCUHW_ETHERNET_LINK_RECOVERY
inline constexpr bool link_recovery_enabled = true;
#else
inline constexpr bool link_recovery_enabled = false;
#endif
bool benchmark_started{};
bool benchmark_completed{};
std::uint32_t benchmark_attempts{};
std::uint32_t benchmark_replies{};
std::uint32_t benchmark_timeouts{};
std::uint32_t benchmark_in_flight{};
std::uint32_t benchmark_started_cycles{};
std::uint32_t benchmark_elapsed_cycles{};
std::uint64_t benchmark_total_rtt_cycles{};
std::uint32_t benchmark_min_rtt_cycles = 0xFFFF'FFFFU;
std::uint32_t benchmark_max_rtt_cycles{};
std::uint32_t benchmark_average_rtt_cycles{};
std::uint32_t benchmark_payload_goodput_bps{};
std::array<std::uint32_t, benchmark_echo_count> benchmark_submit_cycles{};
std::array<bool, benchmark_echo_count> benchmark_outstanding{};
inline constexpr std::uint16_t link_probe_sequence_base = 0x2000U;
inline constexpr std::uint32_t link_reply_timeout_cycles = 24'000'000U;
bool link_probe_waiting{};
bool link_loss_detected{};
bool link_traffic_restored{};
bool link_socket_recovered{};
std::uint16_t link_probe_sequence{};
std::uint32_t link_probe_last_submit_cycles{};
std::uint32_t link_probe_attempts{};
std::uint32_t link_probe_replies{};
std::uint32_t link_probe_timeouts{};

void configure_interrupts() noexcept {
    constexpr std::array<std::uint16_t, 4U> events{
        mcuhw::mcu::ra4m1::event_number(mcuhw::mcu::ra4m1::event::spi1_rxi),
        mcuhw::mcu::ra4m1::event_number(mcuhw::mcu::ra4m1::event::spi1_txi),
        mcuhw::mcu::ra4m1::event_number(mcuhw::mcu::ra4m1::event::spi1_idle),
        mcuhw::mcu::ra4m1::event_number(mcuhw::mcu::ra4m1::event::spi1_error),
    };
    *reinterpret_cast<volatile std::uint32_t*>(0xE000'E180U) = 0x0FU;
    for (std::uint32_t line = 0U; line < events.size(); ++line) {
        *reinterpret_cast<volatile std::uint32_t*>(0x4000'6300U + line * 4U)
            = events[line];
        *reinterpret_cast<volatile std::uint8_t*>(0xE000'E400U + line)
            = 0x80U;
    }
    *reinterpret_cast<volatile std::uint32_t*>(0xE000'E280U) = 0x0FU;
    *reinterpret_cast<volatile std::uint32_t*>(0xE000'E100U) = 0x0FU;
    __asm__ volatile("dsb sy\n\tisb sy\n\tcpsie i" ::: "memory");
}

[[nodiscard]] std::optional<std::uint16_t>
classify(std::span<const std::byte> frame) noexcept {
    ++raw_frames;
    if (first_frame_size == 0U) {
        first_frame_size = frame.size();
        const auto copied = frame.size() < first_frame_prefix.size()
            ? frame.size() : first_frame_prefix.size();
        for (std::size_t index = 0U; index < copied; ++index) {
            first_frame_prefix[index] =
                std::to_integer<std::uint8_t>(frame[index]);
        }
    }
    const auto size = static_cast<std::uint32_t>(frame.size());
    if (size < minimum_frame) minimum_frame = size;
    if (size > maximum_frame) maximum_frame = size;
    const auto ethernet = ::mcuip::parse_ethernet_frame(frame);
    if (!ethernet) return std::nullopt;
    if (ethernet->type == ::mcuip::EtherType::arp) {
        ++arp_frames;
        return std::nullopt;
    }
    if (ethernet->type != ::mcuip::EtherType::ipv4) return std::nullopt;
    const auto ipv4 = ::mcuip::parse_ipv4(ethernet->payload);
    if (!ipv4 || ipv4->protocol
            != static_cast<std::uint8_t>(::mcuip::IpProtocol::icmp)) {
        return std::nullopt;
    }
    const auto icmp = ::mcuip::parse_icmp(ipv4->payload);
    if (icmp && icmp->type
            == static_cast<std::uint8_t>(::mcuip::IcmpType::echo_request)) {
        ++icmp_echo_requests;
    } else if (icmp && icmp->type
            == static_cast<std::uint8_t>(::mcuip::IcmpType::echo_reply)) {
        ++icmp_echo_replies;
        if (icmp->rest.size() >= 4U) {
            return static_cast<std::uint16_t>(
                (static_cast<std::uint16_t>(
                     std::to_integer<std::uint8_t>(icmp->rest[2]))
                 << 8U)
                | std::to_integer<std::uint8_t>(icmp->rest[3]));
        }
    }
    return std::nullopt;
}

template<std::size_t Size>
void prepare_echo(std::array<std::byte, Size>& request,
                  std::uint16_t sequence) noexcept {
    static_assert(Size >= 8U);
    request.fill(std::byte{0xA5U});
    request[0] = static_cast<std::byte>(::mcuip::IcmpType::echo_request);
    request[1] = std::byte{0};
    request[2] = std::byte{0};
    request[3] = std::byte{0};
    request[4] = std::byte{0x45U};
    request[5] = std::byte{0x10U};
    request[6] = static_cast<std::byte>(sequence >> 8U);
    request[7] = static_cast<std::byte>(sequence);
    const auto checksum = ::mcuip::internet_checksum(request);
    request[2] = static_cast<std::byte>(checksum >> 8U);
    request[3] = static_cast<std::byte>(checksum);
}

[[nodiscard]] std::optional<std::size_t> free_transmit_slot() noexcept {
    for (std::size_t index = 0U; index < transmit_busy.size(); ++index) {
        if (!transmit_busy[index]) return index;
    }
    return std::nullopt;
}

[[nodiscard]] bool submit_ipv4(
    host_type& host, std::span<const std::byte> payload,
    ::mcuip::MonotonicTime now,
    std::optional<::mcuip::PendingDatagramId> pending = {}) noexcept {
    const auto slot = free_transmit_slot();
    if (!slot) return false;
    auto& output = transmit_buffers[*slot];
    const auto result = host.send_ipv4(
        peer_ip, static_cast<std::uint8_t>(::mcuip::IpProtocol::icmp),
        payload, output, now, pending);
    if (!result || result->output_size == 0U) return false;
    if (!device.start_transmit(
            *slot, std::span<const std::byte>{output}.first(
                       result->output_size))) {
        return false;
    }
    transmit_busy[*slot] = true;
    ++replies_submitted;
    return true;
}

void record_benchmark_reply(
    std::optional<std::uint16_t> sequence) noexcept {
    if (!benchmark_enabled || !sequence || *sequence < benchmark_sequence_base) {
        return;
    }
    const auto index = static_cast<std::size_t>(
        *sequence - benchmark_sequence_base);
    if (index >= benchmark_echo_count || !benchmark_outstanding[index]) return;
    const auto now = mcuhw::board_test::execution_hooks::now();
    const auto rtt = static_cast<std::uint32_t>(
        now - benchmark_submit_cycles[index]);
    benchmark_total_rtt_cycles += rtt;
    if (rtt < benchmark_min_rtt_cycles) benchmark_min_rtt_cycles = rtt;
    if (rtt > benchmark_max_rtt_cycles) benchmark_max_rtt_cycles = rtt;
    benchmark_outstanding[index] = false;
    --benchmark_in_flight;
    ++benchmark_replies;
}

void finish_benchmark(std::uint32_t now) noexcept {
    if (benchmark_completed || benchmark_attempts != benchmark_echo_count
        || benchmark_in_flight != 0U) {
        return;
    }
    benchmark_completed = true;
    benchmark_elapsed_cycles = static_cast<std::uint32_t>(
        now - benchmark_started_cycles);
    if (benchmark_replies != 0U) {
        benchmark_average_rtt_cycles = static_cast<std::uint32_t>(
            benchmark_total_rtt_cycles / benchmark_replies);
    }
    const auto elapsed_ms =
        (benchmark_elapsed_cycles + 47'999U) / 48'000U;
    const auto acknowledged_bytes = static_cast<std::uint32_t>(
        benchmark_replies * mtu_echo.size());
    benchmark_payload_goodput_bps = elapsed_ms == 0U ? 0U
        : acknowledged_bytes * 8'000U / elapsed_ms;
}

void expire_benchmark_requests(std::uint32_t now) noexcept {
    for (std::size_t index = 0U; index < benchmark_attempts; ++index) {
        if (benchmark_outstanding[index]
            && static_cast<std::uint32_t>(
                   now - benchmark_submit_cycles[index])
                >= benchmark_reply_timeout_cycles) {
            benchmark_outstanding[index] = false;
            --benchmark_in_flight;
            ++benchmark_timeouts;
        }
    }
}

void service_benchmark(host_type& host,
                       ::mcuip::MonotonicTime now) noexcept {
    if (!benchmark_enabled || !recovery_completed || icmp_echo_replies < 3U
        || benchmark_completed) {
        return;
    }
    const auto cycles = mcuhw::board_test::execution_hooks::now();
    expire_benchmark_requests(cycles);
    if (benchmark_attempts < benchmark_echo_count
        && benchmark_in_flight < benchmark_window
        && free_transmit_slot().has_value()) {
        prepare_echo(mtu_echo, static_cast<std::uint16_t>(
            benchmark_sequence_base + benchmark_attempts));
        if (submit_ipv4(host, mtu_echo, now)) {
            if (!benchmark_started) {
                benchmark_started = true;
                benchmark_started_cycles = cycles;
            }
            benchmark_submit_cycles[benchmark_attempts] = cycles;
            benchmark_outstanding[benchmark_attempts] = true;
            ++benchmark_attempts;
            ++benchmark_in_flight;
        }
    }
    finish_benchmark(cycles);
}

void record_link_probe_reply(
    std::optional<std::uint16_t> sequence) noexcept {
    if (!link_recovery_enabled || !link_probe_waiting || !sequence
        || *sequence != link_probe_sequence) {
        return;
    }
    link_probe_waiting = false;
    ++link_probe_replies;
    if (link_loss_detected) link_traffic_restored = true;
}

void service_link_recovery(host_type& host,
                           ::mcuip::MonotonicTime now) noexcept {
    if (!link_recovery_enabled || !recovery_completed
        || icmp_echo_replies < 3U || link_traffic_restored) {
        return;
    }
    const auto cycles = mcuhw::board_test::execution_hooks::now();
    if (link_probe_waiting
        && static_cast<std::uint32_t>(cycles - link_probe_last_submit_cycles)
            >= link_reply_timeout_cycles) {
        link_probe_waiting = false;
        ++link_probe_timeouts;
        if (link_probe_replies != 0U && !link_loss_detected) {
            link_loss_detected = true;
            link_socket_recovered = device.recover().has_value();
        }
    }
    if (link_probe_waiting || !free_transmit_slot()) return;
    link_probe_sequence = static_cast<std::uint16_t>(
        link_probe_sequence_base + link_probe_attempts);
    prepare_echo(small_echo, link_probe_sequence);
    if (!submit_ipv4(host, small_echo, now)) return;
    link_probe_last_submit_cycles = cycles;
    link_probe_waiting = true;
    ++link_probe_attempts;
}

void service(host_type& host, std::uint32_t now_ms) noexcept {
    device.poll();
    for (std::size_t index = 0U; index < transmit_busy.size(); ++index) {
        const auto completed = device.take_transmit_result(index);
        if (!completed) continue;
        transmit_busy[index] = false;
        if (!completed->error) ++replies_completed;
    }
    for (std::size_t index = 0U; index < receive_buffers.size(); ++index) {
        const auto completion = device.take_receive(index);
        if (!completion) continue;
        if (completion->error
            || completion->size > receive_buffers[index].size()) {
            ++receive_errors;
        } else {
            const auto input = std::span<const std::byte>{receive_buffers[index]}
                                   .first(completion->size);
            const auto echo_reply = classify(input);
            record_benchmark_reply(echo_reply);
            record_link_probe_reply(echo_reply);
            if (!transmit_busy[index]) {
                const auto result = host.process_frame(
                    input, transmit_buffers[index],
                    ::mcuip::MonotonicTime::from_ticks(now_ms));
                if (result.output_size != 0U
                    && device.start_transmit(
                        index,
                        std::span<const std::byte>{transmit_buffers[index]}
                            .first(result.output_size))) {
                    transmit_busy[index] = true;
                    ++replies_submitted;
                }
                if (result.released_pending.has_value()) arp_resolved = true;
            }
        }
        static_cast<void>(device.arm_receive(index, receive_buffers[index]));
    }
    const auto now = ::mcuip::MonotonicTime::from_ticks(now_ms);
    if (arp_resolved && !small_echo_submitted) {
        small_echo_submitted = submit_ipv4(host, small_echo, now);
    }
    if (icmp_echo_replies >= 1U && !mtu_echo_submitted) {
        mtu_echo_submitted = submit_ipv4(host, mtu_echo, now);
    }
    if (icmp_echo_replies >= 2U && !recovery_attempted
        && free_transmit_slot().has_value()) {
        recovery_attempted = true;
        recovery_completed = device.recover().has_value();
        if (recovery_completed) {
            post_recovery_echo_submitted = submit_ipv4(host, small_echo, now);
        }
    }
    service_benchmark(host, now);
    service_link_recovery(host, now);
}

#if defined(MCUHW_ETHERNET_METRICS)
#if defined(MCUHW_ETHERNET_FAST_SPI) && MCUHW_ETHERNET_SPI_HZ == 12000000
TEST(uno_ethernet, measures_saturated_mtu_fast_spi_12mhz,
     "REQ-0036", "REQ-0037", "REQ-0044") {
#elif defined(MCUHW_ETHERNET_FAST_SPI)
TEST(uno_ethernet, measures_saturated_mtu_fast_spi_6mhz,
     "REQ-0036", "REQ-0037", "REQ-0044") {
#else
TEST(uno_ethernet, measures_saturated_mtu_interrupt_spi_6mhz,
     "REQ-0036", "REQ-0037", "REQ-0044") {
#endif
    EXPECT_TRUE(benchmark_started);
    EXPECT_TRUE(benchmark_completed);
    EXPECT_EQ(benchmark_replies, benchmark_echo_count);
    EXPECT_EQ(benchmark_attempts, benchmark_echo_count);
    EXPECT_EQ(benchmark_timeouts, 0U);
    EXPECT_TRUE(benchmark_min_rtt_cycles > 0U);
    EXPECT_TRUE(benchmark_average_rtt_cycles >= benchmark_min_rtt_cycles);
    EXPECT_TRUE(benchmark_max_rtt_cycles >= benchmark_average_rtt_cycles);
    EXPECT_TRUE(benchmark_payload_goodput_bps > 0U);
}
#elif defined(MCUHW_ETHERNET_LINK_RECOVERY)
TEST(uno_ethernet, restores_traffic_after_physical_link_loss,
     "REQ-0036", "REQ-0044") {
    EXPECT_TRUE(link_probe_replies > 0U);
    EXPECT_TRUE(link_loss_detected);
    EXPECT_TRUE(link_socket_recovered);
    EXPECT_TRUE(link_traffic_restored);
    EXPECT_TRUE(link_probe_timeouts > 0U);
    EXPECT_EQ(device.metrics().transport_errors, 0U);
}
#else
TEST(uno_ethernet, initializes_w5100_macraw_over_spi,
     "REQ-0044") {
    EXPECT_TRUE(sd_deselected);
    EXPECT_TRUE(spi_initialized);
    EXPECT_TRUE(w5100_initialized);
    EXPECT_EQ(device.metrics().transport_errors, 0U);
}

TEST(uno_ethernet, exchanges_raw_small_and_mtu_frames,
     "REQ-0044") {
    EXPECT_TRUE(raw_frames >= 2U);
    EXPECT_TRUE(minimum_frame <= 64U);
    EXPECT_TRUE(maximum_frame >= 1514U);
    EXPECT_EQ(receive_errors, 0U);
}

TEST(uno_ethernet, answers_arp_and_icmp_through_mcuip,
     "REQ-0035", "REQ-0044") {
    EXPECT_TRUE(mcuip_initialized);
    EXPECT_TRUE(arp_frames >= 1U);
    EXPECT_TRUE(icmp_echo_replies >= 3U);
    EXPECT_TRUE(replies_submitted >= 4U);
    EXPECT_TRUE(replies_completed >= 4U);
    EXPECT_TRUE(recovery_completed);
    EXPECT_TRUE(post_recovery_echo_submitted);
    EXPECT_EQ(device.metrics().recoveries, 1U);
}
#endif

#ifdef MCUHW_ETHERNET_METRICS
#if defined(MCUHW_ETHERNET_FAST_SPI) && MCUHW_ETHERNET_SPI_HZ == 12000000
constexpr char metadata[] =
    "MCUHW_META board=Arduino_UNO_R4_Minima suite=ethernet_metrics_fast12 "
    "firmware=pln-0024 fixture=Ethernet_Shield_R3_W5100 "
    "w5100_cs=D10 sd_cs=D4 spi_hz=12000000 transport=bounded_polling "
    "mode=MACRAW ipv4=192.168.188.240/24 mac=02:00:00:04:51:00 "
    "coverage=disabled optimization=Os toolchain=arm-none-eabi-gcc-15.2.1 "
    "interface=J-Link_SWD_RTT\n";
constexpr mcuhw::board_test::evidence_metadata evidence{
    .suite = "ethernet_metrics_fast12",
    .firmware = "pln-0024",
    .fixture = "Arduino Ethernet Shield R3 W5100; D10 CS; D4 high; local Ethernet peer",
    .coverage_build_id = 0U,
    .coverage_enabled = false,
};
#elif defined(MCUHW_ETHERNET_FAST_SPI)
constexpr char metadata[] =
    "MCUHW_META board=Arduino_UNO_R4_Minima suite=ethernet_metrics_fast6 "
    "firmware=pln-0024 fixture=Ethernet_Shield_R3_W5100 "
    "w5100_cs=D10 sd_cs=D4 spi_hz=6000000 transport=bounded_polling "
    "mode=MACRAW ipv4=192.168.188.240/24 mac=02:00:00:04:51:00 "
    "coverage=disabled optimization=Os toolchain=arm-none-eabi-gcc-15.2.1 "
    "interface=J-Link_SWD_RTT\n";
constexpr mcuhw::board_test::evidence_metadata evidence{
    .suite = "ethernet_metrics_fast6",
    .firmware = "pln-0024",
    .fixture = "Arduino Ethernet Shield R3 W5100; D10 CS; D4 high; local Ethernet peer",
    .coverage_build_id = 0U,
    .coverage_enabled = false,
};
#else
constexpr char metadata[] =
    "MCUHW_META board=Arduino_UNO_R4_Minima suite=ethernet_metrics "
    "firmware=pln-0024 fixture=Ethernet_Shield_R3_W5100 "
    "w5100_cs=D10 sd_cs=D4 spi_hz=6000000 mode=MACRAW "
    "ipv4=192.168.188.240/24 mac=02:00:00:04:51:00 "
    "coverage=disabled optimization=Os toolchain=arm-none-eabi-gcc-15.2.1 "
    "interface=J-Link_SWD_RTT\n";
constexpr mcuhw::board_test::evidence_metadata evidence{
    .suite = "ethernet_metrics",
    .firmware = "pln-0024",
    .fixture = "Arduino Ethernet Shield R3 W5100; D10 CS; D4 high; local Ethernet peer",
    .coverage_build_id = 0U,
    .coverage_enabled = false,
};
#endif
#elif defined(MCUHW_ETHERNET_LINK_RECOVERY)
constexpr char metadata[] =
    "MCUHW_META board=Arduino_UNO_R4_Minima suite=ethernet_link_recovery "
    "firmware=pln-0024 fixture=Ethernet_Shield_R3_W5100 "
    "w5100_cs=D10 sd_cs=D4 spi_hz=6000000 mode=MACRAW "
    "ipv4=192.168.188.240/24 mac=02:00:00:04:51:00 "
    "fault=manual_cable_disconnect_reconnect coverage=disabled "
    "optimization=Os toolchain=arm-none-eabi-gcc-15.2.1 "
    "interface=J-Link_SWD_RTT\n";
constexpr mcuhw::board_test::evidence_metadata evidence{
    .suite = "ethernet_link_recovery",
    .firmware = "pln-0024",
    .fixture = "Arduino Ethernet Shield R3 W5100; manual cable disconnect/reconnect",
    .coverage_build_id = 0U,
    .coverage_enabled = false,
};
#else
constexpr char metadata[] =
    "MCUHW_META board=Arduino_UNO_R4_Minima suite=ethernet_mcuip "
    "firmware=pln-0024 fixture=Ethernet_Shield_R3_W5100 "
    "w5100_cs=D10 sd_cs=D4 spi_hz=6000000 mode=MACRAW "
    "ipv4=192.168.188.240/24 mac=02:00:00:04:51:00 "
    "toolchain=arm-none-eabi-gcc-15.2.1 interface=J-Link_SWD_RTT\n";
constexpr mcuhw::board_test::evidence_metadata evidence{
    .suite = "ethernet_mcuip",
    .firmware = "pln-0024",
    .fixture = "Arduino Ethernet Shield R3 W5100; D10 CS; D4 high; local Ethernet peer",
    .coverage_build_id = 0x4d485702U,
};
#endif

} // namespace

int main() {
    mcuhw::board_test::start(metadata);
    sd_deselected = sd_chip_select.initialize({
        .direction = mcuhw::driver::gpio_direction::output,
        .initial_level = mcuhw::driver::gpio_level::high,
        .output_type = mcuhw::driver::gpio_output_type::push_pull,
        .pull = mcuhw::driver::gpio_pull::none,
        .drive = mcuhw::driver::gpio_drive::middle,
    }).has_value();
    configure_interrupts();
    spi_initialized = spi.initialize(*spi_timing, {
        .mode = mcuhw::driver::spi_mode::mode0,
        .bit_order = mcuhw::driver::spi_bit_order::most_significant_first,
        .frame_width = mcuhw::driver::spi_frame_width::bits8,
    }).has_value();
    if (spi_initialized) {
        w5100_initialized = device.initialize(device_configuration).has_value();
    }
    auto created = host_type::create(mac_address, ipv4_configuration);
    mcuip_initialized = created.has_value();
    if (w5100_initialized && created) {
        for (std::size_t index = 0U; index < receive_buffers.size(); ++index) {
            static_cast<void>(device.arm_receive(index, receive_buffers[index]));
        }
        auto host = std::move(*created);
        prepare_echo(small_echo, 1U);
        prepare_echo(mtu_echo, 2U);
        arp_request_submitted = submit_ipv4(
            host, small_echo, ::mcuip::MonotonicTime::from_ticks(0U),
            ::mcuip::PendingDatagramId{1U});
        const auto started = mcuhw::board_test::execution_hooks::now();
#if defined(MCUHW_ETHERNET_METRICS) || defined(MCUHW_ETHERNET_LINK_RECOVERY)
        constexpr std::uint32_t test_window_cycles = 48'000'000U * 30U;
#else
        constexpr std::uint32_t test_window_cycles = 48'000'000U * 15U;
#endif
        while (static_cast<std::uint32_t>(
                   mcuhw::board_test::execution_hooks::now() - started)
               < test_window_cycles) {
            const auto elapsed = static_cast<std::uint32_t>(
                mcuhw::board_test::execution_hooks::now() - started);
            service(host, elapsed / 48'000U);
            if (arp_frames >= 1U && icmp_echo_replies >= 3U
                && replies_completed >= 4U && recovery_completed
                && minimum_frame <= 64U
                && maximum_frame >= 1514U
                && (!benchmark_enabled || benchmark_completed)
                && (!link_recovery_enabled || link_traffic_restored)) {
                break;
            }
        }
    }

    mcuhw::board_test::finish(evidence, [&](auto& observations) {
        observations("shield", "Arduino Ethernet Shield R3 WIZnet W5100");
        observations("datasheet", "W5100 Datasheet v1.2.8 2019-05-23");
        observations("w5100_identity",
                     "MR reset plus RMSR/TMSR 0x55 and SHAR read-back");
#ifdef MCUHW_ETHERNET_FAST_SPI
        observations("spi", "SPI1 mode0 bounded polling; D10 CS");
#else
        observations("spi", "SPI1 mode0 interrupt-driven; D10 CS");
#endif
        observations("spi_hz", spi_timing->achieved_hz);
        observations("sd_chip_select", "D4 held high");
        observations("ipv4", "192.168.188.240/24");
        observations("mac", "02:00:00:04:51:00");
        observations("mcuhw_source_sha256", MCUHW_SOURCE_ID);
        observations("mcuip_source_sha256", MCUHW_MCUIP_SOURCE_ID);
        observations("rx_slots", device_type::rx_descriptor_count);
        observations("tx_slots", device_type::tx_descriptor_count);
        observations("frame_capacity", device_type::buffer_size);
        observations("raw_frames", raw_frames);
        observations("arp_frames", arp_frames);
        observations("icmp_echo_requests", icmp_echo_requests);
        observations("icmp_echo_replies", icmp_echo_replies);
        observations("outbound_arp_request", arp_request_submitted);
        observations("arp_resolved", arp_resolved);
        observations("outbound_small_echo", small_echo_submitted);
        observations("outbound_mtu_echo", mtu_echo_submitted);
        observations("recovery_attempted", recovery_attempted);
        observations("recovery_completed", recovery_completed);
        observations("post_recovery_echo",
                     post_recovery_echo_submitted);
        observations("replies_submitted", replies_submitted);
        observations("replies_completed", replies_completed);
        observations("receive_errors", receive_errors);
        observations("minimum_frame", minimum_frame);
        observations("maximum_frame", maximum_frame);
        observations("first_frame_size", first_frame_size);
        observations.array("first_frame_prefix", [&](auto& bytes) {
            for (const auto byte : first_frame_prefix) bytes(byte);
        });
        observations("driver_received", device.metrics().received_frames);
        observations("driver_transmitted", device.metrics().transmitted_frames);
        observations("driver_backpressure",
                     device.metrics().receive_backpressure);
        observations("driver_recoveries", device.metrics().recoveries);
        observations("driver_transport_errors",
                     device.metrics().transport_errors);
#ifdef MCUHW_ETHERNET_METRICS
        observations("benchmark_method",
                     "64 1480-byte ICMP echo payloads to gateway; window 4; DWT cycle timing includes MCUIP, W5100 and peer RTT");
        observations("benchmark_window", benchmark_window);
        observations("benchmark_echo_payload_bytes", mtu_echo.size());
        observations("benchmark_echo_replies", benchmark_replies);
        observations("benchmark_attempts", benchmark_attempts);
        observations("benchmark_timeouts", benchmark_timeouts);
        observations("benchmark_elapsed_cycles", benchmark_elapsed_cycles);
        observations("benchmark_min_rtt_cycles", benchmark_min_rtt_cycles);
        observations("benchmark_average_rtt_cycles",
                     benchmark_average_rtt_cycles);
        observations("benchmark_max_rtt_cycles", benchmark_max_rtt_cycles);
        observations("benchmark_min_rtt_us", benchmark_min_rtt_cycles / 48U);
        observations("benchmark_average_rtt_us",
                     benchmark_average_rtt_cycles / 48U);
        observations("benchmark_max_rtt_us", benchmark_max_rtt_cycles / 48U);
        observations("benchmark_payload_goodput_bps",
                     benchmark_payload_goodput_bps);
#endif
#ifdef MCUHW_ETHERNET_LINK_RECOVERY
        observations("link_fault",
                     "manual Ethernet cable disconnect and reconnect");
        observations("link_probe_attempts", link_probe_attempts);
        observations("link_probe_replies", link_probe_replies);
        observations("link_probe_timeouts", link_probe_timeouts);
        observations("link_loss_detected", link_loss_detected);
        observations("link_socket_recovered", link_socket_recovered);
        observations("link_traffic_restored", link_traffic_restored);
#endif
        observations("peer_exchange",
                     peer_exchange_description);
    });
}

extern "C" void IRQ0_Handler() noexcept {
    static_cast<void>(spi.handle_receive_interrupt());
    mcuhw::board_test::clear_icu_line(0U);
}

extern "C" void IRQ1_Handler() noexcept {
    static_cast<void>(spi.handle_transmit_empty_interrupt());
    mcuhw::board_test::clear_icu_line(1U);
}

extern "C" void IRQ2_Handler() noexcept {
    static_cast<void>(spi.handle_idle_interrupt());
    mcuhw::board_test::clear_icu_line(2U);
}

extern "C" void IRQ3_Handler() noexcept {
    static_cast<void>(spi.handle_error_interrupt());
    mcuhw::board_test::clear_icu_line(3U);
}
