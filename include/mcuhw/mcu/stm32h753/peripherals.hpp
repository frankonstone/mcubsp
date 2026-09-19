#pragma once

#include <array>
#include <cstdint>

#include <mcuhw/mcu/stm32h753/interrupts_dma.hpp>
#include <mcuhw/mcu/stm32h753/pins.hpp>

namespace mcuhw::mcu::stm32h753 {

inline constexpr core::source_ref timer_source{
    "RM0433", "Rev.8", "Chapters 38, 39 and 41, PDF pages 1831-2134"};

constexpr std::array<timer_output, 7> timer_outputs(
    timer_output a, timer_output b = timer_output::ch1,
    timer_output c = timer_output::ch1, timer_output d = timer_output::ch1,
    timer_output e = timer_output::ch1, timer_output f = timer_output::ch1,
    timer_output g = timer_output::ch1) noexcept {
    return {a, b, c, d, e, f, g};
}

inline constexpr std::array timer_instances{
    timer_instance{1, 16, 0x4001'0000U, timer_outputs(timer_output::ch1, timer_output::ch1n, timer_output::ch2, timer_output::ch2n, timer_output::ch3, timer_output::ch3n, timer_output::ch4), 7, true, true, timer_source},
    timer_instance{2, 32, 0x4000'0000U, timer_outputs(timer_output::ch1, timer_output::ch2, timer_output::ch3, timer_output::ch4), 4, true, false, timer_source},
    timer_instance{3, 16, 0x4000'0400U, timer_outputs(timer_output::ch1, timer_output::ch2, timer_output::ch3, timer_output::ch4), 4, true, false, timer_source},
    timer_instance{4, 16, 0x4000'0800U, timer_outputs(timer_output::ch1, timer_output::ch2, timer_output::ch3, timer_output::ch4), 4, true, false, timer_source},
    timer_instance{5, 32, 0x4000'0C00U, timer_outputs(timer_output::ch1, timer_output::ch2, timer_output::ch3, timer_output::ch4), 4, true, false, timer_source},
    timer_instance{8, 16, 0x4001'0400U, timer_outputs(timer_output::ch1, timer_output::ch1n, timer_output::ch2, timer_output::ch2n, timer_output::ch3, timer_output::ch3n, timer_output::ch4), 7, true, true, timer_source},
    timer_instance{12, 16, 0x4000'1800U, timer_outputs(timer_output::ch1, timer_output::ch2), 2, false, false, timer_source},
    timer_instance{13, 16, 0x4000'1C00U, timer_outputs(timer_output::ch1), 1, false, false, timer_source},
    timer_instance{14, 16, 0x4000'2000U, timer_outputs(timer_output::ch1), 1, false, false, timer_source},
    timer_instance{15, 16, 0x4001'4000U, timer_outputs(timer_output::ch1, timer_output::ch1n, timer_output::ch2), 3, false, true, timer_source},
    timer_instance{16, 16, 0x4001'4400U, timer_outputs(timer_output::ch1, timer_output::ch1n), 2, false, true, timer_source},
    timer_instance{17, 16, 0x4001'4800U, timer_outputs(timer_output::ch1, timer_output::ch1n), 2, false, true, timer_source},
};

using tim_cr1 = core::register_descriptor<std::uint32_t, 0x00,
    core::access_semantics::read_write, 0x0000'03FFU, true,
    core::reserved_bit_policy::preserve>;
using tim_sr = core::register_descriptor<std::uint32_t, 0x10,
    core::access_semantics::write_zero_to_clear, 0x0001'FFFFU, false,
    core::reserved_bit_policy::write_zero>;
using tim_egr = core::register_descriptor<std::uint32_t, 0x14,
    core::access_semantics::command, 0x0000'00FFU, false,
    core::reserved_bit_policy::write_zero>;
using tim_cnt = core::register_descriptor<std::uint32_t, 0x24,
    core::access_semantics::read_write, 0xFFFF'FFFFU, false,
    core::reserved_bit_policy::write_zero>;
using tim_center_aligned_mode = core::field_descriptor<tim_cr1, 5, 2>;
using tim_update_flag = core::field_descriptor<tim_sr, 0, 1,
    core::access_semantics::write_zero_to_clear>;

inline constexpr std::array timer_registers{
    register_record{"TIMx_CR1", 0x4001'0000U, 4, core::access_semantics::read_write, 0x0000'03FFU, 0U, 0xFFFF'FFFFU, core::reserved_bit_policy::preserve, timer_source},
    register_record{"TIMx_CR2", 0x4001'0004U, 4, core::access_semantics::read_write, 0xFFFF'FFFFU, 0U, 0U, core::reserved_bit_policy::preserve, timer_source},
    register_record{"TIMx_SMCR", 0x4001'0008U, 4, core::access_semantics::read_write, 0xFFFF'FFFFU, 0U, 0U, core::reserved_bit_policy::preserve, timer_source},
    register_record{"TIMx_DIER", 0x4001'000CU, 4, core::access_semantics::read_write, 0x0000'7FFFU, 0U, 0xFFFF'FFFFU, core::reserved_bit_policy::write_zero, timer_source},
    register_record{"TIMx_SR", 0x4001'0010U, 4, core::access_semantics::write_zero_to_clear, 0x0001'FFFFU, 0U, 0xFFFF'FFFFU, core::reserved_bit_policy::write_zero, timer_source},
    register_record{"TIMx_EGR", 0x4001'0014U, 4, core::access_semantics::command, 0x0000'00FFU, 0U, 0xFFFF'FFFFU, core::reserved_bit_policy::write_zero, timer_source},
    register_record{"TIMx_CCMR1", 0x4001'0018U, 4, core::access_semantics::read_write, 0xFFFF'FFFFU, 0U, 0U, core::reserved_bit_policy::preserve, timer_source},
    register_record{"TIMx_CCMR2", 0x4001'001CU, 4, core::access_semantics::read_write, 0xFFFF'FFFFU, 0U, 0U, core::reserved_bit_policy::preserve, timer_source},
    register_record{"TIMx_CCER", 0x4001'0020U, 4, core::access_semantics::read_write, 0x001F'FFFFU, 0U, 0U, core::reserved_bit_policy::preserve, timer_source},
    register_record{"TIMx_CNT", 0x4001'0024U, 4, core::access_semantics::read_write, 0xFFFF'FFFFU, 0U, 0U, core::reserved_bit_policy::write_zero, timer_source},
    register_record{"TIMx_PSC", 0x4001'0028U, 4, core::access_semantics::read_write, 0x0000'FFFFU, 0U, 0U, core::reserved_bit_policy::write_zero, timer_source},
    register_record{"TIMx_ARR", 0x4001'002CU, 4, core::access_semantics::read_write, 0xFFFF'FFFFU, 0xFFFFU, 0U, core::reserved_bit_policy::write_zero, timer_source},
    register_record{"TIMx_CCR1", 0x4001'0034U, 4, core::access_semantics::read_write, 0xFFFF'FFFFU, 0U, 0U, core::reserved_bit_policy::write_zero, timer_source},
    register_record{"TIMx_CCR2", 0x4001'0038U, 4, core::access_semantics::read_write, 0xFFFF'FFFFU, 0U, 0U, core::reserved_bit_policy::write_zero, timer_source},
    register_record{"TIMx_CCR3", 0x4001'003CU, 4, core::access_semantics::read_write, 0xFFFF'FFFFU, 0U, 0U, core::reserved_bit_policy::write_zero, timer_source},
    register_record{"TIMx_CCR4", 0x4001'0040U, 4, core::access_semantics::read_write, 0xFFFF'FFFFU, 0U, 0U, core::reserved_bit_policy::write_zero, timer_source},
    register_record{"TIM1_BDTR", 0x4001'0044U, 4, core::access_semantics::read_write, 0xFFFF'FFFFU, 0U, 0U, core::reserved_bit_policy::preserve, timer_source},
};

inline constexpr core::source_ref i2c_source{
    "RM0433", "Rev.8", "Chapter 47, PDF pages 2269-2355"};
using i2c_cr1 = core::register_descriptor<std::uint32_t, 0x00,
    core::access_semantics::read_write, 0xFFFF'FFFFU, false,
    core::reserved_bit_policy::preserve>;
using i2c_isr = core::register_descriptor<std::uint32_t, 0x18,
    core::access_semantics::read_only, 0U, false,
    core::reserved_bit_policy::write_zero>;
using i2c_icr = core::register_descriptor<std::uint32_t, 0x1C,
    core::access_semantics::write_one_to_clear, 0x0000'3F38U, false,
    core::reserved_bit_policy::write_zero>;
using i2c_enable = core::field_descriptor<i2c_cr1, 0, 1>;

inline constexpr std::array i2c_registers{
    register_record{"I2Cx_CR1", 0x4000'5400U, 4, core::access_semantics::read_write, 0xFFFF'FFFFU, 0U, 0U, core::reserved_bit_policy::preserve, i2c_source},
    register_record{"I2Cx_CR2", 0x4000'5404U, 4, core::access_semantics::read_write, 0x07FF'FFFFU, 0U, 0U, core::reserved_bit_policy::preserve, i2c_source},
    register_record{"I2Cx_OAR1", 0x4000'5408U, 4, core::access_semantics::read_write, 0x0000'83FFU, 0U, 0U, core::reserved_bit_policy::write_zero, i2c_source},
    register_record{"I2Cx_OAR2", 0x4000'540CU, 4, core::access_semantics::read_write, 0x0000'81FFU, 0U, 0U, core::reserved_bit_policy::write_zero, i2c_source},
    register_record{"I2Cx_TIMINGR", 0x4000'5410U, 4, core::access_semantics::read_write, 0xF0FF'FFFFU, 0U, 0U, core::reserved_bit_policy::write_zero, i2c_source},
    register_record{"I2Cx_TIMEOUTR", 0x4000'5414U, 4, core::access_semantics::read_write, 0x8FFF'8FFFU, 0U, 0U, core::reserved_bit_policy::write_zero, i2c_source},
    register_record{"I2Cx_ISR", 0x4000'5418U, 4, core::access_semantics::read_only, 0U, 1U, 0U, core::reserved_bit_policy::write_zero, i2c_source},
    register_record{"I2Cx_ICR", 0x4000'541CU, 4, core::access_semantics::write_one_to_clear, 0x0000'3F38U, 0U, 0xFFFF'FFFFU, core::reserved_bit_policy::write_zero, i2c_source},
    register_record{"I2Cx_PECR", 0x4000'5420U, 4, core::access_semantics::read_only, 0U, 0U, 0U, core::reserved_bit_policy::write_zero, i2c_source},
    register_record{"I2Cx_RXDR", 0x4000'5424U, 4, core::access_semantics::read_only, 0U, 0U, 0U, core::reserved_bit_policy::write_zero, i2c_source},
    register_record{"I2Cx_TXDR", 0x4000'5428U, 4, core::access_semantics::write_only, 0xFFU, 0U, 0xFFFF'FFFFU, core::reserved_bit_policy::write_zero, i2c_source},
};

inline constexpr core::source_ref usart_source{
    "RM0433", "Rev.8", "Chapters 48-49, PDF pages 2356-2470"};
using usart_cr1 = core::register_descriptor<std::uint32_t, 0x00,
    core::access_semantics::read_write, 0xFFFF'FFFFU, false,
    core::reserved_bit_policy::preserve>;
using usart_isr = core::register_descriptor<std::uint32_t, 0x1C,
    core::access_semantics::read_only, 0U, false,
    core::reserved_bit_policy::write_zero>;
using usart_icr = core::register_descriptor<std::uint32_t, 0x20,
    core::access_semantics::write_one_to_clear, 0x0012'1BDFU, false,
    core::reserved_bit_policy::write_zero>;
using usart_enable = core::field_descriptor<usart_cr1, 0, 1>;

inline constexpr std::array usart_registers{
    register_record{"USARTx_CR1", 0x4001'1000U, 4, core::access_semantics::read_write, 0xFFFF'FFFFU, 0U, 0U, core::reserved_bit_policy::preserve, usart_source},
    register_record{"USARTx_CR2", 0x4001'1004U, 4, core::access_semantics::read_write, 0xFFFF'FFFFU, 0U, 0U, core::reserved_bit_policy::preserve, usart_source},
    register_record{"USARTx_CR3", 0x4001'1008U, 4, core::access_semantics::read_write, 0xFFFF'FFFFU, 0U, 0U, core::reserved_bit_policy::preserve, usart_source},
    register_record{"USARTx_BRR", 0x4001'100CU, 4, core::access_semantics::read_write, 0x0000'FFFFU, 0U, 0U, core::reserved_bit_policy::write_zero, usart_source},
    register_record{"USARTx_GTPR", 0x4001'1010U, 4, core::access_semantics::read_write, 0xFFFFU, 0U, 0U, core::reserved_bit_policy::write_zero, usart_source},
    register_record{"USARTx_RTOR", 0x4001'1014U, 4, core::access_semantics::read_write, 0xFFFF'FFFFU, 0U, 0U, core::reserved_bit_policy::write_zero, usart_source},
    register_record{"USARTx_RQR", 0x4001'1018U, 4, core::access_semantics::command, 0x1FU, 0U, 0xFFFF'FFFFU, core::reserved_bit_policy::write_zero, usart_source},
    register_record{"USARTx_ISR", 0x4001'101CU, 4, core::access_semantics::read_only, 0U, 0x0200'00C0U, 0U, core::reserved_bit_policy::write_zero, usart_source},
    register_record{"USARTx_ICR", 0x4001'1020U, 4, core::access_semantics::write_one_to_clear, 0x0012'1BDFU, 0U, 0xFFFF'FFFFU, core::reserved_bit_policy::write_zero, usart_source},
    register_record{"USARTx_RDR", 0x4001'1024U, 4, core::access_semantics::read_only, 0U, 0U, 0U, core::reserved_bit_policy::write_zero, usart_source},
    register_record{"USARTx_TDR", 0x4001'1028U, 4, core::access_semantics::write_only, 0x1FFU, 0U, 0xFFFF'FFFFU, core::reserved_bit_policy::write_zero, usart_source},
    register_record{"USARTx_PRESC", 0x4001'102CU, 4, core::access_semantics::read_write, 0xFU, 0U, 0U, core::reserved_bit_policy::write_zero, usart_source},
};

inline constexpr core::source_ref spi_source{
    "RM0433", "Rev.8", "Chapter 50, PDF pages 2471-2580"};
using spi_cr1 = core::register_descriptor<std::uint32_t, 0x00,
    core::access_semantics::read_write, 0x0000'1FFFU, false,
    core::reserved_bit_policy::preserve>;
using spi_sr = core::register_descriptor<std::uint32_t, 0x14,
    core::access_semantics::read_only, 0U, false,
    core::reserved_bit_policy::write_zero>;
using spi_ifcr = core::register_descriptor<std::uint32_t, 0x18,
    core::access_semantics::write_one_to_clear, 0x0000'0FF8U, false,
    core::reserved_bit_policy::write_zero>;
using spi_enable = core::field_descriptor<spi_cr1, 0, 1>;

inline constexpr std::array spi_registers{
    register_record{"SPIx_CR1", 0x4001'3000U, 4, core::access_semantics::read_write, 0x0000'1FFFU, 0U, 0U, core::reserved_bit_policy::preserve, spi_source},
    register_record{"SPIx_CR2", 0x4001'3004U, 4, core::access_semantics::read_write, 0x0000'FFFFU, 0U, 0U, core::reserved_bit_policy::preserve, spi_source},
    register_record{"SPIx_CFG1", 0x4001'3008U, 4, core::access_semantics::read_write, 0xFFFF'FFFFU, 0x7000'0007U, 0U, core::reserved_bit_policy::preserve, spi_source},
    register_record{"SPIx_CFG2", 0x4001'300CU, 4, core::access_semantics::read_write, 0xFFFF'FFFFU, 0U, 0U, core::reserved_bit_policy::preserve, spi_source},
    register_record{"SPIx_IER", 0x4001'3010U, 4, core::access_semantics::read_write, 0x0000'0FFBU, 0U, 0U, core::reserved_bit_policy::write_zero, spi_source},
    register_record{"SPIx_SR", 0x4001'3014U, 4, core::access_semantics::read_only, 0U, 0U, 0U, core::reserved_bit_policy::write_zero, spi_source},
    register_record{"SPIx_IFCR", 0x4001'3018U, 4, core::access_semantics::write_one_to_clear, 0x0000'0FF8U, 0U, 0xFFFF'FFFFU, core::reserved_bit_policy::write_zero, spi_source},
    register_record{"SPIx_TXDR", 0x4001'3020U, 4, core::access_semantics::write_only, 0xFFFF'FFFFU, 0U, 0xFFFF'FFFFU, core::reserved_bit_policy::write_zero, spi_source},
    register_record{"SPIx_RXDR", 0x4001'3030U, 4, core::access_semantics::read_only, 0U, 0U, 0U, core::reserved_bit_policy::write_zero, spi_source},
    register_record{"SPIx_CRCPOLY", 0x4001'3040U, 4, core::access_semantics::read_write, 0xFFFF'FFFFU, 0x107U, 0U, core::reserved_bit_policy::write_zero, spi_source},
    register_record{"SPIx_TXCRC", 0x4001'3044U, 4, core::access_semantics::read_only, 0U, 0U, 0U, core::reserved_bit_policy::write_zero, spi_source},
    register_record{"SPIx_RXCRC", 0x4001'3048U, 4, core::access_semantics::read_only, 0U, 0U, 0U, core::reserved_bit_policy::write_zero, spi_source},
    register_record{"SPIx_UDRDR", 0x4001'304CU, 4, core::access_semantics::read_write, 0xFFFF'FFFFU, 0U, 0U, core::reserved_bit_policy::write_zero, spi_source},
    register_record{"SPIx_I2SCFGR", 0x4001'3050U, 4, core::access_semantics::read_write, 0xFFFF'FFFFU, 0U, 0U, core::reserved_bit_policy::preserve, spi_source},
};

inline constexpr core::source_ref adc_source{
    "RM0433 / ES0392", "Rev.8 / Rev.15",
    "RM0433 Chapter 25, PDF pages 1315-1446; ES0392 Section 2.9"};
using adc_isr = core::register_descriptor<std::uint32_t, 0x00,
    core::access_semantics::write_one_to_clear, 0x0000'0FFFU, false,
    core::reserved_bit_policy::write_zero>;
using adc_cr = core::register_descriptor<std::uint32_t, 0x08,
    core::access_semantics::read_write, 0xF000'003FU, false,
    core::reserved_bit_policy::preserve>;
using adc_dr = core::register_descriptor<std::uint32_t, 0x40,
    core::access_semantics::read_only, 0U, false,
    core::reserved_bit_policy::write_zero>;
using adc_ready = core::field_descriptor<adc_isr, 0, 1,
    core::access_semantics::write_one_to_clear>;
using adc_enable = core::field_descriptor<adc_cr, 0, 1>;

inline constexpr std::array adc_registers{
    register_record{"ADCx_ISR", 0x4002'2000U, 4, core::access_semantics::write_one_to_clear, 0x0000'0FFFU, 0U, 0xFFFF'FFFFU, core::reserved_bit_policy::write_zero, adc_source},
    register_record{"ADCx_IER", 0x4002'2004U, 4, core::access_semantics::read_write, 0x0000'0FFFU, 0U, 0U, core::reserved_bit_policy::write_zero, adc_source},
    register_record{"ADCx_CR", 0x4002'2008U, 4, core::access_semantics::read_write, 0xF000'003FU, 0x2000'0000U, 0U, core::reserved_bit_policy::preserve, adc_source},
    register_record{"ADCx_CFGR", 0x4002'200CU, 4, core::access_semantics::read_write, 0xFFFF'FFFFU, 0U, 0U, core::reserved_bit_policy::preserve, adc_source},
    register_record{"ADCx_CFGR2", 0x4002'2010U, 4, core::access_semantics::read_write, 0xFFFF'FFFFU, 0U, 0U, core::reserved_bit_policy::preserve, adc_source},
    register_record{"ADCx_SMPR1", 0x4002'2014U, 4, core::access_semantics::read_write, 0x3FFF'FFFFU, 0U, 0U, core::reserved_bit_policy::write_zero, adc_source},
    register_record{"ADCx_SMPR2", 0x4002'2018U, 4, core::access_semantics::read_write, 0x3FFF'FFFFU, 0U, 0U, core::reserved_bit_policy::write_zero, adc_source},
    register_record{"ADCx_PCSEL", 0x4002'201CU, 4, core::access_semantics::read_write, 0x000F'FFFFU, 0U, 0U, core::reserved_bit_policy::write_zero, adc_source},
    register_record{"ADCx_SQR1", 0x4002'2030U, 4, core::access_semantics::read_write, 0xFFFF'FFFFU, 0U, 0U, core::reserved_bit_policy::write_zero, adc_source},
    register_record{"ADCx_SQR2", 0x4002'2034U, 4, core::access_semantics::read_write, 0xFFFF'FFFFU, 0U, 0U, core::reserved_bit_policy::write_zero, adc_source},
    register_record{"ADCx_SQR3", 0x4002'2038U, 4, core::access_semantics::read_write, 0xFFFF'FFFFU, 0U, 0U, core::reserved_bit_policy::write_zero, adc_source},
    register_record{"ADCx_SQR4", 0x4002'203CU, 4, core::access_semantics::read_write, 0xFFFF'FFFFU, 0U, 0U, core::reserved_bit_policy::write_zero, adc_source},
    register_record{"ADCx_DR", 0x4002'2040U, 4, core::access_semantics::read_only, 0U, 0U, 0U, core::reserved_bit_policy::write_zero, adc_source},
};

inline constexpr std::array initial_fields{
    field_record{"GPIOx_MODER", "MODER0", 0, 2, core::access_semantics::read_write, 0x0FU, alternate_function_source},
    field_record{"GPIOx_AFRL", "AFSEL0", 0, 4, core::access_semantics::read_write, 0xFFFFU, alternate_function_source},
    field_record{"DMA1_S0CR", "EN", 0, 1, core::access_semantics::read_write, 0x03U, dma_source},
    field_record{"DMAMUX1_C0CR", "DMAREQ_ID", 0, 7, core::access_semantics::read_write, 0U, dma_source},
    field_record{"TIMx_CR1", "CMS", 5, 2, core::access_semantics::read_write, 0x0FU, timer_source},
    field_record{"TIMx_SR", "UIF", 0, 1, core::access_semantics::write_zero_to_clear, 0x03U, timer_source},
    field_record{"I2Cx_CR1", "PE", 0, 1, core::access_semantics::read_write, 0x03U, i2c_source},
    field_record{"USARTx_CR1", "UE", 0, 1, core::access_semantics::read_write, 0x03U, usart_source},
    field_record{"SPIx_CR1", "SPE", 0, 1, core::access_semantics::read_write, 0x03U, spi_source},
    field_record{"ADCx_ISR", "ADRDY", 0, 1, core::access_semantics::write_one_to_clear, 0x03U, adc_source},
};

} // namespace mcuhw::mcu::stm32h753
