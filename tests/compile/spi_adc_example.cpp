#include <array>
#include <concepts>
#include <cstdint>
#include <expected>
#include <utility>

#include <mcuhw/backend/ra4m1/adc.hpp>
#include <mcuhw/backend/ra4m1/spi.hpp>
#include <mcuhw/board/arduino_uno_r4_minima.hpp>
#include <mcuhw/core/mmio.hpp>
#include <mcuhw/execution/fake_policy.hpp>

namespace {
struct example_mmio {
    template<std::unsigned_integral Value>
    [[nodiscard]] std::expected<Value, mcuhw::core::mmio_error>
    read(std::uintptr_t) noexcept { return Value{}; }
    template<std::unsigned_integral Value>
    [[nodiscard]] std::expected<void, mcuhw::core::mmio_error>
    write(std::uintptr_t, Value) noexcept { return {}; }
};
namespace board = mcuhw::board::arduino_uno_r4_minima;
using spi_binding = mcuhw::backend::ra4m1::spi_binding<
    1, board::d12_miso, board::d11_mosi, board::d13_sck, board::d10_ssl>;
using adc_binding = mcuhw::backend::ra4m1::adc14_binding<
    mcuhw::backend::ra4m1::adc_input<0, board::a1_an000>,
    mcuhw::backend::ra4m1::adc_input<1, board::a2_an001>>;
using execution = mcuhw::execution::fake_policy;
using spi_backend = mcuhw::backend::ra4m1::spi_master_backend<
    spi_binding, example_mmio, execution>;
using adc_backend = mcuhw::backend::ra4m1::adc14_backend<
    adc_binding, example_mmio, execution>;
constexpr auto timing = mcuhw::driver::solve_spi_timing(
    {48'000'000U, 1'000'000U, 0U});
static_assert(timing.has_value());
} // namespace

// @req REQ-0028 REQ-0029 REQ-0038
int mcuhw_spi_adc_example() {
    example_mmio mmio;
    execution policy;
    spi_backend spi{mmio, policy};
    if (!spi.initialize(*timing)) return 1;
    const std::array<std::uint16_t, 1> data{0x5AU};
    const std::array segments{mcuhw::driver::spi_segment{.transmit = data}};
    auto transfer = spi.transaction(segments);
    if (!transfer) return 2;
    static_cast<void>(transfer->cancel());
    static_cast<void>(transfer->take());
    if (!spi.shutdown()) return 3;

    adc_backend adc{mmio, policy};
    if (!adc.initialize()) return 4;
    const std::array channels{mcuhw::driver::adc_channel{0U}};
    std::array<std::uint16_t, 1> results{};
    auto conversion = adc.convert(channels, results);
    if (!conversion) return 5;
    static_cast<void>(conversion->cancel());
    static_cast<void>(conversion->take());
    return adc.shutdown() ? 0 : 6;
}
