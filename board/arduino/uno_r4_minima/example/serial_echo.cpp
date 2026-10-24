#include <array>
#include <chrono>
#include <mcubsp/io.hpp>
#include <mcubsp/io/board/uno_r4_minima.hpp>

namespace io = mcubsp::io;
using namespace std::chrono_literals;
static io::board<io::boards::uno_r4_minima> hw;

// UART on D0/D1, 115200 8N1. Use a voltage-compatible UART peer and common GND.
// This is not a USB-CDC Serial implementation.
int main() {
    if (!hw.startup_ok()) __builtin_trap();
    io::require(hw.begin());
    io::require(hw.serial.begin(115200));
    io::require(hw.serial.println("mcubsp::io ready"));
    std::array<std::uint8_t, 1> byte{};
    for (;;) {
        auto read = hw.serial.read(byte, 100ms);
        if (!read && read.error().code == io::errc::timeout) continue;
        static_cast<void>(io::require(std::move(read)));
        io::require(hw.serial.write(byte, 100ms));
    }
}
