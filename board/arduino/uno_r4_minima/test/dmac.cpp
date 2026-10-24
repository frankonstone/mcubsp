// SPDX-License-Identifier: Apache-2.0 OR MIT

// @req-file REQ-0013 REQ-0018 REQ-0019 REQ-0030 REQ-0036

#include <array>
#include <cstddef>
#include <cstdint>
#include <utility>

#include <mcutest/mcutest.hpp>

#include <mcubsp/backend/ra4m1/dmac.hpp>
#include <mcubsp/backend/ra4m1/spi.hpp>
#include <mcubsp/board/arduino_uno_r4_minima.hpp>
#include <mcubsp/driver/gpio.hpp>

#include "support.hpp"

namespace {

namespace board = mcubsp::board::arduino_uno_r4_minima;
using dmac0_type = mcubsp::backend::ra4m1::dmac_channel_backend<
    0, mcubsp::core::direct_mmio, mcubsp::board_test::execution_policy>;
using dmac1_type = mcubsp::backend::ra4m1::dmac_channel_backend<
    1, mcubsp::core::direct_mmio, mcubsp::board_test::execution_policy>;
using spi_binding = mcubsp::backend::ra4m1::spi_binding<
    1, board::d12_miso, board::d11_mosi, board::d13_sck, board::d10_ssl>;
using spi_type = mcubsp::backend::ra4m1::spi_master_backend<
    spi_binding, mcubsp::core::direct_mmio,
    mcubsp::board_test::execution_policy>;

mcubsp::board_test::execution_policy execution;
dmac0_type dmac0{mcubsp::board_test::mmio, execution};
dmac1_type dmac1{mcubsp::board_test::mmio, execution};
spi_type spi{mcubsp::board_test::mmio, execution};

constexpr mcubsp::core::pin_id sd_chip_select_pin{1U, 3U};
mcubsp::driver::gpio_pin sd_chip_select{
    mcubsp::board_test::gpio, sd_chip_select_pin};
bool sd_deselected{};

constexpr std::array<std::uint32_t, 16> flash_source{
    0x0123'4567U, 0x89AB'CDEFU, 0x1357'9BDFU, 0x2468'ACE0U,
    0xFFFF'0000U, 0x0000'FFFFU, 0xA5A5'5A5AU, 0x5A5A'A5A5U,
    0xDEAD'BEEFU, 0xC001'D00DU, 0x1020'3040U, 0x5060'7080U,
    0x0F1E'2D3CU, 0x4B5A'6978U, 0x8877'6655U, 0x4433'2211U,
};
std::array<std::uint32_t, flash_source.size()> flash_destination{};

std::array<std::uint8_t, 31> sram_source{
    0x01U, 0x03U, 0x05U, 0x07U, 0x09U, 0x0BU, 0x0DU, 0x0FU,
    0x11U, 0x13U, 0x15U, 0x17U, 0x19U, 0x1BU, 0x1DU, 0x1FU,
    0x21U, 0x23U, 0x25U, 0x27U, 0x29U, 0x2BU, 0x2DU, 0x2FU,
    0x31U, 0x33U, 0x35U, 0x37U, 0x39U, 0x3BU, 0x3DU,
};
std::array<std::uint8_t, sram_source.size()> sram_destination{};

volatile std::uint32_t transfer_end_interrupts{};
volatile std::uint32_t transmit_transfer_end_interrupts{};
volatile std::uint32_t spi_idle_interrupts{};
volatile std::uint32_t spi_error_interrupts{};

struct transfer_observation {
    bool started{};
    bool completed{};
    bool matched{};
    std::uintptr_t source{};
    std::uintptr_t destination{};
    std::uint32_t units{};
    std::uint32_t width_bytes{};
};

transfer_observation flash_copy{};
transfer_observation sram_copy{};
bool initialized{};
bool shutdown{};

constexpr auto spi_timing = mcubsp::backend::ra4m1::solve_spi_timing(
    {48'000'000U, 1'000'000U, 0U});
static_assert(spi_timing.has_value());
constexpr std::uint8_t w5100_read = 0x0FU;
constexpr std::uint8_t w5100_write = 0xF0U;
constexpr std::uint16_t w5100_retry_time = 0x0017U;
constexpr std::uint16_t probe_retry_time = 0xA55AU;

struct spi_dma_observation {
    bool dmac0_initialized{};
    bool dmac1_initialized{};
    bool spi_initialized{};
    bool original_read{};
    bool probe_written{};
    bool probe_read{};
    bool probe_matched{};
    bool restored{};
    bool restore_read{};
    bool restore_matched{};
    bool spi_shutdown{};
    bool dmac0_shutdown{};
    bool dmac1_shutdown{};
    std::uint16_t original_retry_time{};
    std::uint16_t probe_retry_time_read{};
    std::uint16_t restored_retry_time{};
    std::uint32_t transactions{};
    std::uint32_t frames{};
    std::uint32_t receive_dma_interrupts{};
    std::uint32_t transmit_dma_interrupts{};
    std::uint32_t idle_interrupts{};
    std::uint32_t error_interrupts{};
};

spi_dma_observation spi_dma{};

void configure_interrupts() noexcept {
    constexpr std::array events{
        mcubsp::mcu::ra4m1::event::dmac0_int,
        mcubsp::mcu::ra4m1::event::dmac1_int,
        mcubsp::mcu::ra4m1::event::spi1_idle,
        mcubsp::mcu::ra4m1::event::spi1_error,
    };
    *reinterpret_cast<volatile std::uint32_t*>(0xE000'E180U) = 0x0FU;
    for (std::uint32_t line = 0U; line < events.size(); ++line) {
        *reinterpret_cast<volatile std::uint32_t*>(0x4000'6300U + line * 4U)
            = mcubsp::mcu::ra4m1::event_number(events[line]);
        *reinterpret_cast<volatile std::uint8_t*>(0xE000'E400U + line)
            = 0x80U;
    }
    *reinterpret_cast<volatile std::uint32_t*>(0xE000'E280U) = 0x0FU;
    *reinterpret_cast<volatile std::uint32_t*>(0xE000'E100U) = 0x0FU;
    __asm__ volatile("dsb sy\n\tisb sy\n\tcpsie i" ::: "memory");
}

[[nodiscard]] mcubsp::execution::deadline deadline_after(
    std::uint32_t cycles) noexcept {
    return {static_cast<std::uint32_t>(
        mcubsp::board_test::execution_hooks::now() + cycles)};
}

template<typename Source, typename Destination>
[[nodiscard]] bool transfer_and_compare(
    Source& source, Destination& destination, mcubsp::driver::dma_width width,
    transfer_observation& observation) noexcept {
    observation.source = reinterpret_cast<std::uintptr_t>(source.data());
    observation.destination =
        reinterpret_cast<std::uintptr_t>(destination.data());
    observation.units = static_cast<std::uint32_t>(source.size());
    observation.width_bytes = static_cast<std::uint32_t>(mcubsp::driver::bytes(width));

    auto started = dmac0.copy({
        .source = observation.source,
        .destination = observation.destination,
        .count = observation.units,
        .width = width,
    });
    observation.started = started.has_value();
    if (!started) return false;

    auto transfer = std::move(*started);
    if (!transfer.wait_until(deadline_after(480'000U))) {
        static_cast<void>(transfer.take());
        return false;
    }
    const auto result = transfer.take();
    observation.completed = result.has_value()
        && result->units == observation.units && result->width == width;
    if (!observation.completed) return false;

    observation.matched = source == destination;
    return observation.matched;
}

[[nodiscard]] bool spi_dma_transact(
    const std::array<std::uint8_t, 4>& command,
    std::array<std::uint8_t, 4>& response) noexcept {
    auto started = spi.transaction_dma({command, response}, dmac0, dmac1);
    if (!started) return false;
    auto transfer = std::move(*started);
    if (!transfer.wait_until(deadline_after(480'000U))) {
        static_cast<void>(transfer.take());
        return false;
    }
    const auto result = transfer.take();
    if (!result || result->frames != command.size()) return false;
    ++spi_dma.transactions;
    spi_dma.frames += static_cast<std::uint32_t>(result->frames);
    return true;
}

[[nodiscard]] bool spi_dma_read_register(std::uint16_t address,
                                         std::uint8_t& value) noexcept {
    const std::array<std::uint8_t, 4> command{
        w5100_read,
        static_cast<std::uint8_t>(address >> 8U),
        static_cast<std::uint8_t>(address),
        0U,
    };
    std::array<std::uint8_t, 4> response{};
    if (!spi_dma_transact(command, response)) return false;
    value = response[3];
    return true;
}

[[nodiscard]] bool spi_dma_write_register(std::uint16_t address,
                                          std::uint8_t value) noexcept {
    const std::array<std::uint8_t, 4> command{
        w5100_write,
        static_cast<std::uint8_t>(address >> 8U),
        static_cast<std::uint8_t>(address),
        value,
    };
    std::array<std::uint8_t, 4> response{};
    return spi_dma_transact(command, response);
}

[[nodiscard]] bool spi_dma_read_retry_time(std::uint16_t& value) noexcept {
    std::uint8_t high{};
    std::uint8_t low{};
    const bool read = spi_dma_read_register(w5100_retry_time, high)
        && spi_dma_read_register(w5100_retry_time + 1U, low);
    value = static_cast<std::uint16_t>(
        (static_cast<std::uint16_t>(high) << 8U) | low);
    return read;
}

[[nodiscard]] bool spi_dma_write_retry_time(std::uint16_t value) noexcept {
    return spi_dma_write_register(
               w5100_retry_time, static_cast<std::uint8_t>(value >> 8U))
        && spi_dma_write_register(
               w5100_retry_time + 1U, static_cast<std::uint8_t>(value));
}

TEST(uno_dmac, flash_and_sram_known_answer_copies_complete_by_interrupt,
     "REQ-0013", "REQ-0018", "REQ-0019", "REQ-0030", "REQ-0036") {
    initialized = dmac0.initialize().has_value();
    ASSERT_TRUE(initialized);
    ASSERT_TRUE(transfer_and_compare(
        flash_source, flash_destination, mcubsp::driver::dma_width::bits32,
        flash_copy));
    ASSERT_TRUE(transfer_and_compare(
        sram_source, sram_destination, mcubsp::driver::dma_width::bits8,
        sram_copy));
    ASSERT_EQ(transfer_end_interrupts, 2U);
    shutdown = dmac0.shutdown().has_value();
    ASSERT_TRUE(shutdown);
}

TEST(uno_dmac, spi1_w5100_full_duplex_dma_read_write_restore_and_idle,
     "REQ-0017", "REQ-0018", "REQ-0019", "REQ-0028", "REQ-0030",
     "REQ-0036") {
    ASSERT_TRUE(sd_deselected);
    const auto receive_before = transfer_end_interrupts;
    const auto transmit_before = transmit_transfer_end_interrupts;
    const auto idle_before = spi_idle_interrupts;
    const auto error_before = spi_error_interrupts;

    spi_dma.dmac0_initialized = dmac0.initialize().has_value();
    spi_dma.dmac1_initialized = dmac1.initialize().has_value();
    spi_dma.spi_initialized = spi.initialize(*spi_timing, {
        .mode = mcubsp::driver::spi_mode::mode0,
        .bit_order = mcubsp::driver::spi_bit_order::most_significant_first,
        .frame_width = mcubsp::driver::spi_frame_width::bits8,
    }).has_value();
    ASSERT_TRUE(spi_dma.dmac0_initialized);
    ASSERT_TRUE(spi_dma.dmac1_initialized);
    ASSERT_TRUE(spi_dma.spi_initialized);

    spi_dma.original_read = spi_dma_read_retry_time(
        spi_dma.original_retry_time);
    ASSERT_TRUE(spi_dma.original_read);
    spi_dma.probe_written = spi_dma_write_retry_time(probe_retry_time);
    ASSERT_TRUE(spi_dma.probe_written);
    spi_dma.probe_read = spi_dma_read_retry_time(
        spi_dma.probe_retry_time_read);
    spi_dma.probe_matched = spi_dma.probe_read
        && spi_dma.probe_retry_time_read == probe_retry_time;

    spi_dma.restored = spi_dma_write_retry_time(spi_dma.original_retry_time);
    spi_dma.restore_read = spi_dma.restored
        && spi_dma_read_retry_time(spi_dma.restored_retry_time);
    spi_dma.restore_matched = spi_dma.restore_read
        && spi_dma.restored_retry_time == spi_dma.original_retry_time;

    spi_dma.spi_shutdown = spi.shutdown().has_value();
    spi_dma.dmac0_shutdown = dmac0.shutdown().has_value();
    spi_dma.dmac1_shutdown = dmac1.shutdown().has_value();
    spi_dma.receive_dma_interrupts = transfer_end_interrupts - receive_before;
    spi_dma.transmit_dma_interrupts =
        transmit_transfer_end_interrupts - transmit_before;
    spi_dma.idle_interrupts = spi_idle_interrupts - idle_before;
    spi_dma.error_interrupts = spi_error_interrupts - error_before;

    ASSERT_TRUE(spi_dma.probe_matched);
    ASSERT_TRUE(spi_dma.restore_matched);
    ASSERT_TRUE(spi_dma.spi_shutdown);
    ASSERT_TRUE(spi_dma.dmac0_shutdown);
    ASSERT_TRUE(spi_dma.dmac1_shutdown);
    ASSERT_EQ(spi_dma.receive_dma_interrupts, spi_dma.transactions);
    ASSERT_EQ(spi_dma.transmit_dma_interrupts, spi_dma.transactions);
    ASSERT_EQ(spi_dma.idle_interrupts, spi_dma.transactions);
    ASSERT_EQ(spi_dma.error_interrupts, 0U);
}

constexpr char metadata[] =
    "MCUBSP_META board=Arduino_UNO_R4_Minima suite=dmac "
    "firmware=pln-0010 fixture=internal_memory_and_Ethernet_Shield_R3_W5100 "
    "channels=DMAC0,DMAC1 activation=software,SPI1_RXI,SPI1_TXI "
    "completion=DMAC0_INT,DMAC1_INT,SPI1_SPII "
    "widths=8,32 toolchain=arm-none-eabi-gcc-15.2.1 "
    "iclk_hz=48000000 interface=J-Link_SWD_RTT\n";
constexpr mcubsp::board_test::evidence_metadata evidence{
    .suite = "dmac",
    .firmware = "pln-0010",
    .fixture = "internal memory plus stacked Arduino Ethernet Shield R3 W5100; network cable attached; SD CS D4 high",
    .coverage_build_id = 0x4d485701U,
};

template<typename Writer>
void write_observation(Writer& out, const transfer_observation& value) noexcept {
    out("started", value.started);
    out("completed", value.completed);
    out("matched", value.matched);
    out("source_address", static_cast<std::uint32_t>(value.source));
    out("destination_address", static_cast<std::uint32_t>(value.destination));
    out("units", value.units);
    out("width_bytes", value.width_bytes);
}

} // namespace

int main() {
    mcubsp::board_test::start(metadata);
    sd_deselected = sd_chip_select.initialize({
        .direction = mcubsp::driver::gpio_direction::output,
        .initial_level = mcubsp::driver::gpio_level::high,
        .output_type = mcubsp::driver::gpio_output_type::push_pull,
        .pull = mcubsp::driver::gpio_pull::none,
        .drive = mcubsp::driver::gpio_drive::middle,
    }).has_value();
    configure_interrupts();
    mcubsp::board_test::finish(evidence, [&](auto& observations) {
        observations("engine", "RA4M1 DMAC0 finite normal mode");
        observations("activation", "software request with continuous request select");
        observations("completion", "DMAC0_INT after DTIF with ACT clear");
        observations("initialized", initialized);
        observations("shutdown", shutdown);
        observations("transfer_end_interrupts", transfer_end_interrupts);
        observations.object("flash_to_sram_32_bit", [&](auto& out) {
            write_observation(out, flash_copy);
        });
        observations.object("sram_to_sram_8_bit", [&](auto& out) {
            write_observation(out, sram_copy);
        });
        observations.object("spi1_w5100_full_duplex_dma", [&](auto& out) {
            out("mode", "8-bit equal-length full duplex");
            out("target_hz", spi_timing->target_hz);
            out("achieved_hz", spi_timing->achieved_hz);
            out("original_retry_time", spi_dma.original_retry_time);
            out("probe_retry_time", probe_retry_time);
            out("probe_retry_time_read", spi_dma.probe_retry_time_read);
            out("probe_matched", spi_dma.probe_matched);
            out("restored_retry_time", spi_dma.restored_retry_time);
            out("restore_matched", spi_dma.restore_matched);
            out("transactions", spi_dma.transactions);
            out("frames", spi_dma.frames);
            out("receive_dma_interrupts", spi_dma.receive_dma_interrupts);
            out("transmit_dma_interrupts", spi_dma.transmit_dma_interrupts);
            out("idle_interrupts", spi_dma.idle_interrupts);
            out("error_interrupts", spi_dma.error_interrupts);
        });
    });
}

extern "C" void IRQ0_Handler() noexcept {
    transfer_end_interrupts = transfer_end_interrupts + 1U;
    static_cast<void>(dmac0.handle_transfer_end_interrupt());
    static_cast<void>(spi.handle_dma_receive_completion());
    mcubsp::board_test::clear_icu_line(0U);
}

extern "C" void IRQ1_Handler() noexcept {
    transmit_transfer_end_interrupts = transmit_transfer_end_interrupts + 1U;
    static_cast<void>(dmac1.handle_transfer_end_interrupt());
    static_cast<void>(spi.handle_dma_transmit_completion());
    mcubsp::board_test::clear_icu_line(1U);
}

extern "C" void IRQ2_Handler() noexcept {
    spi_idle_interrupts = spi_idle_interrupts + 1U;
    static_cast<void>(spi.handle_idle_interrupt());
    mcubsp::board_test::clear_icu_line(2U);
}

extern "C" void IRQ3_Handler() noexcept {
    spi_error_interrupts = spi_error_interrupts + 1U;
    static_cast<void>(spi.handle_error_interrupt());
    mcubsp::board_test::clear_icu_line(3U);
}
