#include <mcuhw/mcu/stm32h753/description.hpp>

using invalid_binding = mcuhw::core::pin_binding<
    mcuhw::mcu::stm32h753::lqfp100_pin_database,
    mcuhw::core::pin_id{10, 0},
    mcuhw::mcu::stm32h753::signal_id(
        mcuhw::mcu::stm32h753::signal::gpio_output),
    mcuhw::core::electrical_mode::push_pull>;

invalid_binding rejected;
