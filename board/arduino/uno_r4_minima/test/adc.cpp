#include <array>
#include <cstdint>
#include <utility>

#include <mcutest/mcutest.hpp>

#include <mcubsp/backend/ra4m1/adc.hpp>
#include <mcubsp/backend/ra4m1/dmac.hpp>
#include <mcubsp/board/arduino_uno_r4_minima.hpp>

#include "support.hpp"

namespace {
namespace board = mcubsp::board::arduino_uno_r4_minima;
using binding = mcubsp::backend::ra4m1::adc14_binding<
    mcubsp::backend::ra4m1::adc_input<9, board::a0_an009>>;
using backend_type = mcubsp::backend::ra4m1::adc14_backend<
    binding, mcubsp::core::direct_mmio, mcubsp::board_test::execution_policy>;
using dmac_type = mcubsp::backend::ra4m1::dmac_channel_backend<
    0, mcubsp::core::direct_mmio, mcubsp::board_test::execution_policy>;

mcubsp::board_test::execution_policy execution;
backend_type backend{mcubsp::board_test::mmio, execution};
dmac_type dmac{mcubsp::board_test::mmio, execution};
constexpr std::array channels{mcubsp::driver::adc_channel{9U}};

struct observation {
    bool initialized{};
    bool completed{};
    std::array<std::uint16_t, 1> raw{};
};
observation result14{};
observation result12{};
bool resolution_scaling_passed{};
std::array<std::uint16_t, 32> dma_samples{};
bool dma_initialized{};
bool dma_completed{};
bool dma_stable{};
bool dma_shutdown{};
std::uint16_t dma_min{};
std::uint16_t dma_max{};
std::uint32_t dma_average{};
volatile std::uint32_t dma_interrupts{};
bool dma_interrupt_mode{};

void configure_interrupt(mcubsp::mcu::ra4m1::event event) noexcept {
    *reinterpret_cast<volatile std::uint32_t*>(0xE000'E180U) = 1U;
    *reinterpret_cast<volatile std::uint32_t*>(0x4000'6300U)
        = mcubsp::mcu::ra4m1::event_number(event);
    *reinterpret_cast<volatile std::uint8_t*>(0xE000'E400U) = 0x80U;
    *reinterpret_cast<volatile std::uint32_t*>(0xE000'E280U) = 1U;
    *reinterpret_cast<volatile std::uint32_t*>(0xE000'E100U) = 1U;
    __asm__ volatile("dsb sy\n\tisb sy\n\tcpsie i" ::: "memory");
}

TEST(uno_adc, adc14_a0_continuous_scan_fills_buffer_via_dmac0,
     "REQ-0018", "REQ-0019", "REQ-0029", "REQ-0030", "REQ-0036") {
    configure_interrupt(mcubsp::mcu::ra4m1::event::dmac0_int);
    dma_interrupt_mode = true;
    dma_initialized = dmac.initialize().has_value()
        && backend.initialize({
            .resolution = mcubsp::driver::adc_resolution::bits14,
            .sample_states = 64U,
        }).has_value();
    ASSERT_TRUE(dma_initialized);
    auto started = backend.sample_dma(
        mcubsp::driver::adc_channel{9U}, dma_samples, dmac);
    ASSERT_TRUE(started.has_value());
    auto transfer = std::move(*started);
    const auto deadline = mcubsp::execution::deadline{
        static_cast<std::uint32_t>(
            mcubsp::board_test::execution_hooks::now() + 480'000U)};
    ASSERT_TRUE(transfer.wait_until(deadline).has_value());
    const auto result = transfer.take();
    dma_completed = result.has_value()
        && result->samples == dma_samples.size();
    ASSERT_TRUE(dma_completed);
    dma_min = dma_samples[0];
    dma_max = dma_samples[0];
    std::uint32_t sum{};
    for (const auto sample : dma_samples) {
        if (sample < dma_min) dma_min = sample;
        if (sample > dma_max) dma_max = sample;
        sum += sample;
    }
    dma_average = sum / dma_samples.size();
    dma_stable = static_cast<std::uint32_t>(dma_max - dma_min)
        <= 16'383U / 20U;
    ASSERT_TRUE(dma_stable);
    ASSERT_EQ(dma_interrupts, 1U);
    dma_shutdown = backend.shutdown().has_value()
        && dmac.shutdown().has_value();
    ASSERT_TRUE(dma_shutdown);
}

[[nodiscard]] bool exercise(mcubsp::driver::adc_resolution resolution,
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
    const auto deadline = mcubsp::execution::deadline{
        static_cast<std::uint32_t>(
            mcubsp::board_test::execution_hooks::now() + 480'000U)};
    if (!transfer.wait_until(deadline)) {
        static_cast<void>(transfer.take());
        static_cast<void>(backend.shutdown());
        return false;
    }
    result.completed = transfer.take().has_value();
    const bool shutdown = backend.shutdown().has_value();
    return result.completed && shutdown;
}

TEST(uno_adc, adc14_a0_potentiometer_scales_at_14_and_12_bits,
     "REQ-0029", "REQ-0036") {
    dma_interrupt_mode = false;
    configure_interrupt(mcubsp::mcu::ra4m1::event::adc_scan_end);
    ASSERT_TRUE(exercise(mcubsp::driver::adc_resolution::bits14, result14));
    ASSERT_TRUE(exercise(mcubsp::driver::adc_resolution::bits12, result12));
    const auto raw14 = static_cast<std::uint32_t>(result14.raw[0]);
    const auto raw12_scaled = static_cast<std::uint32_t>(result12.raw[0]) * 4U;
    const auto difference = raw14 > raw12_scaled ? raw14 - raw12_scaled
                                                  : raw12_scaled - raw14;
    // Both conversions sample the same stationary wiper. Allow 5% of the
    // 14-bit range for pot movement, supply noise, and conversion quantization.
    resolution_scaling_passed = difference <= (16'383U / 20U);
    ASSERT_TRUE(resolution_scaling_passed);
}

constexpr char metadata[] =
    "MCUBSP_META board=Arduino_UNO_R4_Minima suite=adc "
    "firmware=pln-0010 fixture=A0_AN009_potentiometer_GND_to_3V3 "
    "resolutions=14,12 sample_states=64 reference=VREFH0 format=right_aligned "
    "dma=DMAC0 activation=ADC140_ADI mode=continuous_single_channel "
    "toolchain=arm-none-eabi-gcc-15.2.1 iclk_hz=48000000 "
    "pclkb_hz=24000000 pclkc_hz=24000000 interface=J-Link_SWD_RTT\n";
constexpr mcubsp::board_test::evidence_metadata evidence{
    .suite = "adc",
    .firmware = "pln-0010",
    .fixture = "potentiometer wiper on A0/AN009, ends on GND and 3.3 V",
    .coverage_build_id = 0x4d485701U,
};

template<typename Writer>
void write_observation(Writer& out, const observation& result,
                       std::uint32_t bits) noexcept {
    out("resolution_bits", bits);
    out("initialized", result.initialized);
    out("completed", result.completed);
    out("a0_an009_raw", result.raw[0]);
    const auto full_scale = bits == 14U ? 16'383U : 4'095U;
    out("estimated_millivolts",
        static_cast<std::uint32_t>(result.raw[0]) * 3'300U / full_scale);
}
} // namespace

int main() {
    mcubsp::board_test::start(metadata);
    configure_interrupt(mcubsp::mcu::ra4m1::event::adc_scan_end);
    mcubsp::board_test::finish(evidence, [&](auto& observations) {
        observations("converter", "ADC140 software-triggered finite scan");
        observations("requested_order", "AN009");
        observations("reference", "VREFH0/AVCC0 nominal 3.3 V");
        observations("resolution_scaling_passed", resolution_scaling_passed);
        observations.object("bits14", [&](auto& out) {
            write_observation(out, result14, 14U);
        });
        observations.object("bits12", [&](auto& out) {
            write_observation(out, result12, 12U);
        });
        observations.object("dma_continuous_an009", [&](auto& out) {
            out("initialized", dma_initialized);
            out("completed", dma_completed);
            out("samples", static_cast<std::uint32_t>(dma_samples.size()));
            out("minimum", dma_min);
            out("maximum", dma_max);
            out("average", dma_average);
            out("stable_within_5_percent_full_scale", dma_stable);
            out("dmac0_interrupts", dma_interrupts);
            out("shutdown", dma_shutdown);
        });
    });
}

extern "C" void IRQ0_Handler() noexcept {
    if (dma_interrupt_mode) {
        dma_interrupts = dma_interrupts + 1U;
        static_cast<void>(dmac.handle_transfer_end_interrupt());
        static_cast<void>(backend.handle_dma_completion());
    } else {
        static_cast<void>(backend.handle_scan_end_interrupt());
    }
    mcubsp::board_test::clear_icu_line(0U);
}
