#pragma once

// @req-file REQ-0002 REQ-0009 REQ-0035 REQ-0044 REQ-0046
#include <mcuhw/device/ethernet/w5100.hpp>

#if __has_include(<mcuip/target.hpp>)
#include <mcuip/target.hpp>

namespace mcuhw::integration::mcuip {

// Board-independent attached-device composition. MCUIP supplies caller-owned
// frame buffers; the W5100's internal packet RAM needs no native MAC DMA region.
template<typename SpiDevice, std::size_t RxDescriptors = 2U,
         std::size_t TxDescriptors = 2U>
using w5100_network_device =
    device::w5100<SpiDevice, RxDescriptors, TxDescriptors>;

template<typename SpiDevice, std::size_t RxDescriptors = 2U,
         std::size_t TxDescriptors = 2U>
inline constexpr bool w5100_valid_contract =
    ::mcuip::NetworkDevice<w5100_network_device<
        SpiDevice, RxDescriptors, TxDescriptors>>;

template<typename RegisterBus, std::size_t RxDescriptors = 2U,
         std::size_t TxDescriptors = 2U>
using w5100_macraw_network_device =
    device::w5100_macraw_device<RegisterBus, RxDescriptors, TxDescriptors>;

} // namespace mcuhw::integration::mcuip
#endif
