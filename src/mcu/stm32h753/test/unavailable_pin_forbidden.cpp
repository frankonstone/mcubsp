#include <mcubsp/mcu/stm32h753/description.hpp>

using invalid_binding = mcubsp::core::pin_binding<
    mcubsp::mcu::stm32h753::lqfp100_pin_database,
    mcubsp::core::pin_id{10, 0},
    mcubsp::mcu::stm32h753::signal_id(
        mcubsp::mcu::stm32h753::signal::gpio_output),
    mcubsp::core::electrical_mode::push_pull>;

invalid_binding rejected;
