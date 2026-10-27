#include <array>
#include <chrono>
#include <mcubsp/io/board.hpp>
#include <mcubsp/board/arduino/uno_r4_minima.hpp>

namespace io = mcubsp::io;
using namespace std::chrono_literals;
constexpr io::board_config configuration{
    .serial = false, .spi = true, .dma = io::channels<0, 1>,
};
static io::board<mcubsp::board::arduino::uno_r4_minima, configuration> hw;
// RAM, not const data in flash: caller-owned DMA buffers remain alive.
alignas(4) static std::array<std::uint8_t, 4> transmit{0x12, 0x34, 0x56, 0x78};
alignas(4) static std::array<std::uint8_t, 4> receive{};

// SPI1: D10 CS, D11 MOSI, D12 MISO, D13 clock. D11-to-D12 loopback can be
// used without a slave. Completion includes both DMA channels and SPI idle.
int main() {
    if (!hw.startup_ok()) __builtin_trap();
    io::require(hw.begin());
    auto rx_dma = io::require(hw.dma.channel<0>());
    auto tx_dma = io::require(hw.dma.channel<1>());
    io::require(hw.spi.begin(1'000'000));
    auto operation = io::require(hw.spi.transfer_async(
        {.transmit = transmit, .receive = receive}, io::use_dma(rx_dma, tx_dma)));
    static_cast<void>(io::require(operation.finish(100ms)));
    io::require(rx_dma.release());
    io::require(tx_dma.release());
    io::require(hw.shutdown());
    for (;;) { __asm__ volatile("nop"); }
}
