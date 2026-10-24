#pragma once

#include <array>
#include <cstdint>

#include <mcubsp/mcu/esp32p4/interrupts_dma.hpp>

namespace mcubsp::mcu::esp32p4 {

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
inline constexpr core::source_ref i2s_source{
    "ESP32-P4 TRM", "pre-release v0.7", "Chapter 49, PDF pages 3070-3115"};
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
inline constexpr std::uintptr_t i2s0_base = 0x500C'6000U;
inline constexpr std::uintptr_t i2s1_base = 0x500C'7000U;
inline constexpr std::uintptr_t i2s2_base = 0x500C'8000U;
inline constexpr std::uintptr_t uart0_base = 0x500C'A000U;
inline constexpr std::uintptr_t uart1_base = 0x500C'B000U;
inline constexpr std::uintptr_t uart2_base = 0x500C'C000U;
inline constexpr std::uintptr_t uart3_base = 0x500C'D000U;
inline constexpr std::uintptr_t uart4_base = 0x500C'E000U;
inline constexpr std::uintptr_t spi2_base = 0x500D'0000U;
inline constexpr std::uintptr_t spi3_base = 0x500D'1000U;
inline constexpr std::uintptr_t ledc_base = 0x500D'3000U;
inline constexpr std::uintptr_t adc_base = 0x500D'E000U;
using i2s_rx_configuration = core::register_descriptor<std::uint32_t, 0x20,
    core::access_semantics::read_write, 0x07FF'BFFFU, false,
    core::reserved_bit_policy::preserve>;
using i2s_tx_configuration = core::register_descriptor<std::uint32_t, 0x24,
    core::access_semantics::read_write, 0x7FFF'BFFFU, false,
    core::reserved_bit_policy::preserve>;
using i2s_rx_eof_number = core::register_descriptor<std::uint32_t, 0x64,
    core::access_semantics::read_write, 0x0FFFU, false,
    core::reserved_bit_policy::write_zero>;

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
using uart_int_enable = core::register_descriptor<std::uint32_t, 0x0C,
    core::access_semantics::read_write, 0x000F'FFFFU, false,
    core::reserved_bit_policy::preserve>;
using uart_int_clear = core::register_descriptor<std::uint32_t, 0x10,
    core::access_semantics::write_one_to_clear, 0x000F'FFFFU, false,
    core::reserved_bit_policy::write_zero>;
using uart_clock_divider = core::register_descriptor<std::uint32_t, 0x14,
    core::access_semantics::read_write, 0x00F0'0FFFU, false,
    core::reserved_bit_policy::write_zero>;
using uart_status = core::register_descriptor<std::uint32_t, 0x1C,
    core::access_semantics::read_only, 0U, false,
    core::reserved_bit_policy::write_zero>;
using uart_configuration0 = core::register_descriptor<std::uint32_t, 0x20,
    core::access_semantics::read_write, 0x00FF'FFFFU, false,
    core::reserved_bit_policy::preserve>;
using uart_configuration1 = core::register_descriptor<std::uint32_t, 0x24,
    core::access_semantics::read_write, 0x003F'FFFFU, false,
    core::reserved_bit_policy::preserve>;
using uart_memory_configuration = core::register_descriptor<std::uint32_t, 0x60,
    core::access_semantics::read_write, 0x0600'0000U, false,
    core::reserved_bit_policy::preserve>;
using uart_clock_configuration = core::register_descriptor<std::uint32_t, 0x88,
    core::access_semantics::read_write, 0x0F00'0000U, false,
    core::reserved_bit_policy::preserve>;
using uart_register_update = core::register_descriptor<std::uint32_t, 0x98,
    core::access_semantics::read_write, 0x1U, false,
    core::reserved_bit_policy::write_zero>;

using spi_command = core::register_descriptor<std::uint32_t, 0x00,
    core::access_semantics::read_write, 0x0180'0000U, false,
    core::reserved_bit_policy::write_zero>;
using spi_address = core::register_descriptor<std::uint32_t, 0x04,
    core::access_semantics::read_write, 0xFFFF'FFFFU, false,
    core::reserved_bit_policy::write_zero>;

using i2c_control = core::register_descriptor<std::uint32_t, 0x04,
    core::access_semantics::read_write, 0xFFFF'FFFFU, false,
    core::reserved_bit_policy::preserve>;
using i2c_status = core::register_descriptor<std::uint32_t, 0x08,
    core::access_semantics::read_only, 0U, false,
    core::reserved_bit_policy::write_zero>;
using i2c_timeout = core::register_descriptor<std::uint32_t, 0x0C,
    core::access_semantics::read_write, 0x3FU, false,
    core::reserved_bit_policy::preserve>;
using i2c_fifo_configuration = core::register_descriptor<std::uint32_t, 0x18,
    core::access_semantics::read_write, 0x7FFFU, false,
    core::reserved_bit_policy::preserve>;
using i2c_data = core::register_descriptor<std::uint32_t, 0x1C,
    core::access_semantics::read_write, 0xFFU, false,
    core::reserved_bit_policy::write_zero>;
using i2c_int_status = core::register_descriptor<std::uint32_t, 0x2C,
    core::access_semantics::read_only, 0U, false,
    core::reserved_bit_policy::write_zero>;
using i2c_int_enable = core::register_descriptor<std::uint32_t, 0x28,
    core::access_semantics::read_write, 0x0007'FFFFU, false,
    core::reserved_bit_policy::preserve>;
using i2c_int_clear = core::register_descriptor<std::uint32_t, 0x24,
    core::access_semantics::write_one_to_clear, 0xFFFF'FFFFU, false,
    core::reserved_bit_policy::write_zero>;
using i2c_command0 = core::register_descriptor<std::uint32_t, 0x58,
    core::access_semantics::read_write, 0x8000'3FFFU, false,
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
    register_record{"UART_INT_ENA_REG", uart0_base + 0x0CU, 4, core::access_semantics::read_write, 0x000F'FFFFU, 0U, 0U, core::reserved_bit_policy::preserve, uart_source},
    register_record{"UART_INT_CLR_REG", uart0_base + 0x10U, 4, core::access_semantics::write_one_to_clear, 0x000F'FFFFU, 0U, 0U, core::reserved_bit_policy::write_zero, uart_source},
    register_record{"UART_CLKDIV_SYNC_REG", uart0_base + 0x14U, 4, core::access_semantics::read_write, 0x00F0'0FFFU, 0U, 0x0000'02B6U, core::reserved_bit_policy::write_zero, uart_source},
    register_record{"UART_STATUS_REG", uart0_base + 0x1CU, 4, core::access_semantics::read_only, 0U, 0U, 0U, core::reserved_bit_policy::write_zero, uart_source},
    register_record{"UART_CONF0_SYNC_REG", uart0_base + 0x20U, 4, core::access_semantics::read_write, 0x00FF'FFFFU, 0U, 0x0000'003CU, core::reserved_bit_policy::preserve, uart_source},
    register_record{"UART_CONF1_REG", uart0_base + 0x24U, 4, core::access_semantics::read_write, 0x003F'FFFFU, 0U, 0x0000'6060U, core::reserved_bit_policy::preserve, uart_source},
    register_record{"UART_MEM_CONF_REG", uart0_base + 0x60U, 4, core::access_semantics::read_write, 0x0600'0000U, 0U, 0U, core::reserved_bit_policy::preserve, uart_source},
    register_record{"UART_CLK_CONF_REG", uart0_base + 0x88U, 4, core::access_semantics::read_write, 0x0F00'0000U, 0U, 0x0300'0000U, core::reserved_bit_policy::preserve, uart_source},
    register_record{"UART_REG_UPDATE_REG", uart0_base + 0x98U, 4, core::access_semantics::read_write, 0x1U, 0U, 0U, core::reserved_bit_policy::write_zero, uart_source},
    register_record{"SPI_CMD_REG", spi2_base + 0x00U, 4, core::access_semantics::read_write, 0x0180'0000U, 0U, 0U, core::reserved_bit_policy::write_zero, spi_source},
    register_record{"SPI_ADDR_REG", spi2_base + 0x04U, 4, core::access_semantics::read_write, 0xFFFF'FFFFU, 0U, 0U, core::reserved_bit_policy::write_zero, spi_source},
    register_record{"I2C_CTR_REG", i2c0_base + 0x04U, 4, core::access_semantics::read_write, 0xFFFF'FFFFU, 0U, 0U, core::reserved_bit_policy::preserve, i2c_source},
    register_record{"I2C_SR_REG", i2c0_base + 0x08U, 4, core::access_semantics::read_only, 0U, 0U, 0U, core::reserved_bit_policy::write_zero, i2c_source},
    register_record{"I2C_TO_REG", i2c0_base + 0x0CU, 4, core::access_semantics::read_write, 0x3FU, 0U, 0U, core::reserved_bit_policy::preserve, i2c_source},
    register_record{"I2C_FIFO_CONF_REG", i2c0_base + 0x18U, 4, core::access_semantics::read_write, 0x7FFFU, 0U, 0U, core::reserved_bit_policy::preserve, i2c_source},
    register_record{"I2C_DATA_REG", i2c0_base + 0x1CU, 4, core::access_semantics::read_write, 0xFFU, 0U, 0U, core::reserved_bit_policy::write_zero, i2c_source},
    register_record{"I2C_INT_CLR_REG", i2c0_base + 0x24U, 4, core::access_semantics::write_one_to_clear, 0xFFFF'FFFFU, 0U, 0U, core::reserved_bit_policy::write_zero, i2c_source},
    register_record{"I2C_INT_ENA_REG", i2c0_base + 0x28U, 4, core::access_semantics::read_write, 0x0007'FFFFU, 0U, 0U, core::reserved_bit_policy::preserve, i2c_source},
    register_record{"I2C_INT_STATUS_REG", i2c0_base + 0x2CU, 4, core::access_semantics::read_only, 0U, 0U, 0U, core::reserved_bit_policy::write_zero, i2c_source},
    register_record{"I2C_COMD0_REG", i2c0_base + 0x58U, 4, core::access_semantics::read_write, 0x8000'3FFFU, 0U, 0U, core::reserved_bit_policy::write_zero, i2c_source},
    register_record{"I2S_RX_CONF_REG", i2s0_base + 0x20U, 4, core::access_semantics::read_write, 0x07FF'BFFFU, 0U, 0U, core::reserved_bit_policy::preserve, i2s_source},
    register_record{"I2S_TX_CONF_REG", i2s0_base + 0x24U, 4, core::access_semantics::read_write, 0x7FFF'BFFFU, 0U, 0U, core::reserved_bit_policy::preserve, i2s_source},
    register_record{"I2S_RX_EOF_NUM_REG", i2s0_base + 0x64U, 4, core::access_semantics::read_write, 0x0FFFU, 0U, 0U, core::reserved_bit_policy::write_zero, i2s_source},
};

inline constexpr std::array adc_registers{
    register_record{"ADC_CTRL_REG", adc_base + 0x00U, 4, core::access_semantics::read_write, 0xFFFF'FFFFU, 0U, 0U, core::reserved_bit_policy::preserve, adc_source},
    register_record{"ADC_INT_ST_REG", adc_base + 0x58U, 4, core::access_semantics::read_only, 0U, 0U, 0U, core::reserved_bit_policy::write_zero, adc_source},
    register_record{"ADC_INT_CLR_REG", adc_base + 0x5CU, 4, core::access_semantics::write_one_to_clear, 0xFFFF'FFFFU, 0U, 0U, core::reserved_bit_policy::write_zero, adc_source},
    register_record{"ADC_DMA_CONF_REG", adc_base + 0x60U, 4, core::access_semantics::read_write, 0xFFFF'FFFFU, 0U, 0U, core::reserved_bit_policy::preserve, adc_source},
};

} // namespace mcubsp::mcu::esp32p4
