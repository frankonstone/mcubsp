#pragma once

#include <cstdint>
#include <mcubsp/core/metadata.hpp>

namespace mcubsp::board::arduino {

struct uno_r4_minima_pins {
    // Arduino digital-header composition, independently source checked against
    // ArduinoCore-renesas 1.6.0 variants/MINIMA/variant.cpp.
    static constexpr core::pin_id d0_rx{3, 1};
    static constexpr core::pin_id d1_tx{3, 2};
    static constexpr core::pin_id d2{1, 5};
    static constexpr core::pin_id d3{1, 4};
    // Ethernet Shield R3 SD select; same P103 binding as board Ethernet tests.
    static constexpr core::pin_id d4{1, 3};
    static constexpr core::pin_id d10_ssl{1, 12};
    static constexpr core::pin_id d11_mosi{1, 9};
    static constexpr core::pin_id d12_miso{1, 10};
    static constexpr core::pin_id led_builtin_d13{1, 11};
    static constexpr core::pin_id d13_sck = led_builtin_d13;
    static constexpr core::pin_id a0_an009{0, 14};
    static constexpr core::pin_id a1_an000{0, 0};
    static constexpr core::pin_id a2_an001{0, 1};
    static constexpr core::pin_id a4_sda{1, 1};
    static constexpr core::pin_id a5_scl{1, 0};
};

} // namespace mcubsp::board::arduino
