#pragma once

// @req-file REQ-0002 REQ-0003 REQ-0009 REQ-0013 REQ-0034 REQ-0043

#include <mcuhw/backend/esp32p4/ethernet.hpp>

#if __has_include(<mcuip/hardware.hpp>) && __has_include(<mcuip/target.hpp>)
#include <mcuip/hardware.hpp>
#include <mcuip/target.hpp>

namespace mcuhw::integration::mcuip {

inline constexpr ::mcuip::HardwareDescription esp32p4_nano_hardware{
    .dma_region = {
        .begin = 0x4FF0'0000U,
        .size = 768U * 1024U,
        .alignment = 64U,
        .capabilities = ::mcuip::MemoryCapability::dma
            | ::mcuip::MemoryCapability::internal,
    },
    .cache_line = 64U,
    .dma_alignment = 64U,
    .rx_descriptors = 4U,
    .tx_descriptors = 4U,
    .control_tx_descriptors = 1U,
    .cache_maintenance_required = true,
};

using esp32p4_nano_validated_hardware =
    ::mcuip::ValidatedHardware<esp32p4_nano_hardware>;

template<typename MmioPolicy, typename Protection>
using esp32p4_network_device =
    backend::esp32p4::ethernet_dma_device<MmioPolicy, Protection, 4U, 4U>;

template<typename MmioPolicy, typename Protection>
inline constexpr bool esp32p4_valid_contract =
    ::mcuip::NetworkDevice<esp32p4_network_device<MmioPolicy, Protection>>
    && ::mcuip::DescriptorProtection<Protection>;

static_assert(::mcuip::valid_hardware(esp32p4_nano_hardware));

} // namespace mcuhw::integration::mcuip
#endif
