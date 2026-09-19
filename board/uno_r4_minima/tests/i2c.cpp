#include <array>
#include <cstddef>
#include <cstdint>
#include <utility>

#include <mcutest/mcutest.hpp>

#include <mcuhw/backend/ra4m1/iic.hpp>
#include <mcuhw/board/arduino_uno_r4_minima.hpp>
#include <mcuhw/driver/i2c.hpp>

#include "support.hpp"

namespace {

using binding = mcuhw::backend::ra4m1::iic_binding<
    1, mcuhw::board::arduino_uno_r4_minima::a5_scl,
    mcuhw::board::arduino_uno_r4_minima::a4_sda>;
using backend_type = mcuhw::backend::ra4m1::iic_controller_backend<
    binding, mcuhw::core::direct_mmio, mcuhw::board_test::execution_policy>;

mcuhw::board_test::execution_policy execution;
backend_type backend{mcuhw::board_test::mmio, execution};
mcuhw::driver::i2c_controller bus{backend};
mcuhw::driver::i2c_device<decltype(bus), 0x50U> eeprom{bus};

constexpr std::uint16_t scratch_address = 0x0010U;
constexpr std::size_t payload_size = 8U;
constexpr auto device_address = mcuhw::driver::make_i2c_address(0x50U);
static_assert(device_address.has_value());

constexpr auto timing_100khz = mcuhw::driver::solve_i2c_timing({
    .clock_hz = 24'000'000U,
    .target_hz = 100'000U,
    .maximum_error_ppm = 20'000U,
    .rise_time_ns = 300U,
    .fall_time_ns = 100U,
});
constexpr auto timing_400khz = mcuhw::driver::solve_i2c_timing({
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
    bool original_read{};
    bool pattern_written{};
    bool pattern_read{};
    bool pattern_matched{};
    bool original_restored{};
    bool restore_verified{};
    std::array<std::uint8_t, payload_size> original{};
    std::array<std::uint8_t, payload_size> observed{};
    std::array<std::uint8_t, payload_size> restored{};
};

rate_observation standard_observation{};
rate_observation fast_observation{};

void configure_interrupts() noexcept {
    constexpr std::array<std::uint16_t, 4> events{
        mcuhw::mcu::ra4m1::event_number(mcuhw::mcu::ra4m1::event::iic1_rxi),
        mcuhw::mcu::ra4m1::event_number(mcuhw::mcu::ra4m1::event::iic1_txi),
        mcuhw::mcu::ra4m1::event_number(mcuhw::mcu::ra4m1::event::iic1_tei),
        mcuhw::mcu::ra4m1::event_number(mcuhw::mcu::ra4m1::event::iic1_eei),
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

[[nodiscard]] bool read_bytes(
    std::array<std::uint8_t, payload_size>& destination) noexcept {
    mcuhw::driver::i2c_register_request<payload_size, 2> request;
    auto started = eeprom.read_register(
        scratch_address, request, destination.size(),
        mcuhw::driver::register_byte_order::most_significant_first);
    if (!started) {
        return false;
    }
    auto transfer = std::move(*started);
    if (!transfer.wait_until(deadline_after(480'000U))) {
        return false;
    }
    const auto result = transfer.take();
    if (!result || result->bytes_written != 2U
        || result->bytes_read != destination.size()) {
        return false;
    }
    destination = request.data;
    return true;
}

[[nodiscard]] bool wait_until_ready() noexcept {
    constexpr std::array<std::uint8_t, 2> address_bytes{
        static_cast<std::uint8_t>(scratch_address >> 8U),
        static_cast<std::uint8_t>(scratch_address),
    };
    const auto finish = deadline_after(960'000U);
    while (!mcuhw::execution::reached(
        mcuhw::board_test::execution_hooks::now(), finish)) {
        auto started = bus.write(*device_address, address_bytes);
        if (!started) {
            return false;
        }
        auto transfer = std::move(*started);
        if (!transfer.wait_until(deadline_after(96'000U))) {
            return false;
        }
        const auto result = transfer.take();
        if (result) {
            return result->bytes_written == address_bytes.size();
        }
        if (result.error().cause
            != mcuhw::driver::i2c_error::address_nack) {
            return false;
        }
    }
    return false;
}

[[nodiscard]] bool write_bytes(
    const std::array<std::uint8_t, payload_size>& source) noexcept {
    std::array<std::uint8_t, payload_size + 2U> transaction{};
    transaction[0] = static_cast<std::uint8_t>(scratch_address >> 8U);
    transaction[1] = static_cast<std::uint8_t>(scratch_address);
    for (std::size_t index = 0U; index < source.size(); ++index) {
        transaction[index + 2U] = source[index];
    }
    auto started = bus.write(*device_address, transaction);
    if (!started) {
        return false;
    }
    auto transfer = std::move(*started);
    if (!transfer.wait_until(deadline_after(480'000U))) {
        return false;
    }
    const auto result = transfer.take();
    return result && result->bytes_written == transaction.size()
        && wait_until_ready();
}

template<typename Timing>
[[nodiscard]] bool exercise_rate(const Timing& timing,
                                 std::uint8_t seed,
                                 rate_observation& observation) noexcept {
    observation.initialized = bus.initialize(timing).has_value();
    if (!observation.initialized) {
        return false;
    }

    observation.original_read = read_bytes(observation.original);
    if (!observation.original_read) {
        static_cast<void>(bus.shutdown());
        return false;
    }

    std::array<std::uint8_t, payload_size> pattern{};
    for (std::size_t index = 0U; index < pattern.size(); ++index) {
        pattern[index] = static_cast<std::uint8_t>(seed + index * 0x17U);
    }
    observation.pattern_written = write_bytes(pattern);
    if (observation.pattern_written) {
        observation.pattern_read = read_bytes(observation.observed);
        observation.pattern_matched = observation.pattern_read
            && observation.observed == pattern;
    }

    // Restore the fixture's original EEPROM contents even when verification
    // fails after the page write.
    observation.original_restored = write_bytes(observation.original);
    if (observation.original_restored) {
        observation.restore_verified = read_bytes(observation.restored)
            && observation.restored == observation.original;
    }
    const bool shutdown = bus.shutdown().has_value();
    return observation.pattern_written && observation.pattern_read
        && observation.pattern_matched && observation.original_restored
        && observation.restore_verified && shutdown;
}

TEST(uno_i2c, iic1_a4_a5_24lc256_write_and_repeated_start_read,
     "REQ-0024", "REQ-0025", "REQ-0026", "REQ-0036") {
    ASSERT_TRUE(exercise_rate(*timing_100khz, 0x31U,
                              standard_observation));
    ASSERT_TRUE(exercise_rate(*timing_400khz, 0xA6U,
                              fast_observation));
}

constexpr char metadata[] =
    "MCUHW_META board=Arduino_UNO_R4_Minima suite=i2c "
    "firmware=pln-0008 fixture=A5_IIC1_SCL_A4_IIC1_SDA_to_24LC256_0x50 "
    "rates_hz=100000,400000 transaction=page_write_and_repeated_start_read "
    "pullups=external_to_3V3 toolchain=arm-none-eabi-gcc-15.2.1 "
    "iclk_hz=48000000 pclka_hz=48000000 pclkb_hz=24000000 "
    "pclkd_hz=48000000 interface=J-Link_SWD_RTT\n";
constexpr mcuhw::board_test::evidence_metadata evidence{
    .suite = "i2c",
    .firmware = "pln-0008",
    .fixture = "A5 IIC1 SCL and A4 IIC1 SDA to 24LC256 address 0x50 with external 3.3 V pull-ups",
    .coverage_build_id = 0x4d485701U,
};

template<typename Writer>
void write_rate_observation(Writer& rate, const rate_observation& observation,
                            const mcuhw::driver::i2c_timing& timing) noexcept {
    rate("target_hz", timing.target_hz);
    rate("achieved_hz", timing.achieved_hz);
    rate("error_ppm", timing.error_ppm);
    rate("initialized", observation.initialized);
    rate("original_read", observation.original_read);
    rate("pattern_written", observation.pattern_written);
    rate("pattern_read", observation.pattern_read);
    rate("pattern_matched", observation.pattern_matched);
    rate("original_restored", observation.original_restored);
    rate("restore_verified", observation.restore_verified);
}

} // namespace

int main() {
    mcuhw::board_test::start(metadata);
    configure_interrupts();
    mcuhw::board_test::finish(evidence, [&](auto& observations) {
        observations("controller", "IIC1 interrupt-driven controller");
        observations("device", "24LC256-compatible EEPROM at 0x50");
        observations("scratch_address", scratch_address);
        observations("payload_bytes", payload_size);
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
    static_cast<void>(backend.handle_receive_interrupt());
    mcuhw::board_test::clear_icu_line(0U);
}

extern "C" void IRQ1_Handler() noexcept {
    static_cast<void>(backend.handle_transmit_empty_interrupt());
    mcuhw::board_test::clear_icu_line(1U);
}

extern "C" void IRQ2_Handler() noexcept {
    static_cast<void>(backend.handle_transmit_end_interrupt());
    mcuhw::board_test::clear_icu_line(2U);
}

extern "C" void IRQ3_Handler() noexcept {
    static_cast<void>(backend.handle_error_interrupt());
    mcuhw::board_test::clear_icu_line(3U);
}
