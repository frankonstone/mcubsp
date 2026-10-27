#pragma once
// @req-file REQ-0031 REQ-0033
#include <array>
#include <cstdint>
#include <string_view>
#include <mcubsp/board/st/nucleo_h753zi/pins.hpp>

namespace mcubsp::board::st {

// Board identity, clock/memory facts and connections; no I/O facade or backend
// implementation is required to use these bindings.
struct nucleo_h753zi_bindings : nucleo_h753zi_pins {
    static constexpr std::uint8_t ethernet_phy_address = 0U;

    struct ethernet_binding {
        static constexpr std::array rmii_pins{
            ethernet_ref_clk, ethernet_mdio, ethernet_mdc, ethernet_crs_dv,
            ethernet_rxd0, ethernet_rxd1, ethernet_tx_en, ethernet_txd0, ethernet_txd1};
    };
};

} // namespace mcubsp::board::st
