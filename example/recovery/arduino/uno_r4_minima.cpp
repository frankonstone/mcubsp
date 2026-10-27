// @req-file REQ-0018 REQ-0045
#include <array>
#include <chrono>
#include <mcubsp/io/board.hpp>
#include <mcubsp/board/arduino/uno_r4_minima.hpp>
namespace io = mcubsp::io;
using namespace std::chrono_literals;
static io::board<mcubsp::board::arduino::uno_r4_minima> hw;

int main() {
    if (!hw.startup_ok()) __builtin_trap();
    io::require(hw.begin());
    io::require(hw.serial.begin(115200));
    // D0/D1 are physical UART pins; USB CDC is not selected by this profile.
    std::array<std::uint8_t, 1> byte{};
    for (;;) {
        auto operation = io::require(hw.serial.read_async(byte));
        const auto received = operation.finish(100ms);
        if (!received && !operation.ready()) {
            // Context/timeout validation errors can leave the buffer borrowed.
            // Quiesce before letting either buffer or handle leave scope.
            io::require(operation.cancel());
            static_cast<void>(operation.take());
        }
        if (received) {
            if (byte[0] == 'q') break;
            io::require(hw.serial.write(byte, 100ms));
        } else if (received.error().code != io::errc::timeout
                   && received.error().code != io::errc::cancelled) {
            // An unrecoverable quiesce failure is escalated by require().
            io::require(hw.serial.shutdown());
            io::require(hw.serial.begin(115200));
        }
    }
    io::require(hw.serial.shutdown());
    io::require(hw.shutdown());
    for (;;) {}
}
