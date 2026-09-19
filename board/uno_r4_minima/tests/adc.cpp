#include <array>
#include <cstdint>
#include <utility>

#include <mcutest/mcutest.hpp>

#include <mcuhw/backend/ra4m1/adc.hpp>
#include <mcuhw/board/arduino_uno_r4_minima.hpp>

#include "support.hpp"

namespace {
namespace board = mcuhw::board::arduino_uno_r4_minima;
using binding = mcuhw::backend::ra4m1::adc14_binding<
    mcuhw::backend::ra4m1::adc_input<0, board::a1_an000>,
    mcuhw::backend::ra4m1::adc_input<1, board::a2_an001>>;
using backend_type = mcuhw::backend::ra4m1::adc14_backend<
    binding, mcuhw::core::direct_mmio, mcuhw::board_test::execution_policy>;

mcuhw::board_test::execution_policy execution;
backend_type backend{mcuhw::board_test::mmio, execution};
constexpr std::array channels{
    mcuhw::driver::adc_channel{1U}, mcuhw::driver::adc_channel{0U}};

struct observation {
    bool initialized{};
    bool completed{};
    bool thresholds_passed{};
    std::array<std::uint16_t, 2> raw{};
};
observation result14{};
observation result12{};

void configure_interrupt() noexcept {
    *reinterpret_cast<volatile std::uint32_t*>(0xE000'E180U) = 1U;
    *reinterpret_cast<volatile std::uint32_t*>(0x4000'6300U)
        = mcuhw::mcu::ra4m1::event_number(
            mcuhw::mcu::ra4m1::event::adc_scan_end);
    *reinterpret_cast<volatile std::uint8_t*>(0xE000'E400U) = 0x80U;
    *reinterpret_cast<volatile std::uint32_t*>(0xE000'E280U) = 1U;
    *reinterpret_cast<volatile std::uint32_t*>(0xE000'E100U) = 1U;
    __asm__ volatile("dsb sy\n\tisb sy\n\tcpsie i" ::: "memory");
}

[[nodiscard]] bool exercise(mcuhw::driver::adc_resolution resolution,
                            observation& result) noexcept {
    result.initialized = backend.initialize({
        .resolution = resolution,
        .sample_states = 64U,
    }).has_value();
    if (!result.initialized) return false;
    auto started = backend.convert(channels, result.raw);
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
    result.completed = transfer.take().has_value();
    const auto full_scale = resolution == mcuhw::driver::adc_resolution::bits14
        ? 16'383U : 4'095U;
    const auto rail_margin = static_cast<std::uint16_t>(full_scale / 20U);
    // Results preserve requested [AN001, AN000] order: A2 is 3.3 V, A1 is GND.
    result.thresholds_passed = result.raw[0] >= full_scale - rail_margin
        && result.raw[1] <= rail_margin;
    const bool shutdown = backend.shutdown().has_value();
    return result.completed && result.thresholds_passed && shutdown;
}

TEST(uno_adc, adc14_a2_high_a1_low_raw_order_at_14_and_12_bits,
     "REQ-0029", "REQ-0036") {
    ASSERT_TRUE(exercise(mcuhw::driver::adc_resolution::bits14, result14));
    ASSERT_TRUE(exercise(mcuhw::driver::adc_resolution::bits12, result12));
}

constexpr char metadata[] =
    "MCUHW_META board=Arduino_UNO_R4_Minima suite=adc "
    "firmware=pln-0009 fixture=A1_AN000_to_GND_A2_AN001_to_3V3 "
    "resolutions=14,12 sample_states=64 reference=VREFH0 format=right_aligned "
    "toolchain=arm-none-eabi-gcc-15.2.1 iclk_hz=48000000 "
    "pclkb_hz=24000000 pclkc_hz=24000000 interface=J-Link_SWD_RTT\n";
constexpr mcuhw::board_test::evidence_metadata evidence{
    .suite = "adc",
    .firmware = "pln-0009",
    .fixture = "A1/AN000 tied to GND and A2/AN001 tied to 3.3 V",
    .coverage_build_id = 0x4d485701U,
};

template<typename Writer>
void write_observation(Writer& out, const observation& result,
                       std::uint32_t bits) noexcept {
    out("resolution_bits", bits);
    out("initialized", result.initialized);
    out("completed", result.completed);
    out("a2_an001_raw", result.raw[0]);
    out("a1_an000_raw", result.raw[1]);
    out("rail_thresholds_passed", result.thresholds_passed);
}
} // namespace

int main() {
    mcuhw::board_test::start(metadata);
    configure_interrupt();
    mcuhw::board_test::finish(evidence, [&](auto& observations) {
        observations("converter", "ADC140 software-triggered finite scan");
        observations("requested_order", "AN001,AN000");
        observations("reference", "VREFH0/AVCC0 nominal 3.3 V");
        observations.object("bits14", [&](auto& out) {
            write_observation(out, result14, 14U);
        });
        observations.object("bits12", [&](auto& out) {
            write_observation(out, result12, 12U);
        });
    });
}

extern "C" void IRQ0_Handler() noexcept {
    static_cast<void>(backend.handle_scan_end_interrupt());
    mcuhw::board_test::clear_icu_line(0U);
}
