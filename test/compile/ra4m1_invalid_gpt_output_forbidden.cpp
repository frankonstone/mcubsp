#include <mcuhw/backend/ra4m1/gpt.hpp>
#include <mcuhw/board/arduino_uno_r4_minima.hpp>

using invalid_output = mcuhw::backend::ra4m1::gpt_binding<
    1,
    static_cast<mcuhw::mcu::ra4m1::gpt_output>(2),
    mcuhw::board::arduino_uno_r4_minima::d2>;

invalid_output rejected;
