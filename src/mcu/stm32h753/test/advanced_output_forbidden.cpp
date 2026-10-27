#include <mcubsp/mcu/stm32h753/description.hpp>

using invalid_capability = mcubsp::core::capability_request<
    mcubsp::mcu::stm32h753::general_timer32_capabilities,
    mcubsp::mcu::stm32h753::peripheral_capability::dead_time>;

invalid_capability rejected;
