#include "common.hpp"

#include <mcuhw/backend/ra4m1/dmac.hpp>
#include <mcuhw/backend/ra4m1/spi.hpp>
#include <mcuhw/driver/spi.hpp>

namespace cost {
// Both directions use SRAM buffers accepted by the same DMA validation.
alignas(4) std::array<std::uint8_t, 16> transmitted{
    0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70, 0x80,
    0x90, 0xA0, 0xB0, 0xC0, 0xD0, 0xE0, 0xF0, 0xFF,
};
alignas(4) std::array<std::uint8_t, 16> received{};

#if MCUHW_IO_COST_FACADE
constexpr io::board_config configuration{
    .serial = false, .spi = true, .dma = io::channels<0, 1>,
};
auto hardware = io::make_board<board_type, configuration>(execution, mmio,
                                                         48'000'000U);
void progress() noexcept {}
#else
using binding = mcuhw::backend::ra4m1::spi_binding<
    1, board_type::pins_type::miso, board_type::pins_type::mosi,
    board_type::pins_type::sck, board_type::pins_type::cs>;
mcuhw::backend::ra4m1::spi_master_backend<binding, mmio_type, execution_type>
    backend{mmio, execution};
mcuhw::driver::spi_controller spi{backend};
mcuhw::backend::ra4m1::dmac_channel_backend<0, mmio_type, execution_type>
    receive_dma{mmio, execution};
mcuhw::backend::ra4m1::dmac_channel_backend<1, mmio_type, execution_type>
    transmit_dma{mmio, execution};
void progress() noexcept {
    board_type::poll_dma<0>(mmio, receive_dma);
    board_type::poll_dma<1>(mmio, transmit_dma);
    board_type::poll_spi(mmio, backend);
}
#endif
} // namespace cost

extern "C" void mcuhw_io_cost_entry() noexcept {
    using namespace cost;
    using namespace std::chrono_literals;
#if MCUHW_IO_COST_FACADE
    check(hardware.begin().has_value());
    check(hardware.spi.begin(1'000'000U).has_value());
    auto receive_channel = hardware.dma.channel<0>();
    auto transmit_channel = hardware.dma.channel<1>();
    check(receive_channel.has_value() && transmit_channel.has_value());
    auto started = hardware.spi.transfer_async(
        {transmitted, received}, io::use_dma(*receive_channel, *transmit_channel));
    check(started.has_value());
    auto transfer = std::move(*started);
    other_work();
    observation = transfer.finish(100ms).has_value();
#else
    board_type::start(mmio);
    const auto timing = board_type::solve_spi(1'000'000U);
    check(timing.has_value());
    check(spi.initialize(*timing).has_value());
    check(receive_dma.initialize().has_value());
    check(transmit_dma.initialize().has_value());
    auto started = spi.transaction_dma({transmitted, received}, receive_dma,
                                       transmit_dma);
    check(started.has_value());
    auto transfer = std::move(*started);
    other_work();
    finish_driver(transfer);
#endif
    for (;;) __asm__ volatile("");
}
