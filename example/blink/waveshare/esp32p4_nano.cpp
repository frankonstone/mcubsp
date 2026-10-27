// @req-file REQ-0045 REQ-0049
#include <chrono>
#include <mcubsp/io/board.hpp>
#include <mcubsp/board/waveshare/esp32p4_nano.hpp>

namespace io = mcubsp::io;
static io::board<mcubsp::board::waveshare::esp32p4_nano, {.serial = false}> hw;

int main() {
    if (!hw.startup_ok()) __builtin_trap();
    io::require(hw.begin());
    // Attach an LED and series resistor to expansion GPIO2 and ground.
    auto led = io::require(hw.output(hw.pins.gpio2));
    for (;;) {
        io::require(led.toggle());
        io::require(hw.delay(std::chrono::milliseconds{500}));
    }
}
