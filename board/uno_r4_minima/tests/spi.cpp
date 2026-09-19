#include <array>
#include <cstddef>
#include <cstdint>
#include <utility>

#include <mcutest/mcutest.hpp>

#include <mcuhw/backend/ra4m1/spi.hpp>
#include <mcuhw/board/arduino_uno_r4_minima.hpp>

#include "support.hpp"

namespace {
namespace board = mcuhw::board::arduino_uno_r4_minima;
using binding = mcuhw::backend::ra4m1::spi_binding<
    1, board::d12_miso, board::d11_mosi, board::d13_sck, board::d10_ssl>;
using backend_type = mcuhw::backend::ra4m1::spi_master_backend<
    binding, mcuhw::core::direct_mmio, mcuhw::board_test::execution_policy>;

mcuhw::board_test::execution_policy execution;
backend_type backend{mcuhw::board_test::mmio, execution};

constexpr auto timing_1mhz = mcuhw::driver::solve_spi_timing(
    {48'000'000U, 1'000'000U, 0U});
constexpr auto timing_6mhz = mcuhw::driver::solve_spi_timing(
    {48'000'000U, 6'000'000U, 0U});
static_assert(timing_1mhz.has_value() && timing_6mhz.has_value());

constexpr std::array<std::uint16_t, 6> pattern{
    0x00U, 0xFFU, 0x96U, 0x69U, 0x3CU, 0xC3U};

struct observation {
    bool initialized{};
    bool completed{};
    bool matched{};
    std::size_t frames{};
    std::array<std::uint16_t, pattern.size()> received{};
    std::array<std::uint16_t, 2> dummy_received{};
};

observation mode0_observation{};
observation mode3_observation{};

void configure_interrupts() noexcept {
    constexpr std::array<std::uint16_t, 4> events{
        mcuhw::mcu::ra4m1::event_number(mcuhw::mcu::ra4m1::event::spi1_rxi),
        mcuhw::mcu::ra4m1::event_number(mcuhw::mcu::ra4m1::event::spi1_txi),
        mcuhw::mcu::ra4m1::event_number(mcuhw::mcu::ra4m1::event::spi1_idle),
        mcuhw::mcu::ra4m1::event_number(mcuhw::mcu::ra4m1::event::spi1_error),
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

[[nodiscard]] bool exercise(const mcuhw::driver::spi_timing& timing,
                            mcuhw::driver::spi_mode mode,
                            mcuhw::driver::spi_bit_order order,
                            observation& result) noexcept {
    result.initialized = backend.initialize(timing, {
        .mode = mode,
        .bit_order = order,
        .frame_width = mcuhw::driver::spi_frame_width::bits8,
    }).has_value();
    if (!result.initialized) return false;

    constexpr std::uint16_t dummy = 0xA5U;
    const std::array segments{
        mcuhw::driver::spi_segment{.transmit = pattern,
                                   .receive = result.received},
        mcuhw::driver::spi_segment{.receive = result.dummy_received,
                                   .dummy = dummy},
    };
    auto started = backend.transaction(segments);
    if (!started) {
        static_cast<void>(backend.shutdown());
        return false;
    }
    auto transfer = std::move(*started);
    const auto deadline = mcuhw::execution::deadline{
        static_cast<std::uint32_t>(
            mcuhw::board_test::execution_hooks::now() + 480'000U)};
    if (!transfer.wait_until(deadline)) {
        static_cast<void>(transfer.take());
        static_cast<void>(backend.shutdown());
        return false;
    }
    const auto complete = transfer.take();
    result.completed = complete.has_value();
    result.frames = complete ? complete->frames : 0U;
    result.matched = result.received == pattern
        && result.dummy_received[0] == dummy
        && result.dummy_received[1] == dummy;
    const bool shutdown = backend.shutdown().has_value();
    return result.completed && result.frames == 8U && result.matched && shutdown;
}

TEST(uno_spi, spi1_d11_d12_segmented_loopback_modes_rates_and_idle_completion,
     "REQ-0028", "REQ-0036") {
    ASSERT_TRUE(exercise(*timing_1mhz, mcuhw::driver::spi_mode::mode0,
                         mcuhw::driver::spi_bit_order::most_significant_first,
                         mode0_observation));
    ASSERT_TRUE(exercise(*timing_6mhz, mcuhw::driver::spi_mode::mode3,
                         mcuhw::driver::spi_bit_order::least_significant_first,
                         mode3_observation));
}

constexpr char metadata[] =
    "MCUHW_META board=Arduino_UNO_R4_Minima suite=spi "
    "firmware=pln-0009 fixture=D11_SPI1_MOSI_to_D12_SPI1_MISO "
    "cs=D10 clock=D13 modes=0,3 rates_hz=1000000,6000000 "
    "completion=SPI1_SPII toolchain=arm-none-eabi-gcc-15.2.1 "
    "iclk_hz=48000000 pclka_hz=48000000 interface=J-Link_SWD_RTT\n";
constexpr mcuhw::board_test::evidence_metadata evidence{
    .suite = "spi",
    .firmware = "pln-0009",
    .fixture = "D11 SPI1 MOSI jumpered to D12 SPI1 MISO; observe D10 CS and D13 SCK",
    .coverage_build_id = 0x4d485701U,
};

template<typename Writer>
void write_observation(Writer& out, const observation& value,
                       const mcuhw::driver::spi_timing& timing) noexcept {
    out("target_hz", timing.target_hz);
    out("achieved_hz", timing.achieved_hz);
    out("initialized", value.initialized);
    out("completed_from_idle", value.completed);
    out("frames", value.frames);
    out("loopback_matched", value.matched);
}
} // namespace

int main() {
    mcuhw::board_test::start(metadata);
    configure_interrupts();
    mcuhw::board_test::finish(evidence, [&](auto& observations) {
        observations("controller", "SPI1 interrupt-driven master");
        observations("completion_interrupt", "SPI1 SPII after IDLNF=0");
        observations.object("mode0_msb", [&](auto& out) {
            write_observation(out, mode0_observation, *timing_1mhz);
        });
        observations.object("mode3_lsb", [&](auto& out) {
            write_observation(out, mode3_observation, *timing_6mhz);
        });
    });
}

extern "C" void IRQ0_Handler() noexcept {
    static_cast<void>(backend.handle_receive_interrupt());
    mcuhw::board_test::clear_icu_line(0U);
}
extern "C" void IRQ1_Handler() noexcept {
    static_cast<void>(backend.handle_transmit_empty_interrupt());
    mcuhw::board_test::clear_icu_line(1U);
}
extern "C" void IRQ2_Handler() noexcept {
    static_cast<void>(backend.handle_idle_interrupt());
    mcuhw::board_test::clear_icu_line(2U);
}
extern "C" void IRQ3_Handler() noexcept {
    static_cast<void>(backend.handle_error_interrupt());
    mcuhw::board_test::clear_icu_line(3U);
}
