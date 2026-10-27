// SPDX-License-Identifier: Apache-2.0 OR MIT

// @req-file REQ-0017 REQ-0019 REQ-0024 REQ-0025 REQ-0026 REQ-0036

#include <array>
#include <cstddef>
#include <cstdint>
#include <utility>

#include <mcutest/mcutest.hpp>

#include <mcubsp/backend/ra4m1/iic.hpp>
#include <mcubsp/board/arduino/uno_r4_minima/bindings.hpp>
#include <mcubsp/driver/i2c.hpp>

#include "support.hpp"

namespace {

using binding = mcubsp::backend::ra4m1::iic_binding<
    1, mcubsp::board::arduino::uno_r4_minima_bindings::a5_scl,
    mcubsp::board::arduino::uno_r4_minima_bindings::a4_sda>;
using backend_type = mcubsp::backend::ra4m1::iic_controller_backend<
    binding, mcubsp::core::direct_mmio, mcubsp::board_test::execution_policy>;

mcubsp::board_test::execution_policy execution;
backend_type backend{mcubsp::board_test::mmio, execution};
mcubsp::driver::i2c_controller bus{backend};
mcubsp::driver::i2c_device<decltype(bus), 0x73U> gesture_sensor{bus};

volatile std::uint32_t receive_interrupts{};
volatile std::uint32_t transmit_interrupts{};
volatile std::uint32_t transmit_end_interrupts{};
volatile std::uint32_t error_interrupts{};

constexpr std::uint8_t bank_select_register = 0xEFU;
constexpr std::uint8_t bank_zero = 0x00U;
constexpr std::uint8_t part_id_register = 0x00U;
constexpr std::array<std::uint8_t, 2> expected_identity{0x20U, 0x76U};
constexpr auto device_address = mcubsp::driver::make_i2c_address(0x73U);
static_assert(device_address.has_value());

constexpr auto timing_100khz = mcubsp::backend::ra4m1::solve_i2c_timing({
    .clock_hz = 24'000'000U,
    .target_hz = 100'000U,
    .maximum_error_ppm = 20'000U,
    .rise_time_ns = 300U,
    .fall_time_ns = 100U,
});
constexpr auto timing_400khz = mcubsp::backend::ra4m1::solve_i2c_timing({
    .clock_hz = 24'000'000U,
    .target_hz = 400'000U,
    .maximum_error_ppm = 30'000U,
    .rise_time_ns = 120U,
    .fall_time_ns = 60U,
});
static_assert(timing_100khz.has_value());
static_assert(timing_400khz.has_value());

struct rate_observation {
    bool initialized{};
    bool bank_select_started{};
    bool bank_selected{};
    bool identity_read_started{};
    bool identity_read{};
    bool identity_matched{};
    bool shutdown{};
    std::uint8_t bank_error_kind{0xFFU};
    std::uint8_t bank_error_cause{0xFFU};
    std::uint8_t identity_error_kind{0xFFU};
    std::uint8_t identity_error_cause{0xFFU};
    std::uint32_t receive_interrupt_count{};
    std::uint32_t transmit_interrupt_count{};
    std::uint32_t transmit_end_interrupt_count{};
    std::uint32_t error_interrupt_count{};
    std::array<std::uint8_t, expected_identity.size()> identity{};
};

rate_observation standard_observation{};
rate_observation fast_observation{};

void configure_interrupts() noexcept {
    constexpr std::array<std::uint16_t, 4> events{
        mcubsp::mcu::ra4m1::event_number(mcubsp::mcu::ra4m1::event::iic1_rxi),
        mcubsp::mcu::ra4m1::event_number(mcubsp::mcu::ra4m1::event::iic1_txi),
        mcubsp::mcu::ra4m1::event_number(mcubsp::mcu::ra4m1::event::iic1_tei),
        mcubsp::mcu::ra4m1::event_number(mcubsp::mcu::ra4m1::event::iic1_eei),
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

[[nodiscard]] mcubsp::execution::deadline deadline_after(
    std::uint32_t cycles) noexcept {
    return {static_cast<std::uint32_t>(
        mcubsp::board_test::execution_hooks::now() + cycles)};
}

template<typename Timing>
[[nodiscard]] bool exercise_rate(const Timing& timing,
                                 rate_observation& observation) noexcept {
    const auto receive_before = receive_interrupts;
    const auto transmit_before = transmit_interrupts;
    const auto transmit_end_before = transmit_end_interrupts;
    const auto error_before = error_interrupts;
    observation.initialized = bus.initialize(timing).has_value();
    if (!observation.initialized) {
        return false;
    }

    constexpr std::array<std::uint8_t, 2> bank_selection{
        bank_select_register, bank_zero};
    auto bank_started = bus.write(*device_address, bank_selection);
    observation.bank_select_started = bank_started.has_value();
    if (bank_started) {
        auto transfer = std::move(*bank_started);
        static_cast<void>(transfer.wait_until(deadline_after(480'000U)));
        const auto result = transfer.take();
        observation.bank_selected = result.has_value()
            && result->bytes_written == bank_selection.size();
        if (!result) {
            observation.bank_error_kind =
                static_cast<std::uint8_t>(result.error().kind);
            if (result.error().cause) {
                observation.bank_error_cause =
                    static_cast<std::uint8_t>(*result.error().cause);
            }
        }
    } else {
        observation.bank_error_kind =
            static_cast<std::uint8_t>(bank_started.error().kind);
        if (bank_started.error().cause) {
            observation.bank_error_cause =
                static_cast<std::uint8_t>(*bank_started.error().cause);
        }
    }

    if (observation.bank_selected) {
        mcubsp::driver::i2c_register_request<expected_identity.size()> request;
        auto identity_started = gesture_sensor.read_register(
            part_id_register, request, expected_identity.size());
        observation.identity_read_started = identity_started.has_value();
        if (identity_started) {
            auto transfer = std::move(*identity_started);
            static_cast<void>(transfer.wait_until(deadline_after(480'000U)));
            const auto result = transfer.take();
            observation.identity_read = result.has_value()
                && result->bytes_written == 1U
                && result->bytes_read == expected_identity.size();
            if (observation.identity_read) {
                observation.identity = request.data;
                observation.identity_matched =
                    observation.identity == expected_identity;
            } else if (!result) {
                observation.identity_error_kind =
                    static_cast<std::uint8_t>(result.error().kind);
                if (result.error().cause) {
                    observation.identity_error_cause =
                        static_cast<std::uint8_t>(*result.error().cause);
                }
            }
        } else {
            observation.identity_error_kind =
                static_cast<std::uint8_t>(identity_started.error().kind);
            if (identity_started.error().cause) {
                observation.identity_error_cause =
                    static_cast<std::uint8_t>(*identity_started.error().cause);
            }
        }
    }

    observation.shutdown = bus.shutdown().has_value();
    observation.receive_interrupt_count = receive_interrupts - receive_before;
    observation.transmit_interrupt_count = transmit_interrupts - transmit_before;
    observation.transmit_end_interrupt_count =
        transmit_end_interrupts - transmit_end_before;
    observation.error_interrupt_count = error_interrupts - error_before;
    return observation.bank_select_started && observation.bank_selected
        && observation.identity_read_started && observation.identity_read
        && observation.identity_matched && observation.shutdown;
}

TEST(uno_i2c, iic1_a4_a5_paj7620u2_identity_write_and_repeated_start_read,
     "REQ-0024", "REQ-0025", "REQ-0026", "REQ-0036") {
    ASSERT_TRUE(exercise_rate(*timing_100khz, standard_observation));
    ASSERT_TRUE(exercise_rate(*timing_400khz, fast_observation));
}

constexpr char metadata[] =
    "MCUBSP_META board=Arduino_UNO_R4_Minima suite=i2c "
    "firmware=pln-0008 fixture=A5_IIC1_SCL_A4_IIC1_SDA_to_PAJ7620U2_0x73 "
    "rates_hz=100000,400000 transaction=bank_select_and_repeated_start_identity_read "
    "pullups=grove_module toolchain=arm-none-eabi-gcc-15.2.1 "
    "iclk_hz=48000000 pclka_hz=48000000 pclkb_hz=24000000 "
    "pclkd_hz=48000000 interface=J-Link_SWD_RTT\n";
constexpr mcubsp::board_test::evidence_metadata evidence{
    .suite = "i2c",
    .firmware = "pln-0008",
    .fixture = "A5 IIC1 SCL and A4 IIC1 SDA to Grove Gesture PAJ7620U2 address 0x73",
    .coverage_build_id = 0x4d485701U,
};

template<typename Writer>
void write_rate_observation(Writer& rate, const rate_observation& observation,
                            const mcubsp::backend::ra4m1::i2c_timing& timing) noexcept {
    rate("target_hz", timing.target_hz);
    rate("achieved_hz", timing.achieved_hz);
    rate("error_ppm", timing.error_ppm);
    rate("initialized", observation.initialized);
    rate("bank_select_started", observation.bank_select_started);
    rate("bank_selected", observation.bank_selected);
    rate("identity_read_started", observation.identity_read_started);
    rate("identity_read", observation.identity_read);
    rate("identity_matched", observation.identity_matched);
    rate("bank_error_kind", observation.bank_error_kind);
    rate("bank_error_cause", observation.bank_error_cause);
    rate("identity_error_kind", observation.identity_error_kind);
    rate("identity_error_cause", observation.identity_error_cause);
    rate("receive_interrupts", observation.receive_interrupt_count);
    rate("transmit_interrupts", observation.transmit_interrupt_count);
    rate("transmit_end_interrupts", observation.transmit_end_interrupt_count);
    rate("error_interrupts", observation.error_interrupt_count);
    rate("part_id_low", observation.identity[0]);
    rate("part_id_high", observation.identity[1]);
    rate("shutdown", observation.shutdown);
}

} // namespace

int main() {
    mcubsp::board_test::start(metadata);
    configure_interrupts();
    mcubsp::board_test::finish(evidence, [&](auto& observations) {
        observations("controller", "IIC1 interrupt-driven controller");
        observations("device", "Grove Gesture PAJ7620U2 at 0x73");
        observations("expected_part_id", 0x7620U);
        observations.object("standard_mode", [&](auto& rate) {
            write_rate_observation(rate, standard_observation,
                                   *timing_100khz);
        });
        observations.object("fast_mode", [&](auto& rate) {
            write_rate_observation(rate, fast_observation, *timing_400khz);
        });
    });
}

extern "C" void IRQ0_Handler() noexcept {
    receive_interrupts = receive_interrupts + 1U;
    static_cast<void>(backend.handle_receive_interrupt());
    mcubsp::board_test::clear_icu_line(0U);
}

extern "C" void IRQ1_Handler() noexcept {
    transmit_interrupts = transmit_interrupts + 1U;
    static_cast<void>(backend.handle_transmit_empty_interrupt());
    mcubsp::board_test::clear_icu_line(1U);
}

extern "C" void IRQ2_Handler() noexcept {
    transmit_end_interrupts = transmit_end_interrupts + 1U;
    static_cast<void>(backend.handle_transmit_end_interrupt());
    mcubsp::board_test::clear_icu_line(2U);
}

extern "C" void IRQ3_Handler() noexcept {
    error_interrupts = error_interrupts + 1U;
    static_cast<void>(backend.handle_error_interrupt());
    mcubsp::board_test::clear_icu_line(3U);
}
