// @req-file REQ-0002 REQ-0003 REQ-0009 REQ-0013 REQ-0033 REQ-0036 REQ-0043

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <utility>

#include <mcuip/ipv4_host.hpp>
#include <mcutest/mcutest.hpp>

#include <mcuhw/arch/barrier.hpp>
#include <mcuhw/backend/stm32h753/ethernet.hpp>
#include <mcuhw/board/nucleo_h753zi.hpp>
#include <mcuhw/backend/stm32h753/memory.hpp>
#include <mcuhw/board/nucleo_h753zi.hpp>
#include <mcuhw/core/direct_mmio.hpp>
#include <mcuhw/integration/mcuip/stm32h753_ethernet.hpp>

#include "support.hpp"

#ifndef MCUHW_SOURCE_ID
#define MCUHW_SOURCE_ID "unavailable"
#endif

#ifndef MCUHW_MCUIP_SOURCE_ID
#define MCUHW_MCUIP_SOURCE_ID "unavailable"
#endif

extern "C" {
volatile std::uint32_t mcuhw_nucleo_ethernet_status;
volatile std::uint32_t mcuhw_nucleo_ethernet_phy_id;
volatile std::uint32_t mcuhw_nucleo_ethernet_phy_status;
volatile std::uint32_t mcuhw_nucleo_ethernet_rx_frames;
volatile std::uint32_t mcuhw_nucleo_ethernet_replies;
volatile std::uint32_t mcuhw_nucleo_ethernet_rx_min = 0xFFFF'FFFFU;
volatile std::uint32_t mcuhw_nucleo_ethernet_rx_max;
volatile std::uint32_t mcuhw_nucleo_ethernet_link_transitions;
alignas(32) volatile std::uint32_t mcuhw_nucleo_ethernet_evidence_control[8];
alignas(32) char mcuhw_nucleo_ethernet_evidence_buffer[8192];
alignas(32) std::uint8_t mcuhw_nucleo_ethernet_coverage_buffer[32768];
}

