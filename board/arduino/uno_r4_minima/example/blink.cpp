#include <chrono>
#include <mcubsp/io.hpp>
#include <mcubsp/io/board/uno_r4_minima.hpp>

namespace io = mcubsp::io;
using namespace std::chrono_literals;
constexpr io::board_config configuration{.serial = false};
static io::board<io::boards::uno_r4_minima, configuration> hw;


int main() {
    if (!hw.startup_ok()) __builtin_trap();
    io::require(hw.begin());
    auto led = io::require(hw.output(hw.pins.led_builtin));
    for (;;) {
        io::require(led.toggle());
        io::require(hw.delay(500ms));
    }
}
