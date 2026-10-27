#include <chrono>
#include <mcubsp/io/board.hpp>
#include <mcubsp/board/others/promicro_nrf52840.hpp>

// @req-file REQ-0053
namespace io = mcubsp::io;
static io::board<mcubsp::board::others::promicro_nrf52840, {.serial = false}> hw;

int main() {
    if (!hw.startup_ok()) __builtin_trap();
    io::require(hw.begin());
    auto led = io::require(hw.output(hw.pins.led_builtin));
    for (;;) {
        io::require(led.toggle());
        io::require(hw.delay(std::chrono::milliseconds{500}));
    }
}
