// @req-file REQ-0049
#include <chrono>
#include <mcuhw/io.hpp>
#include "selected_board.hpp"

namespace io = mcuhw::io;
using namespace std::chrono_literals;
constexpr io::board_config configuration{.serial = false};
static io::board<mcuhw::app::board_profile, configuration> hw;

int main() {
    if (!mcuhw::app::startup_ok()) __builtin_trap();
    io::require(hw.begin());
    auto led = io::require(hw.output(mcuhw::app::led_pin));
    for (;;) {
        io::require(led.toggle());
        io::require(hw.delay(500ms));
    }
}
