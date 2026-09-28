// @req-file REQ-0021 REQ-0031 REQ-0033 REQ-0036

#include <cstdint>

#include <mcuhw/backend/stm32h753/gpio.hpp>
#include <mcuhw/board/nucleo_h753zi.hpp>
#include <mcuhw/core/direct_mmio.hpp>
#include <mcuhw/driver/gpio.hpp>
#include <mcuhw/mcu/stm32h753/pins.hpp>

extern "C" volatile std::uint32_t mcuhw_nucleo_clock_status;

extern "C" {
volatile std::uint32_t mcuhw_nucleo_smoke_status;
volatile std::uint32_t mcuhw_nucleo_gpio_moder;
volatile std::uint32_t mcuhw_nucleo_gpio_odr;
}

int main() {
    mcuhw::core::direct_mmio policy;
    mcuhw::backend::stm32h753::gpio_backend gpio{policy};
    mcuhw::driver::gpio_pin led{
        gpio, mcuhw::board::nucleo_h753zi::user_led_yellow};
    const mcuhw::driver::gpio_configuration configuration{
        .direction = mcuhw::driver::gpio_direction::output,
        .initial_level = mcuhw::driver::gpio_level::high,
        .output_type = mcuhw::driver::gpio_output_type::push_pull,
        .pull = mcuhw::driver::gpio_pull::none,
        .drive = mcuhw::driver::gpio_drive::low,
    };
    const auto initialized = led.initialize(configuration);
    const auto gpioe = mcuhw::mcu::stm32h753::gpio_base(4U);
    mcuhw_nucleo_gpio_moder =
        *reinterpret_cast<volatile const std::uint32_t*>(gpioe + 0x00U);
    mcuhw_nucleo_gpio_odr =
        *reinterpret_cast<volatile const std::uint32_t*>(gpioe + 0x14U);
    mcuhw_nucleo_smoke_status = mcuhw_nucleo_clock_status != 0U
        ? 0xBAD0'0001U
        : (initialized.has_value() ? 0x4857'0001U : 0xBAD0'0002U);
    for (;;) {
        __asm__ volatile("wfi");
    }
}
