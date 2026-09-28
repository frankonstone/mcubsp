// @req-file REQ-0046 REQ-0049
#include <chrono>
#include <utility>
#include <mcuhw/io.hpp>
#include <mcuhw/io/board/uno_r4_minima.hpp>
#include <mcuhw/device/sensor/tmp102.hpp>

namespace io = mcuhw::io;
using namespace std::chrono_literals;
constexpr io::board_config configuration{.serial = false, .i2c = true};
static io::board<io::boards::uno_r4_minima, configuration> hw;

int main() {
    if (!hw.startup_ok()) __builtin_trap();
    io::require(hw.begin());
    io::require(hw.i2c.begin());
    auto led = io::require(hw.output(hw.pins.led_builtin));
    auto connection = io::require(hw.i2c.device(0x48));
    mcuhw::device::tmp102 sensor{std::move(connection)};
    if (!sensor.begin()) __builtin_trap();
    io::require(hw.delay(20ms));
    for (;;) {
        auto sample = sensor.read(100ms);
        if (!sample) __builtin_trap();
        io::require(led.write(sample->celsius_sixteenths > 25 * 16));
        io::require(hw.delay(500ms));
    }
}
