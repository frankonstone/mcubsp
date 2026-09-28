#include <mcuhw/core/metadata.hpp>
#include <mcuhw/mcu/ra4m1/description.hpp>

using invalid_binding = mcuhw::core::pin_binding<
    mcuhw::mcu::ra4m1::pin_database,
    mcuhw::core::pin_id{6, 0},
    mcuhw::mcu::ra4m1::signal_id(mcuhw::mcu::ra4m1::signal::gpio_output),
    mcuhw::core::electrical_mode::push_pull>;

invalid_binding rejected;
