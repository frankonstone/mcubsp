#include <mcubsp/core/metadata.hpp>
#include <mcubsp/mcu/ra4m1/description.hpp>

using invalid_binding = mcubsp::core::pin_binding<
    mcubsp::mcu::ra4m1::pin_database,
    mcubsp::core::pin_id{6, 0},
    mcubsp::mcu::ra4m1::signal_id(mcubsp::mcu::ra4m1::signal::gpio_output),
    mcubsp::core::electrical_mode::push_pull>;

invalid_binding rejected;
