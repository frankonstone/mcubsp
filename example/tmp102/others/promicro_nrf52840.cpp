// @req-file REQ-0054
#include <chrono>
#include <utility>
#include <mcubsp/io/board.hpp>
#include <mcubsp/board/others/promicro_nrf52840.hpp>
#include <mcubsp/device/sensor/tmp102.hpp>

namespace io = mcubsp::io;
using namespace std::chrono_literals;
constexpr io::board_config configuration{.serial = false, .i2c = true};
static io::board<mcubsp::board::others::promicro_nrf52840, configuration> hw;

// Zephyr-compatible I2C0: SDA P1.00 (100), SCL P0.11 (011).
// TMP102 at 0x48, 3.3 V supply, common ground and external 3.3 V pull-ups.
int main() {
    if (!hw.startup_ok()) __builtin_trap();
    io::require(hw.begin());
    io::require(hw.i2c.begin(100'000));
    auto led = io::require(hw.output(hw.pins.led_builtin));
    auto connection = io::require(hw.i2c.device(0x48));
    mcubsp::device::tmp102 sensor{std::move(connection)};
    if (!sensor.begin(100ms)) __builtin_trap();
    io::require(hw.delay(20ms));
    for (;;) {
        const auto sample = sensor.read(100ms);
        if (!sample) __builtin_trap();
        io::require(led.write(sample->celsius_sixteenths > 25 * 16));
        io::require(hw.delay(500ms));
    }
}
