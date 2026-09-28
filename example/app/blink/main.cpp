// @req-file REQ-0049
#include <chrono>
#include <mcuhw/io.hpp>
#include <mcuhw/io/board/uno_r4_minima.hpp>

namespace io = mcuhw::io;
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
