#pragma once

#include <array>
#include <cstdint>
#include <limits>

#include <mcuhw/mcu/ra4m1/interrupts_dma.hpp>
#include <mcuhw/mcu/ra4m1/pins.hpp>

namespace mcuhw::mcu::ra4m1 {

inline constexpr core::source_ref gpio_source{
    "R01UH0887EJ0110", "Rev.1.10",
    "19.2.1-19.2.4, PDF pages 357-360"};
using pcntr1 = core::register_descriptor<std::uint32_t, 0x00,
    core::access_semantics::read_write, 0xFFFF'FFFFU, true,
    core::reserved_bit_policy::write_zero, 0U, 0U, 0xFFFF'FFFFU>;
using pcntr2 = core::register_descriptor<std::uint32_t, 0x04,
    core::access_semantics::read_only, 0U, false,
    core::reserved_bit_policy::write_zero, 0U, 0U, 0xFFFF'FFFFU>;
using pcntr3 = core::register_descriptor<std::uint32_t, 0x08,
    core::access_semantics::write_only, 0xFFFF'FFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0U, 0xFFFF'FFFFU>;
using pcntr4 = core::register_descriptor<std::uint32_t, 0x0C,
    core::access_semantics::write_only, 0xFFFF'FFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0U, 0xFFFF'FFFFU>;
using podr = core::field_descriptor<pcntr1, 0, 16>;
using pdr = core::field_descriptor<pcntr1, 16, 16>;
using pidr = core::field_descriptor<pcntr2, 0, 16, core::access_semantics::read_only>;
using eidr = core::field_descriptor<pcntr2, 16, 16, core::access_semantics::read_only>;

inline constexpr std::array gpio_registers{
    register_record{"PCNTR1/PODR/PDR", 0x4004'0000U, 4, core::access_semantics::read_write, 0xFFFF'FFFFU, 0U, 0xFFFF'FFFFU, core::reserved_bit_policy::write_zero, gpio_source},
    register_record{"PCNTR2/PIDR/EIDR", 0x4004'0004U, 4, core::access_semantics::read_only, 0U, 0U, 0xFFFF'FFFFU, core::reserved_bit_policy::write_zero, gpio_source},
    register_record{"PCNTR3/POSR/PORR", 0x4004'0008U, 4, core::access_semantics::write_only, 0xFFFF'FFFFU, 0U, 0xFFFF'FFFFU, core::reserved_bit_policy::write_zero, gpio_source},
    register_record{"PCNTR4/EOSR/EORR", 0x4004'000CU, 4, core::access_semantics::write_only, 0xFFFF'FFFFU, 0U, 0xFFFF'FFFFU, core::reserved_bit_policy::write_zero, gpio_source},
};

inline constexpr core::source_ref gpt_source{
    "R01UH0887EJ0110", "Rev.1.10",
    "22.1-22.2 and 22.4-22.9, PDF pages 392-512"};

inline constexpr std::array gpt_instances{
    gpt_instance{0, 32, 0x4007'8000U, {gpt_output::a, gpt_output::b}, gpt_source},
    gpt_instance{1, 32, 0x4007'8100U, {gpt_output::a, gpt_output::b}, gpt_source},
    gpt_instance{2, 16, 0x4007'8200U, {gpt_output::a, gpt_output::b}, gpt_source},
    gpt_instance{3, 16, 0x4007'8300U, {gpt_output::a, gpt_output::b}, gpt_source},
    gpt_instance{4, 16, 0x4007'8400U, {gpt_output::a, gpt_output::b}, gpt_source},
    gpt_instance{5, 16, 0x4007'8500U, {gpt_output::a, gpt_output::b}, gpt_source},
    gpt_instance{6, 16, 0x4007'8600U, {gpt_output::a, gpt_output::b}, gpt_source},
    gpt_instance{7, 16, 0x4007'8700U, {gpt_output::a, gpt_output::b}, gpt_source},
};

using gtcr = core::register_descriptor<std::uint32_t, 0x2C,
    core::access_semantics::read_write, 0x0707'FFFFU, true,
    core::reserved_bit_policy::write_zero, 0U, 0U, 0xFFFF'FFFFU>;
using gtior = core::register_descriptor<std::uint32_t, 0x34,
    core::access_semantics::read_write, 0x3F3F'3F3FU, true,
    core::reserved_bit_policy::write_zero, 0U, 0U, 0xFFFF'FFFFU>;
using gtcnt = core::register_descriptor<std::uint32_t, 0x48,
    core::access_semantics::read_write, 0xFFFF'FFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0U, 0xFFFF'FFFFU>;
using gtccra = core::register_descriptor<std::uint32_t, 0x4C,
    core::access_semantics::read_write, 0xFFFF'FFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0U, 0xFFFF'FFFFU>;
using gtccrb = core::register_descriptor<std::uint32_t, 0x50,
    core::access_semantics::read_write, 0xFFFF'FFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0U, 0xFFFF'FFFFU>;
using gtpr = core::register_descriptor<std::uint32_t, 0x64,
    core::access_semantics::read_write, 0xFFFF'FFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0xFFFF'FFFFU, 0xFFFF'FFFFU>;
using gtcr_mode = core::field_descriptor<gtcr, 16, 3>;
using gtcr_prescaler = core::field_descriptor<gtcr, 24, 3>;
using gtior_output_a_enable = core::field_descriptor<gtior, 8, 1>;
using gtior_output_b_enable = core::field_descriptor<gtior, 24, 1>;

inline constexpr std::array gpt_registers{
    register_record{"GTWP", 0x4007'8000U, 4, core::access_semantics::read_write, 0x0000'FF01U, 0U, 0xFFFF'FFFFU, core::reserved_bit_policy::write_zero, gpt_source},
    register_record{"GTSTR", 0x4007'8004U, 4, core::access_semantics::write_only, 0x0000'00FFU, 0U, 0xFFFF'FFFFU, core::reserved_bit_policy::write_zero, gpt_source},
    register_record{"GTSTP", 0x4007'8008U, 4, core::access_semantics::write_only, 0x0000'00FFU, 0U, 0xFFFF'FFFFU, core::reserved_bit_policy::write_zero, gpt_source},
    register_record{"GTCLR", 0x4007'800CU, 4, core::access_semantics::write_only, 0x0000'00FFU, 0U, 0xFFFF'FFFFU, core::reserved_bit_policy::write_zero, gpt_source},
    register_record{"GTSSR", 0x4007'8010U, 4, core::access_semantics::read_write, 0xFFFF'FFFFU, 0U, 0xFFFF'FFFFU, core::reserved_bit_policy::write_zero, gpt_source},
    register_record{"GTPSR", 0x4007'8014U, 4, core::access_semantics::read_write, 0xFFFF'FFFFU, 0U, 0xFFFF'FFFFU, core::reserved_bit_policy::write_zero, gpt_source},
    register_record{"GTCSR", 0x4007'8018U, 4, core::access_semantics::read_write, 0xFFFF'FFFFU, 0U, 0xFFFF'FFFFU, core::reserved_bit_policy::write_zero, gpt_source},
    register_record{"GTUPSR", 0x4007'801CU, 4, core::access_semantics::read_write, 0xFFFF'FFFFU, 0U, 0xFFFF'FFFFU, core::reserved_bit_policy::write_zero, gpt_source},
    register_record{"GTDNSR", 0x4007'8020U, 4, core::access_semantics::read_write, 0xFFFF'FFFFU, 0U, 0xFFFF'FFFFU, core::reserved_bit_policy::write_zero, gpt_source},
    register_record{"GTICASR", 0x4007'8024U, 4, core::access_semantics::read_write, 0xFFFF'FFFFU, 0U, 0xFFFF'FFFFU, core::reserved_bit_policy::write_zero, gpt_source},
    register_record{"GTICBSR", 0x4007'8028U, 4, core::access_semantics::read_write, 0xFFFF'FFFFU, 0U, 0xFFFF'FFFFU, core::reserved_bit_policy::write_zero, gpt_source},
    register_record{"GTCR", 0x4007'802CU, 4, core::access_semantics::read_write, 0x0707'FFFFU, 0U, 0xFFFF'FFFFU, core::reserved_bit_policy::write_zero, gpt_source},
    register_record{"GTUDDTYC", 0x4007'8030U, 4, core::access_semantics::read_write, 0x0000'003FU, 0U, 0xFFFF'FFFFU, core::reserved_bit_policy::write_zero, gpt_source},
    register_record{"GTIOR", 0x4007'8034U, 4, core::access_semantics::read_write, 0x3F3F'3F3FU, 0U, 0xFFFF'FFFFU, core::reserved_bit_policy::write_zero, gpt_source},
    register_record{"GTINTAD", 0x4007'8038U, 4, core::access_semantics::read_write, 0x01FF'FFFFU, 0U, 0xFFFF'FFFFU, core::reserved_bit_policy::write_zero, gpt_source},
    register_record{"GTST", 0x4007'803CU, 4, core::access_semantics::read_write, 0x0000'0F3FU, 0U, 0xFFFF'FFFFU, core::reserved_bit_policy::preserve, gpt_source},
    register_record{"GTBER", 0x4007'8040U, 4, core::access_semantics::read_write, 0x00FF'FFFFU, 0U, 0xFFFF'FFFFU, core::reserved_bit_policy::write_zero, gpt_source},
    register_record{"GTCNT", 0x4007'8048U, 4, core::access_semantics::read_write, 0xFFFF'FFFFU, 0U, 0xFFFF'FFFFU, core::reserved_bit_policy::write_zero, gpt_source},
    register_record{"GTCCRA-F", 0x4007'804CU, 4, core::access_semantics::read_write, 0xFFFF'FFFFU, 0U, 0xFFFF'FFFFU, core::reserved_bit_policy::write_zero, gpt_source},
    register_record{"GTPR", 0x4007'8064U, 4, core::access_semantics::read_write, 0xFFFF'FFFFU, 0xFFFF'FFFFU, 0xFFFF'FFFFU, core::reserved_bit_policy::write_zero, gpt_source},
    register_record{"GTPBR", 0x4007'8068U, 4, core::access_semantics::read_write, 0xFFFF'FFFFU, 0xFFFF'FFFFU, 0xFFFF'FFFFU, core::reserved_bit_policy::write_zero, gpt_source},
    register_record{"GTDTCR", 0x4007'8088U, 4, core::access_semantics::read_write, 0x0000'003FU, 0U, 0xFFFF'FFFFU, core::reserved_bit_policy::write_zero, gpt_source},
    register_record{"GTDVU", 0x4007'808CU, 4, core::access_semantics::read_write, 0xFFFF'FFFFU, 0U, 0xFFFF'FFFFU, core::reserved_bit_policy::write_zero, gpt_source},
    register_record{"OPSCR", 0x4007'8FF0U, 4, core::access_semantics::read_write, 0x0000'3FFFU, 0U, 0xFFFF'FFFFU, core::reserved_bit_policy::write_zero, gpt_source},
};

inline constexpr core::source_ref sci_source{
    "R01UH0887EJ0110", "Rev.1.10",
    "28.1-28.2 and 28.10-28.14, PDF pages 696-821"};
using sci_smr = core::register_descriptor<std::uint8_t, 0x00, core::access_semantics::read_write>;
using sci_brr = core::register_descriptor<std::uint8_t, 0x01, core::access_semantics::read_write, 0xFFU, false, core::reserved_bit_policy::write_zero, 0U, 0xFFU, 0xFFU>;
using sci_scr = core::register_descriptor<std::uint8_t, 0x02, core::access_semantics::read_write>;
using sci_tdr = core::register_descriptor<std::uint8_t, 0x03, core::access_semantics::write_only>;
using sci_ssr = core::register_descriptor<std::uint8_t, 0x04, core::access_semantics::read_write, 0x38U, false, core::reserved_bit_policy::preserve, 0U, 0x84U, 0xFFU>;
using sci_rdr = core::register_descriptor<std::uint8_t, 0x05, core::access_semantics::read_only, 0U, false, core::reserved_bit_policy::write_zero, 0U, 0U, 0U>;
using sci_semr = core::register_descriptor<std::uint8_t, 0x07, core::access_semantics::read_write>;
using sci_clock_select = core::field_descriptor<sci_smr, 0, 2>;
using sci_transmit_enable = core::field_descriptor<sci_scr, 5, 1>;
using sci_receive_enable = core::field_descriptor<sci_scr, 4, 1>;
using sci_parity_error = core::field_descriptor<sci_ssr, 3, 1, core::access_semantics::write_zero_to_clear>;

inline constexpr std::array sci_uart_registers{
    register_record{"SMR", 0x4007'0000U, 1, core::access_semantics::read_write, 0xFFU, 0U, 0xFFU, core::reserved_bit_policy::write_zero, sci_source},
    register_record{"BRR", 0x4007'0001U, 1, core::access_semantics::read_write, 0xFFU, 0xFFU, 0xFFU, core::reserved_bit_policy::write_zero, sci_source},
    register_record{"SCR", 0x4007'0002U, 1, core::access_semantics::read_write, 0xFFU, 0U, 0xFFU, core::reserved_bit_policy::write_zero, sci_source},
    register_record{"TDR", 0x4007'0003U, 1, core::access_semantics::write_only, 0xFFU, 0xFFU, 0xFFU, core::reserved_bit_policy::write_zero, sci_source},
    register_record{"SSR", 0x4007'0004U, 1, core::access_semantics::read_write, 0x38U, 0x84U, 0xFFU, core::reserved_bit_policy::preserve, sci_source},
    register_record{"RDR", 0x4007'0005U, 1, core::access_semantics::read_only, 0U, 0U, 0U, core::reserved_bit_policy::write_zero, sci_source},
    register_record{"SCMR", 0x4007'0006U, 1, core::access_semantics::read_write, 0xFFU, 0xF2U, 0xFFU, core::reserved_bit_policy::write_zero, sci_source},
    register_record{"SEMR", 0x4007'0007U, 1, core::access_semantics::read_write, 0xFFU, 0U, 0xFFU, core::reserved_bit_policy::write_zero, sci_source},
    register_record{"MDDR", 0x4007'0012U, 1, core::access_semantics::read_write, 0xFFU, 0xFFU, 0xFFU, core::reserved_bit_policy::write_zero, sci_source},
};

inline constexpr core::source_ref iic_source{
    "R01UH0887EJ0110", "Rev.1.10",
    "29.1-29.2 and 29.14-29.17, PDF pages 822-899"};
using iccr1 = core::register_descriptor<std::uint8_t, 0x00, core::access_semantics::read_write>;
using iccr2 = core::register_descriptor<std::uint8_t, 0x01, core::access_semantics::read_write>;
using icmr1 = core::register_descriptor<std::uint8_t, 0x02, core::access_semantics::read_write>;
using icier = core::register_descriptor<std::uint8_t, 0x07, core::access_semantics::read_write>;
using icsr2 = core::register_descriptor<std::uint8_t, 0x09, core::access_semantics::read_write, 0x7FU, false, core::reserved_bit_policy::preserve>;
using icdrr = core::register_descriptor<std::uint8_t, 0x13, core::access_semantics::read_only, 0U, false, core::reserved_bit_policy::write_zero, 0U, 0U, 0U>;
using iic_enable = core::field_descriptor<iccr1, 7, 1>;
using iic_start = core::field_descriptor<iccr2, 1, 1>;
using iic_stop = core::field_descriptor<iccr2, 3, 1>;
using iic_nack = core::field_descriptor<icsr2, 4, 1, core::access_semantics::write_zero_to_clear>;

inline constexpr std::array iic_registers{
    register_record{"ICCR1", 0x4005'3000U, 1, core::access_semantics::read_write, 0xFFU, 0U, 0xFFU, core::reserved_bit_policy::write_zero, iic_source},
    register_record{"ICCR2", 0x4005'3001U, 1, core::access_semantics::read_write, 0xFFU, 0U, 0xFFU, core::reserved_bit_policy::write_zero, iic_source},
    register_record{"ICMR1", 0x4005'3002U, 1, core::access_semantics::read_write, 0xFFU, 0U, 0xFFU, core::reserved_bit_policy::write_zero, iic_source},
    register_record{"ICMR2", 0x4005'3003U, 1, core::access_semantics::read_write, 0xFFU, 0U, 0xFFU, core::reserved_bit_policy::write_zero, iic_source},
    register_record{"ICMR3", 0x4005'3004U, 1, core::access_semantics::read_write, 0xFFU, 0U, 0xFFU, core::reserved_bit_policy::write_zero, iic_source},
    register_record{"ICFER", 0x4005'3005U, 1, core::access_semantics::read_write, 0x7FU, 0U, 0xFFU, core::reserved_bit_policy::write_zero, iic_source},
    register_record{"ICSER", 0x4005'3006U, 1, core::access_semantics::read_write, 0x7FU, 0U, 0xFFU, core::reserved_bit_policy::write_zero, iic_source},
    register_record{"ICIER", 0x4005'3007U, 1, core::access_semantics::read_write, 0xFFU, 0U, 0xFFU, core::reserved_bit_policy::write_zero, iic_source},
    register_record{"ICSR1", 0x4005'3008U, 1, core::access_semantics::read_write, 0x7FU, 0U, 0xFFU, core::reserved_bit_policy::preserve, iic_source},
    register_record{"ICSR2", 0x4005'3009U, 1, core::access_semantics::read_write, 0x7FU, 0U, 0xFFU, core::reserved_bit_policy::preserve, iic_source},
    register_record{"SARL0-2", 0x4005'300AU, 1, core::access_semantics::read_write, 0xFEU, 0U, 0xFFU, core::reserved_bit_policy::write_zero, iic_source},
    register_record{"SARU0-2", 0x4005'300BU, 1, core::access_semantics::read_write, 0x07U, 0U, 0xFFU, core::reserved_bit_policy::write_zero, iic_source},
    register_record{"ICBRL", 0x4005'3010U, 1, core::access_semantics::read_write, 0xFFU, 0xFFU, 0xFFU, core::reserved_bit_policy::write_zero, iic_source},
    register_record{"ICBRH", 0x4005'3011U, 1, core::access_semantics::read_write, 0xFFU, 0xFFU, 0xFFU, core::reserved_bit_policy::write_zero, iic_source},
    register_record{"ICDRT", 0x4005'3012U, 1, core::access_semantics::read_write, 0xFFU, 0xFFU, 0xFFU, core::reserved_bit_policy::write_zero, iic_source},
    register_record{"ICDRR", 0x4005'3013U, 1, core::access_semantics::read_only, 0U, 0U, 0xFFU, core::reserved_bit_policy::write_zero, iic_source},
};

inline constexpr core::source_ref spi_source{
    "R01UH0887EJ0110", "Rev.1.10",
    "31.1-31.2 and 31.5, PDF pages 946-961 and 1007-1011"};
using spcr = core::register_descriptor<std::uint8_t, 0x00, core::access_semantics::read_write>;
using spsr = core::register_descriptor<std::uint8_t, 0x03, core::access_semantics::read_write, 0x1DU, false, core::reserved_bit_policy::preserve>;
using spdr = core::register_descriptor<std::uint32_t, 0x04, core::access_semantics::read_write>;
using spdr_ha = core::register_descriptor<std::uint16_t, 0x04, core::access_semantics::read_write, 0xFFFFU, false, core::reserved_bit_policy::write_zero, 0U, 0U, 0xFFFFU, spdr>;
using spbr = core::register_descriptor<std::uint8_t, 0x0A, core::access_semantics::read_write, 0xFFU, false, core::reserved_bit_policy::write_zero, 0U, 0xFFU, 0xFFU>;
using spcr2 = core::register_descriptor<std::uint8_t, 0x0F, core::access_semantics::read_write, 0x1FU, false, core::reserved_bit_policy::write_zero>;
using spcmd0 = core::register_descriptor<std::uint16_t, 0x10, core::access_semantics::read_write>;
using spi_enable = core::field_descriptor<spcr, 6, 1>;
using spi_master = core::field_descriptor<spcr, 3, 1>;
using spi_mode_fault = core::field_descriptor<spsr, 2, 1, core::access_semantics::write_zero_to_clear>;
using spi_word_length = core::field_descriptor<spcmd0, 8, 4>;

inline constexpr std::array spi_registers{
    register_record{"SPCR", 0x4007'2000U, 1, core::access_semantics::read_write, 0xFFU, 0U, 0xFFU, core::reserved_bit_policy::write_zero, spi_source},
    register_record{"SSLP", 0x4007'2001U, 1, core::access_semantics::read_write, 0x0FU, 0U, 0xFFU, core::reserved_bit_policy::write_zero, spi_source},
    register_record{"SPPCR", 0x4007'2002U, 1, core::access_semantics::read_write, 0xCFU, 0U, 0xFFU, core::reserved_bit_policy::write_zero, spi_source},
    register_record{"SPSR", 0x4007'2003U, 1, core::access_semantics::read_write, 0x1DU, 0x20U, 0xFFU, core::reserved_bit_policy::preserve, spi_source},
    register_record{"SPDR", 0x4007'2004U, 4, core::access_semantics::read_write, 0xFFFF'FFFFU, 0U, 0xFFFF'FFFFU, core::reserved_bit_policy::write_zero, spi_source},
    register_record{"SPDR_HA", 0x4007'2004U, 2, core::access_semantics::read_write, 0xFFFFU, 0U, 0xFFFFU, core::reserved_bit_policy::write_zero, spi_source},
    register_record{"SPBR", 0x4007'200AU, 1, core::access_semantics::read_write, 0xFFU, 0xFFU, 0xFFU, core::reserved_bit_policy::write_zero, spi_source},
    register_record{"SPDCR", 0x4007'200BU, 1, core::access_semantics::read_write, 0xE0U, 0U, 0xFFU, core::reserved_bit_policy::write_zero, spi_source},
    register_record{"SPCKD", 0x4007'200CU, 1, core::access_semantics::read_write, 0x07U, 0U, 0xFFU, core::reserved_bit_policy::write_zero, spi_source},
    register_record{"SSLND", 0x4007'200DU, 1, core::access_semantics::read_write, 0x07U, 0U, 0xFFU, core::reserved_bit_policy::write_zero, spi_source},
    register_record{"SPND", 0x4007'200EU, 1, core::access_semantics::read_write, 0x07U, 0U, 0xFFU, core::reserved_bit_policy::write_zero, spi_source},
    register_record{"SPCR2", 0x4007'200FU, 1, core::access_semantics::read_write, 0x1FU, 0U, 0xFFU, core::reserved_bit_policy::write_zero, spi_source},
    register_record{"SPCMD0", 0x4007'2010U, 2, core::access_semantics::read_write, 0xFFFFU, 0x070DU, 0xFFFFU, core::reserved_bit_policy::write_zero, spi_source},
};

inline constexpr core::source_ref adc_source{
    "R01UH0887EJ0110", "Rev.1.10",
    "35.1-35.2 and 35.4-35.8, PDF pages 1084-1162"};
using adcsr = core::register_descriptor<std::uint16_t, 0x00, core::access_semantics::read_write, 0xE7DFU, false, core::reserved_bit_policy::preserve>;
using adansa0 = core::register_descriptor<std::uint16_t, 0x04, core::access_semantics::read_write, 0x7FFFU, true, core::reserved_bit_policy::write_zero>;
using adcer = core::register_descriptor<std::uint16_t, 0x0E, core::access_semantics::read_write, 0xFF9FU, false, core::reserved_bit_policy::write_zero>;
using addr0 = core::register_descriptor<std::uint16_t, 0x20, core::access_semantics::read_only, 0U, false, core::reserved_bit_policy::write_zero, 0U, 0U, 0U>;
using adc_start = core::field_descriptor<adcsr, 15, 1>;
using adc_scan_mode = core::field_descriptor<adcsr, 13, 2>;
using adc_resolution = core::field_descriptor<adcer, 1, 2>;

inline constexpr std::array adc_registers{
    register_record{"ADCSR", 0x4005'C000U, 2, core::access_semantics::read_write, 0xE7DFU, 0U, 0xFFFFU, core::reserved_bit_policy::preserve, adc_source},
    register_record{"ADANSA0", 0x4005'C004U, 2, core::access_semantics::read_write, 0x7FFFU, 0U, 0xFFFFU, core::reserved_bit_policy::write_zero, adc_source},
    register_record{"ADANSA1", 0x4005'C006U, 2, core::access_semantics::read_write, 0x0000U, 0U, 0xFFFFU, core::reserved_bit_policy::write_zero, adc_source},
    register_record{"ADADS0", 0x4005'C008U, 2, core::access_semantics::read_write, 0x7FFFU, 0U, 0xFFFFU, core::reserved_bit_policy::write_zero, adc_source},
    register_record{"ADADC", 0x4005'C00CU, 1, core::access_semantics::read_write, 0x83U, 0U, 0xFFU, core::reserved_bit_policy::write_zero, adc_source},
    register_record{"ADCER", 0x4005'C00EU, 2, core::access_semantics::read_write, 0xFF9FU, 0U, 0xFFFFU, core::reserved_bit_policy::write_zero, adc_source},
    register_record{"ADSTRGR", 0x4005'C010U, 2, core::access_semantics::read_write, 0x3F3FU, 0U, 0xFFFFU, core::reserved_bit_policy::write_zero, adc_source},
    register_record{"ADEXICR", 0x4005'C012U, 2, core::access_semantics::read_write, 0x3F3FU, 0U, 0xFFFFU, core::reserved_bit_policy::write_zero, adc_source},
    register_record{"ADDR0-14", 0x4005'C020U, 2, core::access_semantics::read_only, 0U, 0U, 0U, core::reserved_bit_policy::write_zero, adc_source},
    register_record{"ADSSTR00-14", 0x4005'C0E0U, 1, core::access_semantics::read_write, 0xFFU, 0x0DU, 0xFFU, core::reserved_bit_policy::write_zero, adc_source},
};

// Representative field catalogue preserves mixed field semantics that a
// register-level access label cannot express by itself.
inline constexpr std::array initial_fields{
    field_record{"PmnPFS", "PSEL", 24, 5, core::access_semantics::read_write, 0U, pfs_source},
    field_record{"PmnPFS", "ASEL", 15, 1, core::access_semantics::read_write, 0x3U, pfs_source},
    field_record{"PmnPFS", "PIDR", 1, 1, core::access_semantics::read_only, 0x3U, pfs_source},
    field_record{"GTIOR", "OAE", 8, 1, core::access_semantics::read_write, 0x3U, gpt_source},
    field_record{"GTIOR", "OBE", 24, 1, core::access_semantics::read_write, 0x3U, gpt_source},
    field_record{"SCR", "TE", 5, 1, core::access_semantics::read_write, 0x3U, sci_source},
    field_record{"SSR", "PER", 3, 1, core::access_semantics::write_zero_to_clear, 0x3U, sci_source},
    field_record{"ICCR2", "ST", 1, 1, core::access_semantics::read_write, 0x3U, iic_source},
    field_record{"ICSR2", "NACKF", 4, 1, core::access_semantics::write_zero_to_clear, 0x3U, iic_source},
    field_record{"SPCR", "SPE", 6, 1, core::access_semantics::read_write, 0x3U, spi_source},
    field_record{"SPSR", "MODF", 2, 1, core::access_semantics::write_zero_to_clear, 0x3U, spi_source},
    field_record{"ADCSR", "ADST", 15, 1, core::access_semantics::read_write, 0x3U, adc_source},
};

} // namespace mcuhw::mcu::ra4m1