namespace {

using mmio_type = mcuhw::core::direct_mmio;
using protection_type =
    mcuhw::integration::mcuip::descriptor_protection<mmio_type>;
using device_type =
    mcuhw::integration::mcuip::network_device<mmio_type, protection_type>;
using host_type = ::mcuip::EthernetIpv4Host<4U>;

inline constexpr std::uint32_t coverage_build_id = 0x4d48'7502U;
inline constexpr auto mac_address =
    ::mcuip::MacAddress::from_octets(0x02U, 0x00U, 0x00U, 0x75U, 0x30U, 0x01U);
inline constexpr ::mcuip::Ipv4InterfaceConfig ipv4_configuration{
    .mode = ::mcuip::AddressingMode::static_address,
    .address = ::mcuip::Ipv4Address::from_octets(192U, 168U, 75U, 30U),
    .netmask = ::mcuip::Ipv4Address::from_octets(255U, 255U, 255U, 0U),
};

alignas(32) std::array<std::array<std::byte, device_type::buffer_size>,
                       device_type::rx_descriptor_count> receive_buffers{};
alignas(32) std::array<std::array<std::byte, device_type::buffer_size>,
                       device_type::tx_descriptor_count> transmit_buffers{};

TEST(nucleo_ethernet, reads_lan8742_identity_over_mdio,
     "REQ-0033", "REQ-0036", "REQ-0043") {
    EXPECT_EQ(mcuhw_nucleo_ethernet_status & 0x01U, 0x01U);
    EXPECT_EQ(mcuhw_nucleo_ethernet_phy_id >> 16U, 0x0007U);
    EXPECT_EQ(mcuhw_nucleo_ethernet_phy_id & 0xFFF0U, 0xC130U);
}

TEST(nucleo_ethernet, negotiates_physical_rmii_link,
     "REQ-0033", "REQ-0036", "REQ-0043") {
    EXPECT_EQ(mcuhw_nucleo_ethernet_status & 0x02U, 0x02U);
    EXPECT_EQ(mcuhw_nucleo_ethernet_phy_status & 0x0004U, 0x0004U);
}

TEST(nucleo_ethernet, initializes_fixed_dma_rings_and_mcuip_host,
     "REQ-0002", "REQ-0009", "REQ-0013", "REQ-0043") {
    EXPECT_EQ(mcuhw_nucleo_ethernet_status & 0x0CU, 0x0CU);
    static_assert(::mcuip::NetworkDevice<device_type>);
    static_assert(::mcuip::DescriptorProtection<protection_type>);
}

[[nodiscard]] bool clean_for_swd(mmio_type& mmio, std::uintptr_t address,
                                 std::size_t size) noexcept {
    const auto plan = mcuhw::backend::stm32h753::plan_dma_buffer(
        mcuhw::core::dma_engine_kind::stm32_dma, address, size,
        mcuhw::backend::stm32h753::dma_buffer_access::dma_reads);
    mcuhw::backend::stm32h753::dma_cache_maintenance maintenance{mmio};
    return plan && maintenance.prepare(*plan).has_value();
}

void enable_data_cache() noexcept {
    constexpr auto scb_ccr = 0xE000'ED14U;
    constexpr auto scb_dcisw = 0xE000'EF60U;
    for (std::uint32_t way = 0U; way < 4U; ++way) {
        for (std::uint32_t set = 0U; set < 128U; ++set) {
            *reinterpret_cast<volatile std::uint32_t*>(scb_dcisw) =
                (way << 30U) | (set << 5U);
        }
    }
    mcuhw::arch::data_synchronization_barrier();
    *reinterpret_cast<volatile std::uint32_t*>(scb_ccr) |= 1U << 16U;
    mcuhw::arch::data_synchronization_barrier();
    mcuhw::arch::instruction_barrier();
}

void refresh_link(mcuhw::backend::stm32h753::ethernet_rmii_mac<mmio_type, mcuhw::board::nucleo_h753zi::ethernet_binding>& mac,
                  bool& link_known, bool& link_up,
                  bool& mac_enabled) noexcept {
    const auto first = mac.read_phy(
        mcuhw::board::nucleo_h753zi::ethernet_phy_address, 1U);
    const auto bsr = mac.read_phy(
        mcuhw::board::nucleo_h753zi::ethernet_phy_address, 1U);
    if (!first || !bsr) return;
    mcuhw_nucleo_ethernet_phy_status = *bsr;
    const bool next_up = (*bsr & 0x04U) != 0U;
    if (link_known && next_up == link_up) return;
    link_known = true;
    link_up = next_up;
    mcuhw_nucleo_ethernet_link_transitions =
        mcuhw_nucleo_ethernet_link_transitions + 1U;
    if (!next_up) {
        if (mac_enabled) (void)mac.disable();
        mac_enabled = false;
        mcuhw_nucleo_ethernet_status &= ~0x02U;
        return;
    }
    const auto physical = mac.read_phy(
        mcuhw::board::nucleo_h753zi::ethernet_phy_address, 0x1FU);
    if (physical && mac.configure_link(*physical) && mac.enable()) {
        mac_enabled = true;
        mcuhw_nucleo_ethernet_status |= 0x02U;
    }
}

void service(device_type& device, host_type& host,
             mcuhw::backend::stm32h753::ethernet_rmii_mac<mmio_type, mcuhw::board::nucleo_h753zi::ethernet_binding>& mac,
             std::array<bool, device_type::tx_descriptor_count>& tx_busy,
             bool& link_known, bool& link_up, bool& mac_enabled,
             std::uint32_t ticks) noexcept {
    if ((ticks & 0xFFFFU) == 0U) {
        refresh_link(mac, link_known, link_up, mac_enabled);
    }
    device.poll();
    for (std::size_t index = 0U; index < device_type::tx_descriptor_count;
         ++index) {
        if (device.take_transmit(index)) tx_busy[index] = false;
    }
    for (std::size_t index = 0U; index < device_type::rx_descriptor_count;
         ++index) {
        const auto completion = device.take_receive(index);
        if (!completion) continue;
        if (!completion->error && completion->size <= receive_buffers[index].size()) {
            mcuhw_nucleo_ethernet_rx_frames =
                mcuhw_nucleo_ethernet_rx_frames + 1U;
            if (completion->size < mcuhw_nucleo_ethernet_rx_min) {
                mcuhw_nucleo_ethernet_rx_min = completion->size;
            }
            if (completion->size > mcuhw_nucleo_ethernet_rx_max) {
                mcuhw_nucleo_ethernet_rx_max = completion->size;
            }
            if (!tx_busy[index]) {
                const auto result = host.process_frame(
                    std::span<const std::byte>{receive_buffers[index]}.first(
                        completion->size),
                    transmit_buffers[index],
                    ::mcuip::MonotonicTime::from_ticks(ticks));
                if (result.output_size != 0U
                    && device.start_transmit(
                        index, std::span<const std::byte>{transmit_buffers[index]}
                                   .first(result.output_size))) {
                    tx_busy[index] = true;
                    mcuhw_nucleo_ethernet_replies =
                        mcuhw_nucleo_ethernet_replies + 1U;
                }
            }
        }
        (void)device.arm_receive(index, receive_buffers[index]);
    }
}

constexpr mcuhw::board_test::nucleo_h753zi::evidence_metadata evidence{
    .suite = "ethernet_mcuip",
    .firmware = "pln-0023",
    .fixture = "MB1364 LAN8742A + ordinary Ethernet peer",
    .coverage_build_id = coverage_build_id,
};

void capture_evidence(mmio_type& mmio) noexcept {
    const auto captured = mcuhw::board_test::nucleo_h753zi::capture(
        std::span<char>{mcuhw_nucleo_ethernet_evidence_buffer},
        std::span<std::uint8_t>{mcuhw_nucleo_ethernet_coverage_buffer},
        evidence, [&](auto& observations) {
            observations("ethernet_status", mcuhw_nucleo_ethernet_status);
            observations("phy_id", mcuhw_nucleo_ethernet_phy_id);
            observations("phy_status", mcuhw_nucleo_ethernet_phy_status);
            observations("ipv4", "192.168.75.30/24");
            observations("mac", "02:00:00:75:30:01");
            observations("mcuhw_source_sha256", MCUHW_SOURCE_ID);
            observations("mcuip_source_sha256", MCUHW_MCUIP_SOURCE_ID);
            observations("link_transitions",
                         mcuhw_nucleo_ethernet_link_transitions);
            observations("rx_frames", mcuhw_nucleo_ethernet_rx_frames);
            observations("mcuip_replies", mcuhw_nucleo_ethernet_replies);
            observations("rx_min_bytes", mcuhw_nucleo_ethernet_rx_min);
            observations("rx_max_bytes", mcuhw_nucleo_ethernet_rx_max);
            observations("board_bridges",
                "JP6,SB27,SB29,SB30,SB31,SB36,SB57,SB64,SB72 required");
        });
    const bool evidence_clean = clean_for_swd(
        mmio, reinterpret_cast<std::uintptr_t>(
                  mcuhw_nucleo_ethernet_evidence_buffer),
        sizeof(mcuhw_nucleo_ethernet_evidence_buffer));
    const bool coverage_clean = clean_for_swd(
        mmio, reinterpret_cast<std::uintptr_t>(
                  mcuhw_nucleo_ethernet_coverage_buffer),
        sizeof(mcuhw_nucleo_ethernet_coverage_buffer));
    mcuhw_nucleo_ethernet_evidence_control[0] = 0x4D43'5543U;
    mcuhw_nucleo_ethernet_evidence_control[1] =
        captured.complete && evidence_clean && coverage_clean ? 1U : 0U;
    mcuhw_nucleo_ethernet_evidence_control[2] = captured.evidence_size;
    mcuhw_nucleo_ethernet_evidence_control[3] = captured.coverage_size;
    mcuhw_nucleo_ethernet_evidence_control[4] = captured.test_count;
    mcuhw_nucleo_ethernet_evidence_control[5] = captured.failures;
    mcuhw_nucleo_ethernet_evidence_control[6] = coverage_build_id;
    mcuhw_nucleo_ethernet_evidence_control[7] =
        mcuhw_nucleo_ethernet_status;
    (void)clean_for_swd(
        mmio, reinterpret_cast<std::uintptr_t>(
                  mcuhw_nucleo_ethernet_evidence_control),
        sizeof(mcuhw_nucleo_ethernet_evidence_control));
}

} // namespace

