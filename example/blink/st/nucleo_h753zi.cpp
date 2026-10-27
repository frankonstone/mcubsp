// @req-file REQ-0045 REQ-0049
#include <chrono>
#include <mcubsp/io/board.hpp>
#include <mcubsp/board/st/nucleo_h753zi.hpp>

namespace io = mcubsp::io;
static io::board<mcubsp::board::st::nucleo_h753zi, {.serial = false}> hw;

int main() {
    if (!hw.startup_ok()) __builtin_trap();
    io::require(hw.begin());
    auto led = io::require(hw.output(hw.pins.led_builtin));
    for (;;) {
        io::require(led.toggle());
        io::require(hw.delay(std::chrono::milliseconds{500}));
    }
}
