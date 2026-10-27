// SPDX-License-Identifier: Apache-2.0 OR MIT

// @req-file REQ-0036 REQ-0037 REQ-0041

#include <array>
#include <cstddef>
#include <cstdint>
#include <utility>

#include <mcutest/mcutest.hpp>

#include <mcubsp/backend/ra4m1/adc.hpp>
#include <mcubsp/backend/ra4m1/dmac.hpp>
#include <mcubsp/backend/ra4m1/gpt.hpp>
#include <mcubsp/backend/ra4m1/iic.hpp>
#include <mcubsp/backend/ra4m1/sci_uart.hpp>
#include <mcubsp/backend/ra4m1/spi.hpp>
#include <mcubsp/board/arduino/uno_r4_minima/bindings.hpp>
#include <mcubsp/driver/adc.hpp>
#include <mcubsp/driver/gpio.hpp>
#include <mcubsp/driver/i2c.hpp>
#include <mcubsp/driver/pwm.hpp>
#include <mcubsp/driver/spi.hpp>
#include <mcubsp/driver/uart.hpp>

#include "support.hpp"

extern "C" std::byte __data_load;
extern "C" std::byte __data_start;
extern "C" std::byte __data_end;
extern "C" std::byte __image_ram_end;
extern "C" std::byte __stack_limit;
extern "C" std::byte __stack;

