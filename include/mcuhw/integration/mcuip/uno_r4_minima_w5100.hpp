#pragma once

// @req-file REQ-0002 REQ-0009 REQ-0035 REQ-0044

#include <mcuhw/driver/w5100.hpp>

#if __has_include(<mcuip/target.hpp>)
#include <mcuip/target.hpp>

namespace mcuhw::integration::mcuip {

// The W5100 owns its packet memory, so this composition deliberately has no
// invented native-MAC descriptor protection or DMA-region description. MCUIP
// provides caller-owned frame buffers at this thin raw-frame boundary.
template<typename Bus, std::size_t RxDescriptors = 2U,
         std::size_t TxDescriptors = 2U>
using uno_r4_minima_w5100_network_device =
    driver::w5100_macraw_device<Bus, RxDescriptors, TxDescriptors>;

template<typename Bus, std::size_t RxDescriptors = 2U,
         std::size_t TxDescriptors = 2U>
inline constexpr bool uno_r4_minima_w5100_valid_contract =
    ::mcuip::NetworkDevice<uno_r4_minima_w5100_network_device<
        Bus, RxDescriptors, TxDescriptors>>;

} // namespace mcuhw::integration::mcuip
#endif
