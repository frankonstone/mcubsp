#include <mcuhw/mcu/esp32p4/description.hpp>

using invalid_binding = mcuhw::core::pin_binding<
    mcuhw::mcu::esp32p4::qfn104_pin_database,
    mcuhw::core::pin_id{0, 55},
    mcuhw::mcu::esp32p4::signal_id(
        mcuhw::mcu::esp32p4::signal::gpio_output),
    mcuhw::core::electrical_mode::push_pull>;

invalid_binding rejected;
