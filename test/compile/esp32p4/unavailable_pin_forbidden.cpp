#include <mcubsp/mcu/esp32p4/description.hpp>

using invalid_binding = mcubsp::core::pin_binding<
    mcubsp::mcu::esp32p4::qfn104_pin_database,
    mcubsp::core::pin_id{0, 55},
    mcubsp::mcu::esp32p4::signal_id(
        mcubsp::mcu::esp32p4::signal::gpio_output),
    mcubsp::core::electrical_mode::push_pull>;

invalid_binding rejected;