namespace {

using board = mcubsp::board::arduino::uno_r4_minima_bindings;
using execution_policy = mcubsp::board_test::execution_policy;
using mmio_policy = mcubsp::core::direct_mmio;

using pwm_binding = mcubsp::backend::ra4m1::gpt_binding<
    1, mcubsp::mcu::ra4m1::gpt_output::b, board::d3>;
using pwm_backend = mcubsp::backend::ra4m1::gpt_pwm_backend<
    pwm_binding, mmio_policy, execution_policy>;
using uart_binding = mcubsp::backend::ra4m1::sci_uart_binding<
    2, board::d0_rx, board::d1_tx>;
using uart_backend = mcubsp::backend::ra4m1::sci_uart_backend<
    uart_binding, mmio_policy, execution_policy>;
using i2c_binding = mcubsp::backend::ra4m1::iic_binding<
    1, board::a5_scl, board::a4_sda>;
using i2c_backend = mcubsp::backend::ra4m1::iic_controller_backend<
    i2c_binding, mmio_policy, execution_policy>;
using spi_binding = mcubsp::backend::ra4m1::spi_binding<
    1, board::d12_miso, board::d11_mosi, board::d13_sck, board::d10_ssl>;
using spi_backend = mcubsp::backend::ra4m1::spi_master_backend<
    spi_binding, mmio_policy, execution_policy>;
using adc_binding = mcubsp::backend::ra4m1::adc14_binding<
    mcubsp::backend::ra4m1::adc_input<9, board::a0_an009>>;
using adc_backend = mcubsp::backend::ra4m1::adc14_backend<
    adc_binding, mmio_policy, execution_policy>;
using dmac_backend = mcubsp::backend::ra4m1::dmac_channel_backend<
    0, mmio_policy, execution_policy>;

mcubsp::driver::gpio_pin led{
    mcubsp::board_test::gpio, board::led_builtin_d13};
bool led_initialized{};

struct latency_observation {
    std::uint32_t iterations{};
    std::uint32_t empty_loop_cycles{};
    std::uint32_t direct_gpio_pair_cycles{};
    std::uint32_t driver_gpio_pair_cycles{};
    std::uint32_t critical_pair_cycles{};
    std::uint32_t direct_gpio_cycles_per_write{};
    std::uint32_t driver_gpio_cycles_per_write{};
    std::uint32_t critical_cycles_per_pair{};
    bool driver_operations_succeeded{};
};

latency_observation latency{};
struct isr_observation {
    std::uint32_t handler_cycles{};
    bool completed{};
    bool matched{};
    bool shutdown{};
};

execution_policy execution;
dmac_backend dmac{mcubsp::board_test::mmio, execution};
alignas(4) std::array<std::uint32_t, 16> dma_source{
    0x0123'4567U, 0x89AB'CDEFU, 0x1357'9BDFU, 0x2468'ACE0U,
    0xFFFF'0000U, 0x0000'FFFFU, 0xA5A5'5A5AU, 0x5A5A'A5A5U,
    0xDEAD'BEEFU, 0xC001'D00DU, 0x1020'3040U, 0x5060'7080U,
    0x0F1E'2D3CU, 0x4B5A'6978U, 0x8877'6655U, 0x4433'2211U,
};
alignas(4) std::array<std::uint32_t, dma_source.size()> dma_destination{};
isr_observation dmac_isr{};
constexpr std::uint32_t benchmark_iterations = 256U;
constexpr std::uint32_t stack_pattern = 0xA5A5'A5A5U;
std::uint32_t* watermark_begin{};
std::uint32_t* watermark_end{};

[[nodiscard]] std::uintptr_t address_of(const std::byte& symbol) noexcept {
    return reinterpret_cast<std::uintptr_t>(&symbol);
}

[[nodiscard]] std::uintptr_t current_stack_pointer() noexcept {
    std::uintptr_t value;
    __asm__ volatile("mrs %0, msp" : "=r"(value));
    return value;
}

void prepare_stack_watermark() noexcept {
    constexpr std::uintptr_t active_stack_guard = 256U;
    const auto first = (address_of(__image_ram_end) + 3U) & ~std::uintptr_t{3U};
    const auto current = current_stack_pointer();
    if (current <= first + active_stack_guard) {
        return;
    }
    const auto last = (current - active_stack_guard) & ~std::uintptr_t{3U};
    watermark_begin = reinterpret_cast<std::uint32_t*>(first);
    watermark_end = reinterpret_cast<std::uint32_t*>(last);
    for (auto* cursor = watermark_begin; cursor < watermark_end; ++cursor) {
        *cursor = stack_pattern;
    }
}

[[nodiscard]] std::uint32_t stack_peak_upper_bound() noexcept {
    if (watermark_begin == nullptr || watermark_end == nullptr) {
        return static_cast<std::uint32_t>(
            address_of(__stack) - current_stack_pointer());
    }
    auto* first_changed = watermark_begin;
    while (first_changed < watermark_end && *first_changed == stack_pattern) {
        ++first_changed;
    }
    const auto lowest_observed = first_changed == watermark_end
        ? reinterpret_cast<std::uintptr_t>(watermark_end)
        : reinterpret_cast<std::uintptr_t>(first_changed);
    return static_cast<std::uint32_t>(address_of(__stack) - lowest_observed);
}

[[nodiscard]] std::uint32_t net_per_unit(
    std::uint32_t measured, std::uint32_t overhead,
    std::uint32_t units) noexcept {
    return measured > overhead ? (measured - overhead + units - 1U) / units
                               : 0U;
}

void run_latency_benchmarks() noexcept {
    latency.iterations = benchmark_iterations;
    const auto empty_start = mcubsp::board_test::execution_hooks::now();
    for (std::uint32_t index = 0; index < benchmark_iterations; ++index) {
        __asm__ volatile("" : : "r"(index) : "memory");
    }
    latency.empty_loop_cycles =
        mcubsp::board_test::execution_hooks::now() - empty_start;

    auto& port_command = *reinterpret_cast<volatile std::uint32_t*>(
        0x4004'0028U);
    constexpr std::uint32_t set_mask = 1U << 11U;
    constexpr std::uint32_t clear_mask = 1U << (16U + 11U);
    const auto direct_start = mcubsp::board_test::execution_hooks::now();
    for (std::uint32_t index = 0; index < benchmark_iterations; ++index) {
        port_command = set_mask;
        __asm__ volatile("dmb sy" ::: "memory");
        port_command = clear_mask;
        __asm__ volatile("dmb sy" ::: "memory");
    }
    latency.direct_gpio_pair_cycles =
        mcubsp::board_test::execution_hooks::now() - direct_start;

    bool succeeded = true;
    const auto driver_start = mcubsp::board_test::execution_hooks::now();
    for (std::uint32_t index = 0; index < benchmark_iterations; ++index) {
        succeeded = led.set().has_value() && succeeded;
        succeeded = led.clear().has_value() && succeeded;
    }
    latency.driver_gpio_pair_cycles =
        mcubsp::board_test::execution_hooks::now() - driver_start;
    latency.driver_operations_succeeded = succeeded;

    const auto critical_start = mcubsp::board_test::execution_hooks::now();
    for (std::uint32_t index = 0; index < benchmark_iterations; ++index) {
        const auto token =
            mcubsp::board_test::execution_hooks::enter_critical();
        mcubsp::board_test::execution_hooks::leave_critical(token);
    }
    latency.critical_pair_cycles =
        mcubsp::board_test::execution_hooks::now() - critical_start;

    latency.direct_gpio_cycles_per_write = net_per_unit(
        latency.direct_gpio_pair_cycles, latency.empty_loop_cycles,
        benchmark_iterations * 2U);
    latency.driver_gpio_cycles_per_write = net_per_unit(
        latency.driver_gpio_pair_cycles, latency.empty_loop_cycles,
        benchmark_iterations * 2U);
    latency.critical_cycles_per_pair = net_per_unit(
        latency.critical_pair_cycles, latency.empty_loop_cycles,
        benchmark_iterations);
}

void configure_dmac_interrupt() noexcept {
    *reinterpret_cast<volatile std::uint32_t*>(0xE000'E180U) = 1U;
    *reinterpret_cast<volatile std::uint32_t*>(0x4000'6300U) =
        mcubsp::mcu::ra4m1::event_number(
            mcubsp::mcu::ra4m1::event::dmac0_int);
    *reinterpret_cast<volatile std::uint8_t*>(0xE000'E400U) = 0x80U;
    *reinterpret_cast<volatile std::uint32_t*>(0xE000'E280U) = 1U;
    *reinterpret_cast<volatile std::uint32_t*>(0xE000'E100U) = 1U;
    __asm__ volatile("dsb sy\n\tisb sy\n\tcpsie i" ::: "memory");
}

TEST(uno_release_metrics, records_direct_and_driver_cycle_costs,
     "REQ-0036", "REQ-0037", "REQ-0041") {
    ASSERT_TRUE(led_initialized);
    run_latency_benchmarks();
    EXPECT_TRUE(latency.driver_operations_succeeded);
    EXPECT_TRUE(latency.direct_gpio_cycles_per_write > 0U);
    EXPECT_TRUE(latency.driver_gpio_cycles_per_write > 0U);
    EXPECT_TRUE(latency.critical_cycles_per_pair > 0U);
}

TEST(uno_release_metrics, records_representative_dmac_isr_body_cost,
     "REQ-0030", "REQ-0036", "REQ-0037", "REQ-0041") {
    configure_dmac_interrupt();
    ASSERT_TRUE(dmac.initialize().has_value());
    auto started = dmac.copy({
        .source = reinterpret_cast<std::uintptr_t>(dma_source.data()),
        .destination = reinterpret_cast<std::uintptr_t>(
            dma_destination.data()),
        .count = static_cast<std::uint32_t>(dma_source.size()),
        .width = mcubsp::driver::dma_width::bits32,
    });
    ASSERT_TRUE(started.has_value());
    auto transfer = std::move(*started);
    const auto completed = transfer.wait_until({
        static_cast<std::uint32_t>(
            mcubsp::board_test::execution_hooks::now() + 480'000U)});
    ASSERT_TRUE(completed.has_value());
    const auto result = transfer.take();
    dmac_isr.completed = result.has_value()
        && result->units == dma_source.size();
    dmac_isr.matched = dma_source == dma_destination;
    dmac_isr.shutdown = dmac.shutdown().has_value();
    EXPECT_TRUE(dmac_isr.completed);
    EXPECT_TRUE(dmac_isr.matched);
    EXPECT_TRUE(dmac_isr.shutdown);
    EXPECT_TRUE(dmac_isr.handler_cycles > 0U);
}

TEST(uno_release_metrics, records_fixed_storage_and_memory_bounds,
     "REQ-0037", "REQ-0041") {
    EXPECT_TRUE(address_of(__image_ram_end) <= address_of(__stack_limit));
    EXPECT_EQ(address_of(__stack) - address_of(__stack_limit), 4096U);
    EXPECT_TRUE(sizeof(uart_backend) > sizeof(mcubsp::driver::uart<uart_backend>));
    EXPECT_TRUE(sizeof(spi_backend) > sizeof(mcubsp::driver::spi_controller<spi_backend>));
}

constexpr char metadata[] =
    "MCUBSP_META board=Arduino_UNO_R4_Minima suite=release_metrics "
    "firmware=pln-0011 observation=internal_DWT_and_linker_symbols "
    "coverage=disabled optimization=Os toolchain=arm-none-eabi-gcc-15.2.1 "
    "iclk_hz=48000000 interface=J-Link_SWD_RTT\n";
constexpr mcubsp::board_test::evidence_metadata evidence{
    .suite = "release_metrics",
    .firmware = "pln-0011",
    .fixture = "built-in D13 plus internal DWT and linker symbols",
    .coverage_build_id = 0U,
    .coverage_enabled = false,
};

} // namespace

int main() {
    mcubsp::board_test::start(metadata);
    const mcubsp::driver::gpio_configuration configuration{
        .direction = mcubsp::driver::gpio_direction::output,
        .initial_level = mcubsp::driver::gpio_level::low,
        .output_type = mcubsp::driver::gpio_output_type::push_pull,
        .pull = mcubsp::driver::gpio_pull::none,
        .drive = mcubsp::driver::gpio_drive::middle,
    };
    led_initialized = led.initialize(configuration).has_value();
    prepare_stack_watermark();
    mcubsp::board_test::finish(evidence, [&](auto& observations) {
        const auto data_bytes =
            address_of(__data_end) - address_of(__data_start);
        observations.object("latency_cycles", [&](auto& cycles) {
            cycles("iterations", latency.iterations);
            cycles("empty_loop_total", latency.empty_loop_cycles);
            cycles("direct_gpio_pair_total", latency.direct_gpio_pair_cycles);
            cycles("driver_gpio_pair_total", latency.driver_gpio_pair_cycles);
            cycles("critical_pair_total", latency.critical_pair_cycles);
            cycles("direct_gpio_per_write",
                   latency.direct_gpio_cycles_per_write);
            cycles("driver_gpio_per_write",
                   latency.driver_gpio_cycles_per_write);
            cycles("critical_section_pair",
                   latency.critical_cycles_per_pair);
            cycles("dmac_completion_isr_body", dmac_isr.handler_cycles);
        });
        observations.object("storage_bytes", [&](auto& storage) {
            storage("execution_policy", sizeof(execution_policy));
            storage("gpio_pin", sizeof(decltype(led)));
            storage("pwm_backend", sizeof(pwm_backend));
            storage("pwm_channel",
                    sizeof(mcubsp::driver::pwm_channel<pwm_backend>));
            storage("uart_backend", sizeof(uart_backend));
            storage("uart", sizeof(mcubsp::driver::uart<uart_backend>));
            storage("i2c_backend", sizeof(i2c_backend));
            storage("i2c_controller",
                    sizeof(mcubsp::driver::i2c_controller<i2c_backend>));
            storage("spi_backend", sizeof(spi_backend));
            storage("spi_controller",
                    sizeof(mcubsp::driver::spi_controller<spi_backend>));
            storage("adc_backend", sizeof(adc_backend));
            storage("adc_controller",
                    sizeof(mcubsp::driver::adc_controller<adc_backend>));
            storage("dmac_channel_backend", sizeof(dmac_backend));
        });
        observations.object("linked_image_bytes", [&](auto& image) {
            image("flash", static_cast<std::uint32_t>(
                address_of(__data_load) + data_bytes
                - board::application_flash_origin));
            image("static_ram", static_cast<std::uint32_t>(
                address_of(__image_ram_end) - board::sram_origin));
            image("ram_to_stack_reserve", static_cast<std::uint32_t>(
                address_of(__stack_limit) - address_of(__image_ram_end)));
            image("stack_reserve", static_cast<std::uint32_t>(
                address_of(__stack) - address_of(__stack_limit)));
            image("stack_peak_upper_bound", stack_peak_upper_bound());
        });
        observations("dynamic_allocation", "unsupported by restricted runtime");
        observations("measurement_scope",
                     "non-instrumented -Os release-metrics image; DWT CYCCNT at 48 MHz");
    });
}

extern "C" void IRQ0_Handler() {
    const auto start = mcubsp::board_test::execution_hooks::now();
    static_cast<void>(dmac.handle_transfer_end_interrupt());
    mcubsp::board_test::clear_icu_line(0U);
    dmac_isr.handler_cycles =
        mcubsp::board_test::execution_hooks::now() - start;
}
