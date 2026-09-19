#pragma once

#include <array>
#include <cstdint>

#include <mcuhw/mcu/esp32p4/interrupts_dma.hpp>

namespace mcuhw::mcu::esp32p4 {

inline constexpr core::source_ref systimer_source{
    "ESP32-P4 TRM", "pre-release v0.7", "Chapter 16, PDF pages 1507-1531"};
inline constexpr core::source_ref timer_group_source{
    "ESP32-P4 TRM", "pre-release v0.7", "Chapter 17, PDF pages 1532-1560"};
inline constexpr core::source_ref uart_source{
    "ESP32-P4 TRM", "pre-release v0.7", "Chapter 45, PDF pages 2733-2818"};
inline constexpr core::source_ref spi_source{
    "ESP32-P4 TRM", "pre-release v0.7", "Chapter 46, PDF pages 2819-2978"};
inline constexpr core::source_ref i2c_source{
    "ESP32-P4 TRM", "pre-release v0.7", "Chapter 47, PDF pages 2979-3054"};
inline constexpr core::source_ref ledc_source{
    "ESP32-P4 TRM", "pre-release v0.7", "Chapter 58, PDF pages 3397-3422"};
inline constexpr core::source_ref mcpwm_source{
    "ESP32-P4 TRM", "pre-release v0.7", "Chapter 59, PDF pages 3423-3501"};
inline constexpr core::source_ref adc_source{
    "ESP32-P4 TRM / Series Datasheet", "pre-release v0.7",
    "TRM Chapter 65, PDF pages 3646-3693; Datasheet GPIO/ADC tables"};

inline constexpr std::uintptr_t systimer_base = 0x500E'2000U;
inline constexpr std::uintptr_t timer_group0_base = 0x500C'2000U;
inline constexpr std::uintptr_t timer_group1_base = 0x500C'3000U;
inline constexpr std::uintptr_t mcpwm0_base = 0x500C'0000U;
inline constexpr std::uintptr_t mcpwm1_base = 0x500C'1000U;
inline constexpr std::uintptr_t i2c0_base = 0x500C'4000U;
inline constexpr std::uintptr_t i2c1_base = 0x500C'5000U;
inline constexpr std::uintptr_t uart0_base = 0x500C'A000U;
inline constexpr std::uintptr_t uart1_base = 0x500C'B000U;
inline constexpr std::uintptr_t uart2_base = 0x500C'C000U;
inline constexpr std::uintptr_t uart3_base = 0x500C'D000U;
inline constexpr std::uintptr_t uart4_base = 0x500C'E000U;
inline constexpr std::uintptr_t spi2_base = 0x500D'0000U;
inline constexpr std::uintptr_t spi3_base = 0x500D'1000U;
inline constexpr std::uintptr_t ledc_base = 0x500D'3000U;
inline constexpr std::uintptr_t adc_base = 0x500D'E000U;

using systimer_conf = core::register_descriptor<std::uint32_t, 0x00,
    core::access_semantics::read_write, 0xFFFF'FFFFU, false,
    core::reserved_bit_policy::preserve>;
using timg_t0config = core::register_descriptor<std::uint32_t, 0x00,
    core::access_semantics::read_write, 0xFFFF'FFFFU, false,
    core::reserved_bit_policy::preserve>;
using timg_t0lo = core::register_descriptor<std::uint32_t, 0x04,
    core::access_semantics::read_only, 0U, false,
    core::reserved_bit_policy::write_zero>;
using timg_t0hi = core::register_descriptor<std::uint32_t, 0x08,
    core::access_semantics::read_only, 0U, false,
    core::reserved_bit_policy::write_zero>;
using timg_int_clr = core::register_descriptor<std::uint32_t, 0x7C,
    core::access_semantics::write_one_to_clear, 0x1FU, false,
    core::reserved_bit_policy::write_zero>;

using uart_fifo = core::register_descriptor<std::uint32_t, 0x00,
    core::access_semantics::read_write, 0xFFU, false,
    core::reserved_bit_policy::write_zero>;
using uart_int_status = core::register_descriptor<std::uint32_t, 0x08,
    core::access_semantics::read_only, 0U, false,
    core::reserved_bit_policy::write_zero>;
using uart_int_clear = core::register_descriptor<std::uint32_t, 0x10,
    core::access_semantics::write_one_to_clear, 0xFFFF'FFFFU, false,
    core::reserved_bit_policy::write_zero>;

using spi_command = core::register_descriptor<std::uint32_t, 0x00,
    core::access_semantics::read_write, 0x0004'0000U, false,
    core::reserved_bit_policy::write_zero>;
using spi_address = core::register_descriptor<std::uint32_t, 0x04,
    core::access_semantics::read_write, 0xFFFF'FFFFU, false,
    core::reserved_bit_policy::write_zero>;

using i2c_control = core::register_descriptor<std::uint32_t, 0x04,
    core::access_semantics::read_write, 0xFFFF'FFFFU, false,
    core::reserved_bit_policy::preserve>;
using i2c_data = core::register_descriptor<std::uint32_t, 0x1C,
    core::access_semantics::read_write, 0xFFU, false,
    core::reserved_bit_policy::write_zero>;
using i2c_int_clear = core::register_descriptor<std::uint32_t, 0x24,
    core::access_semantics::write_one_to_clear, 0xFFFF'FFFFU, false,
    core::reserved_bit_policy::write_zero>;

using ledc_channel0_conf0 = core::register_descriptor<std::uint32_t, 0x00,
    core::access_semantics::read_write, 0xFFFF'FFFFU, false,
    core::reserved_bit_policy::preserve>;
using ledc_channel0_hpoint = core::register_descriptor<std::uint32_t, 0x04,
    core::access_semantics::read_write, 0x000F'FFFFU, false,
    core::reserved_bit_policy::write_zero>;
using ledc_channel0_duty = core::register_descriptor<std::uint32_t, 0x08,
    core::access_semantics::read_write, 0x01FF'FFFFU, false,
    core::reserved_bit_policy::write_zero>;
using ledc_int_clear = core::register_descriptor<std::uint32_t, 0xCC,
    core::access_semantics::write_one_to_clear, 0x000F'FFFFU, false,
    core::reserved_bit_policy::write_zero>;
using ledc_hpoint = core::field_descriptor<ledc_channel0_hpoint, 0, 20>;

using mcpwm_clock_config = core::register_descriptor<std::uint32_t, 0x00,
    core::access_semantics::read_write, 0x0000'00FFU, false,
    core::reserved_bit_policy::write_zero>;
using mcpwm_timer0_config = core::register_descriptor<std::uint32_t, 0x04,
    core::access_semantics::read_write, 0xFFFF'FFFFU, false,
    core::reserved_bit_policy::preserve>;
using mcpwm_dead_time0 = core::register_descriptor<std::uint32_t, 0x58,
    core::access_semantics::read_write, 0xFFFF'FFFFU, false,
    core::reserved_bit_policy::preserve>;
using mcpwm_int_clear = core::register_descriptor<std::uint32_t, 0x11C,
    core::access_semantics::write_one_to_clear, 0xFFFF'FFFFU, false,
    core::reserved_bit_policy::write_zero>;

using adc_control = core::register_descriptor<std::uint32_t, 0x00,
    core::access_semantics::read_write, 0xFFFF'FFFFU, false,
    core::reserved_bit_policy::preserve>;
using adc_int_status = core::register_descriptor<std::uint32_t, 0x58,
    core::access_semantics::read_only, 0U, false,
    core::reserved_bit_policy::write_zero>;
using adc_int_clear = core::register_descriptor<std::uint32_t, 0x5C,
    core::access_semantics::write_one_to_clear, 0xFFFF'FFFFU, false,
    core::reserved_bit_policy::write_zero>;

inline constexpr std::array timers{
    timer_instance{timer_kind::system, 0, 2, 52, 0, false, false, systimer_base, systimer_source},
    timer_instance{timer_kind::timer_group, 0, 2, 54, 0, false, false, timer_group0_base, timer_group_source},
    timer_instance{timer_kind::timer_group, 1, 2, 54, 0, false, false, timer_group1_base, timer_group_source},
    timer_instance{timer_kind::led_pwm, 0, 4, 20, 8, false, false, ledc_base, ledc_source},
    timer_instance{timer_kind::motor_pwm, 0, 3, 16, 6, true, true, mcpwm0_base, mcpwm_source},
    timer_instance{timer_kind::motor_pwm, 1, 3, 16, 6, true, true, mcpwm1_base, mcpwm_source},
};

inline constexpr std::array timer_registers{
    register_record{"SYSTIMER_CONF_REG", systimer_base + 0x00U, 4, core::access_semantics::read_write, 0xFFFF'FFFFU, 0U, 0U, core::reserved_bit_policy::preserve, systimer_source},
    register_record{"TIMG_T0CONFIG_REG", timer_group0_base + 0x00U, 4, core::access_semantics::read_write, 0xFFFF'FFFFU, 0U, 0U, core::reserved_bit_policy::preserve, timer_group_source},
    register_record{"TIMG_T0LO_REG", timer_group0_base + 0x04U, 4, core::access_semantics::read_only, 0U, 0U, 0U, core::reserved_bit_policy::write_zero, timer_group_source},
    register_record{"TIMG_T0HI_REG", timer_group0_base + 0x08U, 4, core::access_semantics::read_only, 0U, 0U, 0U, core::reserved_bit_policy::write_zero, timer_group_source},
    register_record{"TIMG_T0UPDATE_REG", timer_group0_base + 0x0CU, 4, core::access_semantics::command, 0xFFFF'FFFFU, 0U, 0U, core::reserved_bit_policy::write_zero, timer_group_source},
    register_record{"TIMG_INT_CLR_TIMERS_REG", timer_group0_base + 0x7CU, 4, core::access_semantics::write_one_to_clear, 0x1FU, 0U, 0U, core::reserved_bit_policy::write_zero, timer_group_source},
    register_record{"LEDC_CH0_CONF0_REG", ledc_base + 0x00U, 4, core::access_semantics::read_write, 0xFFFF'FFFFU, 0U, 0U, core::reserved_bit_policy::preserve, ledc_source},
    register_record{"LEDC_CH0_HPOINT_REG", ledc_base + 0x04U, 4, core::access_semantics::read_write, 0x000F'FFFFU, 0U, 0xFFFF'FFFFU, core::reserved_bit_policy::write_zero, ledc_source},
    register_record{"LEDC_CH0_DUTY_REG", ledc_base + 0x08U, 4, core::access_semantics::read_write, 0x01FF'FFFFU, 0U, 0xFFFF'FFFFU, core::reserved_bit_policy::write_zero, ledc_source},
    register_record{"LEDC_INT_CLR_REG", ledc_base + 0xCCU, 4, core::access_semantics::write_one_to_clear, 0x000F'FFFFU, 0U, 0U, core::reserved_bit_policy::write_zero, ledc_source},
    register_record{"MCPWM_CLK_CFG_REG", mcpwm0_base + 0x00U, 4, core::access_semantics::read_write, 0xFFU, 0U, 0U, core::reserved_bit_policy::write_zero, mcpwm_source},
    register_record{"MCPWM_TIMER0_CFG0_REG", mcpwm0_base + 0x04U, 4, core::access_semantics::read_write, 0xFFFF'FFFFU, 0U, 0U, core::reserved_bit_policy::preserve, mcpwm_source},
    register_record{"MCPWM_DT0_CFG_REG", mcpwm0_base + 0x58U, 4, core::access_semantics::read_write, 0xFFFF'FFFFU, 0U, 0U, core::reserved_bit_policy::preserve, mcpwm_source},
    register_record{"MCPWM_INT_CLR_REG", mcpwm0_base + 0x11CU, 4, core::access_semantics::write_one_to_clear, 0xFFFF'FFFFU, 0U, 0U, core::reserved_bit_policy::write_zero, mcpwm_source},
};

inline constexpr std::array serial_registers{
    register_record{"UART_FIFO_REG", uart0_base + 0x00U, 4, core::access_semantics::read_write, 0xFFU, 0U, 0U, core::reserved_bit_policy::write_zero, uart_source},
    register_record{"UART_INT_ST_REG", uart0_base + 0x08U, 4, core::access_semantics::read_only, 0U, 0U, 0U, core::reserved_bit_policy::write_zero, uart_source},
    register_record{"UART_INT_CLR_REG", uart0_base + 0x10U, 4, core::access_semantics::write_one_to_clear, 0xFFFF'FFFFU, 0U, 0U, core::reserved_bit_policy::write_zero, uart_source},
    register_record{"SPI_CMD_REG", spi2_base + 0x00U, 4, core::access_semantics::read_write, 0x0004'0000U, 0U, 0U, core::reserved_bit_policy::write_zero, spi_source},
    register_record{"SPI_ADDR_REG", spi2_base + 0x04U, 4, core::access_semantics::read_write, 0xFFFF'FFFFU, 0U, 0U, core::reserved_bit_policy::write_zero, spi_source},
    register_record{"I2C_CTR_REG", i2c0_base + 0x04U, 4, core::access_semantics::read_write, 0xFFFF'FFFFU, 0U, 0U, core::reserved_bit_policy::preserve, i2c_source},
    register_record{"I2C_DATA_REG", i2c0_base + 0x1CU, 4, core::access_semantics::read_write, 0xFFU, 0U, 0U, core::reserved_bit_policy::write_zero, i2c_source},
    register_record{"I2C_INT_CLR_REG", i2c0_base + 0x24U, 4, core::access_semantics::write_one_to_clear, 0xFFFF'FFFFU, 0U, 0U, core::reserved_bit_policy::write_zero, i2c_source},
};

inline constexpr std::array adc_registers{
    register_record{"ADC_CTRL_REG", adc_base + 0x00U, 4, core::access_semantics::read_write, 0xFFFF'FFFFU, 0U, 0U, core::reserved_bit_policy::preserve, adc_source},
    register_record{"ADC_INT_ST_REG", adc_base + 0x58U, 4, core::access_semantics::read_only, 0U, 0U, 0U, core::reserved_bit_policy::write_zero, adc_source},
    register_record{"ADC_INT_CLR_REG", adc_base + 0x5CU, 4, core::access_semantics::write_one_to_clear, 0xFFFF'FFFFU, 0U, 0U, core::reserved_bit_policy::write_zero, adc_source},
    register_record{"ADC_DMA_CONF_REG", adc_base + 0x60U, 4, core::access_semantics::read_write, 0xFFFF'FFFFU, 0U, 0U, core::reserved_bit_policy::preserve, adc_source},
};

} // namespace mcuhw::mcu::esp32p4
