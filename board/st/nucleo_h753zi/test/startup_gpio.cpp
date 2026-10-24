// @req-file REQ-0021 REQ-0031 REQ-0033 REQ-0036

#include <cstdint>

#include <mcubsp/backend/stm32h753/gpio.hpp>
#include <mcubsp/board/nucleo_h753zi.hpp>
#include <mcubsp/core/direct_mmio.hpp>
#include <mcubsp/driver/gpio.hpp>
#include <mcubsp/mcu/stm32h753/pins.hpp>

extern "C" volatile std::uint32_t mcubsp_nucleo_clock_status;

extern "C" {
volatile std::uint32_t mcubsp_nucleo_smoke_status;
volatile std::uint32_t mcubsp_nucleo_gpio_moder;
volatile std::uint32_t mcubsp_nucleo_gpio_odr;
}

int main() {
    mcubsp::core::direct_mmio policy;
    mcubsp::backend::stm32h753::gpio_backend gpio{policy};
    mcubsp::driver::gpio_pin led{
        gpio, mcubsp::board::nucleo_h753zi::user_led_yellow};
    const mcubsp::driver::gpio_configuration configuration{
        .direction = mcubsp::driver::gpio_direction::output,
        .initial_level = mcubsp::driver::gpio_level::high,
        .output_type = mcubsp::driver::gpio_output_type::push_pull,
        .pull = mcubsp::driver::gpio_pull::none,
        .drive = mcubsp::driver::gpio_drive::low,
    };
    const auto initialized = led.initialize(configuration);
    const auto gpioe = mcubsp::mcu::stm32h753::gpio_base(4U);
    mcubsp_nucleo_gpio_moder =
        *reinterpret_cast<volatile const std::uint32_t*>(gpioe + 0x00U);
    mcubsp_nucleo_gpio_odr =
        *reinterpret_cast<volatile const std::uint32_t*>(gpioe + 0x14U);
    mcubsp_nucleo_smoke_status = mcubsp_nucleo_clock_status != 0U
        ? 0xBAD0'0001U
        : (initialized.has_value() ? 0x4857'0001U : 0xBAD0'0002U);
    for (;;) {
        __asm__ volatile("wfi");
    }
}
