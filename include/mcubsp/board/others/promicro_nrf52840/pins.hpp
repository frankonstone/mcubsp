#pragma once

// @req-file REQ-0053 REQ-0054
#include <mcubsp/core/metadata.hpp>

namespace mcubsp::board::others {

// Native port names avoid incompatible Arduino-numbering conventions in clones.
// Initial alias: Zephyr promicro_nrf52840 LED0 is active-high P0.15.
struct promicro_nrf52840_pins {
    // Zephyr's default I2C0 route, using the physical native pin labels.
    static constexpr core::pin_id p0_11{0, 11};
    static constexpr core::pin_id p1_00{1, 0};
    static constexpr auto scl = p0_11;
    static constexpr auto sda = p1_00;
    static constexpr core::pin_id p0_15{0, 15};
    static constexpr auto led_builtin = p0_15;
    static constexpr bool led_active_high = true;
};

} // namespace mcubsp::board::others
