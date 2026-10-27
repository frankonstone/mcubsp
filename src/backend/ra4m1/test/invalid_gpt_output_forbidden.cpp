#include <mcubsp/backend/ra4m1/gpt.hpp>
#include <mcubsp/board/arduino/uno_r4_minima/bindings.hpp>

using invalid_output = mcubsp::backend::ra4m1::gpt_binding<
    1,
    static_cast<mcubsp::mcu::ra4m1::gpt_output>(2),
    mcubsp::board::arduino::uno_r4_minima_bindings::d2>;

invalid_output rejected;
