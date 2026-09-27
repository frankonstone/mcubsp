// SPDX-License-Identifier: Apache-2.0 OR MIT

// @req-file REQ-0017 REQ-0019 REQ-0028 REQ-0036

#include <array>
#include <cstddef>
#include <cstdint>
#include <span>
#include <utility>

#include <mcutest/mcutest.hpp>

#include <mcuhw/backend/ra4m1/spi.hpp>
#include <mcuhw/board/arduino_uno_r4_minima.hpp>
#include <mcuhw/driver/gpio.hpp>

#include "support.hpp"

namespace {
namespace board = mcuhw::board::arduino_uno_r4_minima;
using binding = mcuhw::backend::ra4m1::spi_binding<
    1, board::d12_miso, board::d11_mosi, board::d13_sck, board::d10_ssl>;
using backend_type = mcuhw::backend::ra4m1::spi_master_backend<
    binding, mcuhw::core::direct_mmio, mcuhw::board_test::execution_policy>;

mcuhw::board_test::execution_policy execution;
backend_type backend{mcuhw::board_test::mmio, execution};

// The Ethernet Shield R3 shares SPI with its microSD socket. D4/P103 is the
// SD chip select and must stay high while the W5100 on D10 is selected.
constexpr mcuhw::core::pin_id sd_chip_select_pin{1U, 3U};
mcuhw::driver::gpio_pin sd_chip_select{
    mcuhw::board_test::gpio, sd_chip_select_pin};
bool sd_deselected{};

constexpr auto timing_1mhz = mcuhw::backend::ra4m1::solve_spi_timing(
    {48'000'000U, 1'000'000U, 0U});
constexpr auto timing_6mhz = mcuhw::backend::ra4m1::solve_spi_timing(
    {48'000'000U, 6'000'000U, 0U});
static_assert(timing_1mhz.has_value() && timing_6mhz.has_value());

constexpr std::uint8_t w5100_read = 0x0FU;
constexpr std::uint8_t w5100_write = 0xF0U;
constexpr std::uint16_t w5100_retry_time = 0x0017U;
constexpr std::uint16_t probe_retry_time = 0xA55AU;

volatile std::uint32_t receive_interrupts{};
volatile std::uint32_t transmit_interrupts{};
volatile std::uint32_t idle_interrupts{};
volatile std::uint32_t error_interrupts{};

struct rate_observation {
    bool initialized{};
    bool original_read{};
    bool probe_written{};
    bool probe_read{};
    bool probe_matched{};
    bool restored{};
    bool restore_read{};
    bool restore_matched{};
    bool shutdown{};
    std::uint16_t original_retry_time{};
    std::uint16_t probe_retry_time_read{};
    std::uint16_t restored_retry_time{};
    std::size_t transactions{};
    std::size_t frames{};
    std::uint32_t receive_interrupt_count{};
    std::uint32_t transmit_interrupt_count{};
    std::uint32_t idle_interrupt_count{};
    std::uint32_t error_interrupt_count{};
};

rate_observation slow_observation{};
rate_observation fast_observation{};

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

[[nodiscard]] mcuhw::execution::deadline deadline_after(
    std::uint32_t cycles) noexcept {
    return {static_cast<std::uint32_t>(
        mcuhw::board_test::execution_hooks::now() + cycles)};
}

[[nodiscard]] bool transact(
    std::span<const mcuhw::driver::spi_segment> segments,
    std::size_t expected_frames, rate_observation& observation) noexcept {
    auto started = backend.transaction(segments);
    if (!started) {
        return false;
    }
    auto transfer = std::move(*started);
    if (!transfer.wait_until(deadline_after(480'000U))) {
        static_cast<void>(transfer.take());
        return false;
    }
    const auto result = transfer.take();
    if (!result || result->frames != expected_frames) {
        return false;
    }
    ++observation.transactions;
    observation.frames += result->frames;
    return true;
}

[[nodiscard]] bool read_register(std::uint16_t address, std::uint8_t& value,
                                 rate_observation& observation) noexcept {
    const std::array<std::uint16_t, 3> header{
        w5100_read,
        static_cast<std::uint8_t>(address >> 8U),
        static_cast<std::uint8_t>(address),
    };
    std::array<std::uint16_t, 1> response{};
    const std::array segments{
        mcuhw::driver::spi_segment{.transmit = header},
        mcuhw::driver::spi_segment{.receive = response, .dummy = 0x00U},
    };
    if (!transact(segments, 4U, observation)) {
        return false;
    }
    value = static_cast<std::uint8_t>(response[0]);
    return true;
}

[[nodiscard]] bool write_register(std::uint16_t address, std::uint8_t value,
                                  rate_observation& observation) noexcept {
    const std::array<std::uint16_t, 4> command{
        w5100_write,
        static_cast<std::uint8_t>(address >> 8U),
        static_cast<std::uint8_t>(address),
        value,
    };
    const std::array segments{
        mcuhw::driver::spi_segment{.transmit = command},
    };
    return transact(segments, command.size(), observation);
}

[[nodiscard]] bool read_retry_time(std::uint16_t& value,
                                   rate_observation& observation) noexcept {
    std::uint8_t high{};
    std::uint8_t low{};
    const bool read = read_register(w5100_retry_time, high, observation)
        && read_register(w5100_retry_time + 1U, low, observation);
    value = static_cast<std::uint16_t>(
        (static_cast<std::uint16_t>(high) << 8U) | low);
    return read;
}

[[nodiscard]] bool write_retry_time(std::uint16_t value,
                                    rate_observation& observation) noexcept {
    return write_register(w5100_retry_time,
                          static_cast<std::uint8_t>(value >> 8U), observation)
        && write_register(w5100_retry_time + 1U,
                          static_cast<std::uint8_t>(value), observation);
}

[[nodiscard]] bool exercise(const mcuhw::backend::ra4m1::spi_timing& timing,
                            rate_observation& observation) noexcept {
    const auto receive_before = receive_interrupts;
    const auto transmit_before = transmit_interrupts;
    const auto idle_before = idle_interrupts;
    const auto error_before = error_interrupts;

    observation.initialized = backend.initialize(timing, {
        .mode = mcuhw::driver::spi_mode::mode0,
        .bit_order = mcuhw::driver::spi_bit_order::most_significant_first,
        .frame_width = mcuhw::driver::spi_frame_width::bits8,
    }).has_value();
    if (!observation.initialized) {
        return false;
    }

    observation.original_read = read_retry_time(
        observation.original_retry_time, observation);
    if (observation.original_read) {
        observation.probe_written = write_retry_time(
            probe_retry_time, observation);
        if (observation.probe_written) {
            observation.probe_read = read_retry_time(
                observation.probe_retry_time_read, observation);
            observation.probe_matched = observation.probe_read
                && observation.probe_retry_time_read == probe_retry_time;
        }

        // Always attempt to put the shield back exactly as it was, including
        // after a failed probe verification.
        observation.restored = write_retry_time(
            observation.original_retry_time, observation);
        if (observation.restored) {
            observation.restore_read = read_retry_time(
                observation.restored_retry_time, observation);
            observation.restore_matched = observation.restore_read
                && observation.restored_retry_time
                    == observation.original_retry_time;
        }
    }

    observation.shutdown = backend.shutdown().has_value();
    observation.receive_interrupt_count = receive_interrupts - receive_before;
    observation.transmit_interrupt_count = transmit_interrupts - transmit_before;
    observation.idle_interrupt_count = idle_interrupts - idle_before;
    observation.error_interrupt_count = error_interrupts - error_before;
    return observation.original_read && observation.probe_written
        && observation.probe_matched && observation.restored
        && observation.restore_matched && observation.shutdown
        && observation.error_interrupt_count == 0U;
}

TEST(uno_spi, spi1_w5100_segmented_read_write_restore_and_idle_completion,
     "REQ-0028", "REQ-0036") {
    ASSERT_TRUE(sd_deselected);
    ASSERT_TRUE(exercise(*timing_1mhz, slow_observation));
    ASSERT_TRUE(exercise(*timing_6mhz, fast_observation));
}

constexpr char metadata[] =
    "MCUHW_META board=Arduino_UNO_R4_Minima suite=spi "
    "firmware=pln-0009 fixture=Ethernet_Shield_R3_W5100 "
    "w5100_cs=D10 sd_cs=D4 clock=D13 mode=0 rates_hz=1000000,6000000 "
    "completion=SPI1_SPII toolchain=arm-none-eabi-gcc-15.2.1 "
    "iclk_hz=48000000 pclka_hz=48000000 interface=J-Link_SWD_RTT\n";
constexpr mcuhw::board_test::evidence_metadata evidence{
    .suite = "spi",
    .firmware = "pln-0009",
    .fixture = "Arduino Ethernet Shield R3 W5100 stacked; W5100 CS D10; SD CS D4 held high",
    .coverage_build_id = 0x4d485701U,
};

template<typename Writer>
void write_observation(Writer& out, const rate_observation& value,
                       const mcuhw::backend::ra4m1::spi_timing& timing) noexcept {
    out("target_hz", timing.target_hz);
    out("achieved_hz", timing.achieved_hz);
    out("initialized", value.initialized);
    out("original_read", value.original_read);
    out("original_retry_time", value.original_retry_time);
    out("probe_written", value.probe_written);
    out("probe_retry_time", probe_retry_time);
    out("probe_retry_time_read", value.probe_retry_time_read);
    out("probe_matched", value.probe_matched);
    out("restored", value.restored);
    out("restored_retry_time", value.restored_retry_time);
    out("restore_matched", value.restore_matched);
    out("shutdown", value.shutdown);
    out("transactions", value.transactions);
    out("frames", value.frames);
    out("receive_interrupts", value.receive_interrupt_count);
    out("transmit_interrupts", value.transmit_interrupt_count);
    out("idle_interrupts", value.idle_interrupt_count);
    out("error_interrupts", value.error_interrupt_count);
}
} // namespace

int main() {
    mcuhw::board_test::start(metadata);
    sd_deselected = sd_chip_select.initialize({
        .direction = mcuhw::driver::gpio_direction::output,
        .initial_level = mcuhw::driver::gpio_level::high,
        .output_type = mcuhw::driver::gpio_output_type::push_pull,
        .pull = mcuhw::driver::gpio_pull::none,
        .drive = mcuhw::driver::gpio_drive::middle,
    }).has_value();
    configure_interrupts();
    mcuhw::board_test::finish(evidence, [&](auto& observations) {
        observations("controller", "SPI1 interrupt-driven master");
        observations("device", "WIZnet W5100 using four-byte SPI frames");
        observations("device_register", "RTR 0x0017-0x0018; restored after probe");
        observations("sd_chip_select", "D4 held high");
        observations("completion_interrupt", "SPI1 SPII after IDLNF=0");
        observations.object("one_mhz", [&](auto& out) {
            write_observation(out, slow_observation, *timing_1mhz);
        });
        observations.object("six_mhz", [&](auto& out) {
            write_observation(out, fast_observation, *timing_6mhz);
        });
    });
}

extern "C" void IRQ0_Handler() noexcept {
    receive_interrupts = receive_interrupts + 1U;
    static_cast<void>(backend.handle_receive_interrupt());
    mcuhw::board_test::clear_icu_line(0U);
}
extern "C" void IRQ1_Handler() noexcept {
    transmit_interrupts = transmit_interrupts + 1U;
    static_cast<void>(backend.handle_transmit_empty_interrupt());
    mcuhw::board_test::clear_icu_line(1U);
}
extern "C" void IRQ2_Handler() noexcept {
    idle_interrupts = idle_interrupts + 1U;
    static_cast<void>(backend.handle_idle_interrupt());
    mcuhw::board_test::clear_icu_line(2U);
}
extern "C" void IRQ3_Handler() noexcept {
    error_interrupts = error_interrupts + 1U;
    static_cast<void>(backend.handle_error_interrupt());
    mcuhw::board_test::clear_icu_line(3U);
}