int main() {
    mmio_type mmio;
    enable_data_cache();
    mcuhw::backend::stm32h753::ethernet_rmii_mac<decltype(mmio), mcuhw::board::nucleo_h753zi::ethernet_binding> mac{mmio};
    const mcuhw::backend::stm32h753::ethernet_mac_address hardware_mac{
        {0x02U, 0x00U, 0x00U, 0x75U, 0x30U, 0x01U}};

    std::uint32_t status{};
    if (mac.initialize(hardware_mac)) {
        const auto id1 = mac.read_phy(
            mcuhw::board::nucleo_h753zi::ethernet_phy_address, 2U);
        const auto id2 = mac.read_phy(
            mcuhw::board::nucleo_h753zi::ethernet_phy_address, 3U);
        if (id1 && id2) {
            mcuhw_nucleo_ethernet_phy_id =
                (static_cast<std::uint32_t>(*id1) << 16U) | *id2;
            if (*id1 == 0x0007U && (*id2 & 0xFFF0U) == 0xC130U) status |= 0x01U;
        }
    }

    std::optional<std::uint16_t> bsr;
    // Do not block bring-up on the peer. The service loop continues sampling
    // the latch-low BSR and enables the MAC when autonegotiation completes.
    for (std::uint32_t attempt = 0U; attempt < 16U; ++attempt) {
        bsr = mac.read_phy(mcuhw::board::nucleo_h753zi::ethernet_phy_address, 1U);
        if (bsr && (*bsr & 0x04U) != 0U) break;
    }
    const auto physical = mac.read_phy(
        mcuhw::board::nucleo_h753zi::ethernet_phy_address, 0x1FU);
    if (bsr) mcuhw_nucleo_ethernet_phy_status = *bsr;
    if (bsr && physical && (*bsr & 0x04U) != 0U
        && mac.configure_link(*physical)) status |= 0x02U;
    bool link_known = bsr.has_value();
    bool link_up = bsr && (*bsr & 0x04U) != 0U;
    if (link_known) mcuhw_nucleo_ethernet_link_transitions = 1U;

    protection_type protection{mmio};
    device_type device{mmio, protection};
    if (device.initialize()) {
        bool armed = true;
        for (std::size_t index = 0U; index < receive_buffers.size(); ++index) {
            armed = device.arm_receive(index, receive_buffers[index]) && armed;
        }
        if (armed) status |= 0x04U;
    }
    auto created = host_type::create(mac_address, ipv4_configuration);
    if (created) status |= 0x08U;
    mcuhw_nucleo_ethernet_status = status;

    std::array<bool, device_type::tx_descriptor_count> tx_busy{};
    if (created) {
        auto host = std::move(*created);
        bool mac_enabled = link_up && (status & 0x02U) != 0U && mac.enable();
        const bool dma_ready = (status & 0x0CU) == 0x0CU;
        if (dma_ready) {
            device.start();
            for (std::uint32_t ticks = 0U; ticks < 4'096U; ++ticks) {
                service(device, host, mac, tx_busy, link_known, link_up,
                        mac_enabled, ticks);
                if (mcuhw_nucleo_ethernet_replies >= 2U
                    && mcuhw_nucleo_ethernet_rx_min <= 64U
                    && mcuhw_nucleo_ethernet_rx_max >= 1514U) break;
            }
        }

        capture_evidence(mmio);

        if (dma_ready) {
            bool traffic_evidence_captured =
                mcuhw_nucleo_ethernet_replies >= 2U
                && mcuhw_nucleo_ethernet_rx_min <= 64U
                && mcuhw_nucleo_ethernet_rx_max >= 1514U;
            for (std::uint32_t ticks = 4'096U;; ++ticks) {
                service(device, host, mac, tx_busy, link_known, link_up,
                        mac_enabled, ticks);
                if (!traffic_evidence_captured
                    && mcuhw_nucleo_ethernet_replies >= 2U
                    && mcuhw_nucleo_ethernet_rx_min <= 64U
                    && mcuhw_nucleo_ethernet_rx_max >= 1514U) {
                    capture_evidence(mmio);
                    traffic_evidence_captured = true;
                }
            }
        }
    }

    for (;;) mcuhw::arch::instruction_barrier();
}
