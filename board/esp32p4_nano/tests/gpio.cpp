// SPDX-License-Identifier: Apache-2.0 OR MIT

// @req-file REQ-0009 REQ-0034 REQ-0036

#include "support.hpp"

#include <cstdint>

#include <mcutest/assert.hpp>
#include <mcutest/core.hpp>

#include <mcucov/mcucov.h>

#include <mcuhw/backend/esp32p4/gpio.hpp>
#include <mcuhw/core/direct_mmio.hpp>
#include <mcuhw/driver/gpio.hpp>
#include <mcuhw/mcu/esp32p4/pins.hpp>

extern "C" MCUCOV_NO_INSTRUMENT void
mcuhw_esp32p4_gpio_test_link_anchor() noexcept {}

namespace mcuhw::board_test::esp32p4_nano {
namespace {

constexpr core::pin_id smoke_pin{0, 45};
constexpr std::uint32_t smoke_mask = 1U << (smoke_pin.index - 32U);

[[nodiscard]] volatile const std::uint32_t&
register_at(std::uintptr_t address) noexcept {
    return *reinterpret_cast<volatile const std::uint32_t*>(address);
}

} // namespace

TEST(esp32p4_gpio, gpio45_initializes_toggles_and_releases,
     "REQ-0009", "REQ-0034", "REQ-0036") {
    core::direct_mmio mmio;
    backend::esp32p4::gpio_backend backend{mmio};
    driver::gpio_pin pin{backend, smoke_pin};

    const auto initialized = pin.initialize({
        .direction = driver::gpio_direction::output,
        .initial_level = driver::gpio_level::low,
        .output_type = driver::gpio_output_type::push_pull,
        .pull = driver::gpio_pull::none,
        .drive = driver::gpio_drive::low,
    });
    ASSERT_TRUE(initialized.has_value());
    if (!initialized) return;

    const auto gpio_base = mcu::esp32p4::gpio_matrix_base;
    const auto io_mux_base = mcu::esp32p4::io_mux_base;
    const auto output_select = register_at(
        gpio_base + 0x558U + smoke_pin.index * 4U);
    const auto pad = register_at(io_mux_base + 0x04U + smoke_pin.index * 4U);
    ASSERT_TRUE((register_at(gpio_base + 0x2CU) & smoke_mask) != 0U);
    ASSERT_TRUE((output_select & 0xFFFU) == (256U | (1U << 10U)));
    ASSERT_TRUE(((pad >> 12U) & 0x7U) == 1U);

    const auto set = pin.set();
    ASSERT_TRUE(set.has_value());
    if (!set) {
        static_cast<void>(pin.release());
        return;
    }
    ASSERT_TRUE((register_at(gpio_base + 0x10U) & smoke_mask) != 0U);

    const auto cleared = pin.clear();
    ASSERT_TRUE(cleared.has_value());
    if (!cleared) {
        static_cast<void>(pin.release());
        return;
    }
    ASSERT_TRUE((register_at(gpio_base + 0x10U) & smoke_mask) == 0U);

    const auto released = pin.release();
    ASSERT_TRUE(released.has_value());
    if (!released) return;
    ASSERT_TRUE((register_at(gpio_base + 0x2CU) & smoke_mask) == 0U);
    ASSERT_TRUE(((register_at(io_mux_base + 0x04U + smoke_pin.index * 4U)
                  >> 9U) & 0x1U) == 1U);
}

} // namespace mcuhw::board_test::esp32p4_nano
