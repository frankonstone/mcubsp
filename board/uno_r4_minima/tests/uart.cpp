#include <array>
#include <cstdint>
#include <utility>

#include <mcutest/mcutest.hpp>

#include <mcuhw/backend/ra4m1/sci_uart.hpp>
#include <mcuhw/backend/ra4m1/dmac.hpp>
#include <mcuhw/board/arduino_uno_r4_minima.hpp>
#include <mcuhw/driver/uart.hpp>

#include "support.hpp"

namespace {

using binding = mcuhw::backend::ra4m1::sci_uart_binding<
    2, mcuhw::board::arduino_uno_r4_minima::d0_rx,
    mcuhw::board::arduino_uno_r4_minima::d1_tx>;
using backend_type = mcuhw::backend::ra4m1::sci_uart_backend<
    binding, mcuhw::core::direct_mmio, mcuhw::board_test::execution_policy>;
using dmac1_type = mcuhw::backend::ra4m1::dmac_channel_backend<
    1, mcuhw::core::direct_mmio, mcuhw::board_test::execution_policy>;

mcuhw::board_test::execution_policy execution;
backend_type backend{mcuhw::board_test::mmio, execution};
dmac1_type transmit_dmac{mcuhw::board_test::mmio, execution};
mcuhw::driver::uart uart{backend};
bool initialized{};
bool interrupt_shutdown{};
constexpr std::array<std::uint8_t, 8> transmitted_bytes{
    0x55U, 0xAAU, 0x00U, 0xFFU, 0x31U, 0x7EU, 0x80U, 0x19U};
bool transmit_completed{};

constexpr std::array<std::uint8_t, 16> dma_transmitted_bytes{
    0x44U, 0x4DU, 0x41U, 0x2DU, 0x55U, 0x41U, 0x52U, 0x54U,
    0x00U, 0xFFU, 0xA5U, 0x5AU, 0x11U, 0x22U, 0x33U, 0x7EU,
};
bool dma_initialized{};
bool dma_transmit_completed{};
bool dma_shutdown{};
bool dma_interrupt_mode{};
volatile std::uint32_t transmit_dma_interrupts{};
volatile std::uint32_t dma_tei_interrupts{};
volatile std::uint32_t dma_error_interrupts{};

constexpr auto timing = mcuhw::driver::solve_uart_baud({
    .clock_hz = 48'000'000U,
    .target_baud = 115'200U,
    .maximum_error_ppm = 2'000U,
});
static_assert(timing.has_value());

void configure_interrupts(bool dma_mode) noexcept {
    const std::array events{
        dma_mode ? mcuhw::mcu::ra4m1::event::dmac0_int
                 : mcuhw::mcu::ra4m1::event::sci2_rxi,
        dma_mode ? mcuhw::mcu::ra4m1::event::dmac1_int
                 : mcuhw::mcu::ra4m1::event::sci2_txi,
        mcuhw::mcu::ra4m1::event::sci2_tei,
        mcuhw::mcu::ra4m1::event::sci2_eri,
    };
    *reinterpret_cast<volatile std::uint32_t*>(0xE000'E180U) = 0x0FU;
    for (std::uint32_t line = 0U; line < events.size(); ++line) {
        *reinterpret_cast<volatile std::uint32_t*>(0x4000'6300U + line * 4U)
            = mcuhw::mcu::ra4m1::event_number(events[line]);
        *reinterpret_cast<volatile std::uint8_t*>(0xE000'E400U + line)
            = 0x80U;
    }
    *reinterpret_cast<volatile std::uint32_t*>(0xE000'E280U) = 0x0FU;
    *reinterpret_cast<volatile std::uint32_t*>(0xE000'E100U) = 0x0FU;
    __asm__ volatile("dsb sy\n\tisb sy\n\tcpsie i" ::: "memory");
}

TEST(uno_uart, sci2_d1_irq_transmit_and_final_wire_completion,
     "REQ-0027", "REQ-0036") {
    ASSERT_TRUE(initialized);
    auto transmit_start = uart.transmit(transmitted_bytes);
    ASSERT_TRUE(transmit_start.has_value());
    auto transmit = std::move(*transmit_start);
    const auto deadline = mcuhw::execution::deadline{
        static_cast<std::uint32_t>(
            mcuhw::board_test::execution_hooks::now() + 480'000U)};
    ASSERT_TRUE(transmit.wait_until(deadline).has_value());
    transmit_completed = transmit.take().has_value();
    ASSERT_TRUE(transmit_completed);
    interrupt_shutdown = uart.shutdown().has_value();
    ASSERT_TRUE(interrupt_shutdown);
}

TEST(uno_uart, sci2_d1_dma_transmit_and_final_wire_completion,
     "REQ-0017", "REQ-0018", "REQ-0019", "REQ-0027", "REQ-0030",
     "REQ-0036") {
    dma_interrupt_mode = true;
    configure_interrupts(true);
    dma_initialized = transmit_dmac.initialize().has_value()
        && uart.initialize(*timing, {}).has_value();
    ASSERT_TRUE(dma_initialized);
    auto transmit_start = uart.transmit_dma(
        dma_transmitted_bytes, transmit_dmac);
    ASSERT_TRUE(transmit_start.has_value());
    auto transmit = std::move(*transmit_start);
    const auto deadline = mcuhw::execution::deadline{
        static_cast<std::uint32_t>(
            mcuhw::board_test::execution_hooks::now() + 480'000U)};
    ASSERT_TRUE(transmit.wait_until(deadline).has_value());
    dma_transmit_completed = transmit.take().has_value();
    ASSERT_TRUE(dma_transmit_completed);
    ASSERT_EQ(transmit_dma_interrupts, 1U);
    ASSERT_EQ(dma_tei_interrupts, 1U);
    ASSERT_EQ(dma_error_interrupts, 0U);
    dma_shutdown = uart.shutdown().has_value()
        && transmit_dmac.shutdown().has_value();
    ASSERT_TRUE(dma_shutdown);
}

constexpr char metadata[] =
    "MCUHW_META board=Arduino_UNO_R4_Minima suite=uart "
    "firmware=pln-0010 fixture=D1_SCI2_TX_header_unloaded "
    "baud_target=115200 baud_achieved=115385 tolerance_ppm=2000 "
    "dma=DMAC1_TX activation=SCI2_TXI completion=DMAC1_INT,SCI2_TEI "
    "toolchain=arm-none-eabi-gcc-15.2.1 "
    "iclk_hz=48000000 pclka_hz=48000000 pclkb_hz=24000000 "
    "pclkd_hz=48000000 interface=J-Link_SWD_RTT\n";
constexpr mcuhw::board_test::evidence_metadata evidence{
    .suite = "uart",
    .firmware = "pln-0010",
    .fixture = "D1 SCI2 TX header; no receiver or electrical capture attached",
    .coverage_build_id = 0x4d485701U,
};

} // namespace

int main() {
    mcuhw::board_test::start(metadata);
    initialized = uart.initialize(*timing, {}).has_value();
    if (initialized) {
        configure_interrupts(false);
    }
    mcuhw::board_test::finish(evidence, [&](auto& observations) {
        observations("initialized", initialized);
        observations("peripheral", "SCI2 interrupt-driven UART");
        observations("baud_target", 115'200U);
        observations("baud_achieved", timing->achieved_baud);
        observations("completion_interrupt", "SCI2 TEI");
        observations("transmit_completed", transmit_completed);
        observations("interrupt_shutdown", interrupt_shutdown);
        observations.array("transmitted_bytes", [&](auto& bytes) {
            for (const auto byte : transmitted_bytes) {
                bytes(byte);
            }
        });
        observations.object("transmit_dma", [&](auto& out) {
            out("initialized", dma_initialized);
            out("transmit_completed_after_tei", dma_transmit_completed);
            out("bytes", static_cast<std::uint32_t>(dma_transmitted_bytes.size()));
            out("transmit_dma_interrupts", transmit_dma_interrupts);
            out("tei_interrupts", dma_tei_interrupts);
            out("error_interrupts", dma_error_interrupts);
            out("shutdown", dma_shutdown);
        });
    });
}

extern "C" void IRQ0_Handler() noexcept {
    if (dma_interrupt_mode) {
        // RX DMA is native-tested; this physical image has no receiver or
        // D1-to-D0 loopback attached.
    } else {
        static_cast<void>(backend.handle_receive_interrupt());
    }
    mcuhw::board_test::clear_icu_line(0U);
}

extern "C" void IRQ1_Handler() noexcept {
    if (dma_interrupt_mode) {
        transmit_dma_interrupts = transmit_dma_interrupts + 1U;
        static_cast<void>(transmit_dmac.handle_transfer_end_interrupt());
        static_cast<void>(backend.handle_dma_transmit_completion());
    } else {
        static_cast<void>(backend.handle_transmit_data_empty_interrupt());
    }
    mcuhw::board_test::clear_icu_line(1U);
}

extern "C" void IRQ2_Handler() noexcept {
    if (dma_interrupt_mode) {
        dma_tei_interrupts = dma_tei_interrupts + 1U;
    }
    static_cast<void>(backend.handle_transmit_end_interrupt());
    mcuhw::board_test::clear_icu_line(2U);
}

extern "C" void IRQ3_Handler() noexcept {
    if (dma_interrupt_mode) {
        dma_error_interrupts = dma_error_interrupts + 1U;
    }
    static_cast<void>(backend.handle_receive_error_interrupt());
    mcuhw::board_test::clear_icu_line(3U);
}
