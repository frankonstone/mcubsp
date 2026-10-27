#pragma once

// @req-file REQ-0053
#include <mcubsp/core/metadata.hpp>

namespace mcubsp::board::others {

// Native port names avoid incompatible Arduino-numbering conventions in clones.
// Initial alias: Zephyr promicro_nrf52840 LED0 is active-high P0.15.
struct promicro_nrf52840_pins {
    static constexpr core::pin_id p0_15{0, 15};
    static constexpr auto led_builtin = p0_15;
    static constexpr bool led_active_high = true;
};

} // namespace mcubsp::board::others
