#pragma once

// @req-file REQ-0002 REQ-0003 REQ-0009 REQ-0013 REQ-0043

#include <mcubsp/backend/stm32h753/ethernet.hpp>

#if __has_include(<mcuip/hardware.hpp>) && __has_include(<mcuip/target.hpp>)
#include <mcuip/hardware.hpp>
#include <mcuip/target.hpp>

namespace mcubsp::integration::mcuip {

inline constexpr ::mcuip::HardwareDescription nucleo_h753zi_hardware{
    .dma_region = {
        .begin = 0x2400'0000U,
        .size = 512U * 1024U,
        .alignment = 32U,
        .capabilities = ::mcuip::MemoryCapability::dma
            | ::mcuip::MemoryCapability::internal,
    },
    .cache_line = 32U,
    .dma_alignment = 32U,
    .rx_descriptors = 4U,
    .tx_descriptors = 4U,
    .control_tx_descriptors = 1U,
    .cache_maintenance_required = true,
};

using nucleo_h753zi_validated_hardware =
    ::mcuip::ValidatedHardware<nucleo_h753zi_hardware>;

template<typename MmioPolicy>
using descriptor_protection =
    backend::stm32h753::ethernet_descriptor_protection<MmioPolicy>;

template<typename MmioPolicy, typename Protection>
using network_device =
    backend::stm32h753::ethernet_dma_device<MmioPolicy, Protection, 4U, 4U>;

template<typename MmioPolicy, typename Protection>
inline constexpr bool valid_contract =
    ::mcuip::NetworkDevice<network_device<MmioPolicy, Protection>>
    && ::mcuip::DescriptorProtection<Protection>;

static_assert(::mcuip::valid_hardware(nucleo_h753zi_hardware));

} // namespace mcubsp::integration::mcuip
#endif
