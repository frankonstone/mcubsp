// @req-file REQ-0046
#include <array>
#include <chrono>
#include <utility>
#include <mcuhw/io.hpp>
#include <mcuhw/io/boards/uno_r4_minima.hpp>
#include <mcuhw/device/ethernet/w5100.hpp>

namespace io = mcuhw::io;
using namespace std::chrono_literals;
constexpr io::board_config configuration{.serial = true, .spi = true};
static io::board<io::boards::uno_r4_minima, configuration> hw;
static std::array<std::byte, 1536> receive;

// Ethernet Shield R3: D10 W5100 CS; D4 SD CS must remain high.
// Reports received raw Ethernet frames on D0/D1 serial. To process IPv4, give
// this same device to the MCUIP composition described in the components guide.
int main() {
    if (!hw.startup_ok()) __builtin_trap();
    io::require(hw.begin());
    io::require(hw.serial.begin());
    auto sd_select = io::require(hw.output(hw.pins.d4, true));
    io::require(hw.spi.begin());
    auto connection = io::require(hw.spi.device({.cs = hw.pins.d10, .frequency = 6'000'000U}));
    mcuhw::device::w5100 ethernet{std::move(connection)};
    if (!ethernet.begin({.mac = {{0x02, 0x00, 0x00, 0x04, 0x51, 0x00}}})) __builtin_trap();
    if (!ethernet.arm_receive(0U, receive)) __builtin_trap();
    for (;;) {
        auto limit = io::require(hw.deadline_after(10ms));
        if (!ethernet.poll_until(limit)) {
            if (!ethernet.recover()) __builtin_trap();
        }
        if (auto frame = ethernet.take_receive(0U)) {
            io::require(hw.serial.println(frame->error ? "receive error" : "Ethernet frame received"));
            if (!ethernet.arm_receive(0U, receive)) __builtin_trap();
        }
        io::require(hw.delay(1ms));
    }
}
