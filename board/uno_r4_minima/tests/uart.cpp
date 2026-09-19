#include <array>
#include <cstdint>
#include <utility>

#include <mcutest/mcutest.hpp>

#include <mcuhw/backend/ra4m1/sci_uart.hpp>
#include <mcuhw/board/arduino_uno_r4_minima.hpp>
#include <mcuhw/driver/uart.hpp>

#include "support.hpp"

namespace {

using binding = mcuhw::backend::ra4m1::sci_uart_binding<
    2, mcuhw::board::arduino_uno_r4_minima::d0_rx,
    mcuhw::board::arduino_uno_r4_minima::d1_tx>;
using backend_type = mcuhw::backend::ra4m1::sci_uart_backend<
    binding, mcuhw::core::direct_mmio, mcuhw::board_test::execution_policy>;

mcuhw::board_test::execution_policy execution;
backend_type backend{mcuhw::board_test::mmio, execution};
mcuhw::driver::uart uart{backend};
bool initialized{};
constexpr std::array<std::uint8_t, 8> transmitted_bytes{
    0x55U, 0xAAU, 0x00U, 0xFFU, 0x31U, 0x7EU, 0x80U, 0x19U};
std::array<std::uint8_t, transmitted_bytes.size()> received_bytes{};
std::size_t received_count_observation{};
bool transmit_completed{};

constexpr auto timing = mcuhw::driver::solve_uart_baud({
    .clock_hz = 48'000'000U,
    .target_baud = 115'200U,
    .maximum_error_ppm = 2'000U,
});
static_assert(timing.has_value());

void configure_interrupts() noexcept {
    constexpr std::array<std::uint16_t, 4> events{
        mcuhw::mcu::ra4m1::event_number(mcuhw::mcu::ra4m1::event::sci2_rxi),
        mcuhw::mcu::ra4m1::event_number(mcuhw::mcu::ra4m1::event::sci2_txi),
        mcuhw::mcu::ra4m1::event_number(mcuhw::mcu::ra4m1::event::sci2_tei),
        mcuhw::mcu::ra4m1::event_number(mcuhw::mcu::ra4m1::event::sci2_eri),
    };
    *reinterpret_cast<volatile std::uint32_t*>(0xE000'E180U) = 0x0FU;
    for (std::uint32_t line = 0U; line < events.size(); ++line) {
        *reinterpret_cast<volatile std::uint32_t*>(0x4000'6300U + line * 4U)
            = events[line];
        *reinterpret_cast<volatile std::uint8_t*>(0xE000'E400U + line)
            = 0x80U;
    }
    *reinterpret_cast<volatile std::uint32_t*>(0xE000'E280U) = 0x0FU;
    *reinterpret_cast<volatile std::uint32_t*>(0xE000'E100U) = 0x0FU;
    __asm__ volatile("dsb sy\n\tisb sy\n\tcpsie i" ::: "memory");
}

TEST(uno_uart, sci2_d1_to_d0_irq_loopback_and_final_wire_completion,
     "REQ-0027", "REQ-0036") {
    ASSERT_TRUE(initialized);
    auto receive_start = uart.receive(received_bytes);
    ASSERT_TRUE(receive_start.has_value());
    auto receive = std::move(*receive_start);
    auto transmit_start = uart.transmit(transmitted_bytes);
    ASSERT_TRUE(transmit_start.has_value());
    auto transmit = std::move(*transmit_start);
    const auto deadline = mcuhw::execution::deadline{
        static_cast<std::uint32_t>(
            mcuhw::board_test::execution_hooks::now() + 480'000U)};
    ASSERT_TRUE(receive.wait_until(deadline).has_value());
    const auto received_count = receive.take();
    ASSERT_TRUE(received_count.has_value());
    received_count_observation = *received_count;
    EXPECT_EQ(received_count_observation, received_bytes.size());
    ASSERT_TRUE(transmit.wait_until(deadline).has_value());
    transmit_completed = transmit.take().has_value();
    ASSERT_TRUE(transmit_completed);
    for (std::size_t index = 0; index < transmitted_bytes.size(); ++index) {
        EXPECT_EQ(received_bytes[index], transmitted_bytes[index]);
    }
}

constexpr char metadata[] =
    "MCUHW_META board=Arduino_UNO_R4_Minima suite=uart "
    "firmware=pln-0008 fixture=D1_SCI2_TX_to_D0_SCI2_RX "
    "baud_target=115200 baud_achieved=115385 tolerance_ppm=2000 "
    "completion=SCI2_TEI toolchain=arm-none-eabi-gcc-15.2.1 "
    "iclk_hz=48000000 pclka_hz=48000000 pclkb_hz=24000000 "
    "pclkd_hz=48000000 interface=J-Link_SWD_RTT\n";
constexpr mcuhw::board_test::evidence_metadata evidence{
    .suite = "uart",
    .firmware = "pln-0008",
    .fixture = "D1 SCI2 TX jumpered to D0 SCI2 RX",
    .coverage_build_id = 0x4d485701U,
};

} // namespace

int main() {
    mcuhw::board_test::start(metadata);
    initialized = uart.initialize(*timing, {}).has_value();
    if (initialized) {
        configure_interrupts();
    }
    mcuhw::board_test::finish(evidence, [&](auto& observations) {
        observations("initialized", initialized);
        observations("peripheral", "SCI2 interrupt-driven UART");
        observations("baud_target", 115'200U);
        observations("baud_achieved", timing->achieved_baud);
        observations("completion_interrupt", "SCI2 TEI");
        observations("transmit_completed", transmit_completed);
        observations("received_count", received_count_observation);
        observations.array("transmitted_bytes", [&](auto& bytes) {
            for (const auto byte : transmitted_bytes) {
                bytes(byte);
            }
        });
        observations.array("received_bytes", [&](auto& bytes) {
            for (const auto byte : received_bytes) {
                bytes(byte);
            }
        });
    });
}

extern "C" void IRQ0_Handler() noexcept {
    static_cast<void>(backend.handle_receive_interrupt());
    mcuhw::board_test::clear_icu_line(0U);
}

extern "C" void IRQ1_Handler() noexcept {
    static_cast<void>(backend.handle_transmit_data_empty_interrupt());
    mcuhw::board_test::clear_icu_line(1U);
}

extern "C" void IRQ2_Handler() noexcept {
    static_cast<void>(backend.handle_transmit_end_interrupt());
    mcuhw::board_test::clear_icu_line(2U);
}

extern "C" void IRQ3_Handler() noexcept {
    static_cast<void>(backend.handle_receive_error_interrupt());
    mcuhw::board_test::clear_icu_line(3U);
}
