#include <mcuhw/mcu/stm32h753/description.hpp>

using invalid_capability = mcuhw::core::capability_request<
    mcuhw::mcu::stm32h753::general_timer32_capabilities,
    mcuhw::mcu::stm32h753::peripheral_capability::dead_time>;

invalid_capability rejected;
