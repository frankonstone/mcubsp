#pragma once

// @req-file REQ-0055
// Nordic Semiconductor BSD-3-Clause derived register facts.
// Source: nrfx v3.2.0 nrf52840.svd; license: doc/reference/nrf52840/NORDIC-LICENSE.txt.
// Raw source facts; see system.hpp and doc/reference/nrf52840/README.md for limits.
#include <mcubsp/core/register.hpp>

namespace mcubsp::mcu::nrf52840::registers::ppi {
struct tasks_chg0_en : core::register_descriptor<std::uint32_t, 0x0U,
    core::access_semantics::command, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace tasks_chg0_en_fields {
using en = core::field_descriptor<tasks_chg0_en, 0U, 1U>;
inline constexpr std::uint32_t en_trigger = 0x1U;
}
struct tasks_chg0_dis : core::register_descriptor<std::uint32_t, 0x4U,
    core::access_semantics::command, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace tasks_chg0_dis_fields {
using dis = core::field_descriptor<tasks_chg0_dis, 0U, 1U>;
inline constexpr std::uint32_t dis_trigger = 0x1U;
}
struct tasks_chg1_en : core::register_descriptor<std::uint32_t, 0x8U,
    core::access_semantics::command, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace tasks_chg1_en_fields {
using en = core::field_descriptor<tasks_chg1_en, 0U, 1U>;
inline constexpr std::uint32_t en_trigger = 0x1U;
}
struct tasks_chg1_dis : core::register_descriptor<std::uint32_t, 0xCU,
    core::access_semantics::command, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace tasks_chg1_dis_fields {
using dis = core::field_descriptor<tasks_chg1_dis, 0U, 1U>;
inline constexpr std::uint32_t dis_trigger = 0x1U;
}
struct tasks_chg2_en : core::register_descriptor<std::uint32_t, 0x10U,
    core::access_semantics::command, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace tasks_chg2_en_fields {
using en = core::field_descriptor<tasks_chg2_en, 0U, 1U>;
inline constexpr std::uint32_t en_trigger = 0x1U;
}
struct tasks_chg2_dis : core::register_descriptor<std::uint32_t, 0x14U,
    core::access_semantics::command, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace tasks_chg2_dis_fields {
using dis = core::field_descriptor<tasks_chg2_dis, 0U, 1U>;
inline constexpr std::uint32_t dis_trigger = 0x1U;
}
struct tasks_chg3_en : core::register_descriptor<std::uint32_t, 0x18U,
    core::access_semantics::command, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace tasks_chg3_en_fields {
using en = core::field_descriptor<tasks_chg3_en, 0U, 1U>;
inline constexpr std::uint32_t en_trigger = 0x1U;
}
struct tasks_chg3_dis : core::register_descriptor<std::uint32_t, 0x1CU,
    core::access_semantics::command, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace tasks_chg3_dis_fields {
using dis = core::field_descriptor<tasks_chg3_dis, 0U, 1U>;
inline constexpr std::uint32_t dis_trigger = 0x1U;
}
struct tasks_chg4_en : core::register_descriptor<std::uint32_t, 0x20U,
    core::access_semantics::command, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace tasks_chg4_en_fields {
using en = core::field_descriptor<tasks_chg4_en, 0U, 1U>;
inline constexpr std::uint32_t en_trigger = 0x1U;
}
struct tasks_chg4_dis : core::register_descriptor<std::uint32_t, 0x24U,
    core::access_semantics::command, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace tasks_chg4_dis_fields {
using dis = core::field_descriptor<tasks_chg4_dis, 0U, 1U>;
inline constexpr std::uint32_t dis_trigger = 0x1U;
}
struct tasks_chg5_en : core::register_descriptor<std::uint32_t, 0x28U,
    core::access_semantics::command, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace tasks_chg5_en_fields {
using en = core::field_descriptor<tasks_chg5_en, 0U, 1U>;
inline constexpr std::uint32_t en_trigger = 0x1U;
}
struct tasks_chg5_dis : core::register_descriptor<std::uint32_t, 0x2CU,
    core::access_semantics::command, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace tasks_chg5_dis_fields {
using dis = core::field_descriptor<tasks_chg5_dis, 0U, 1U>;
inline constexpr std::uint32_t dis_trigger = 0x1U;
}
struct chen : core::register_descriptor<std::uint32_t, 0x500U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace chen_fields {
using ch0 = core::field_descriptor<chen, 0U, 1U>;
inline constexpr std::uint32_t ch0_disabled = 0x0U;
inline constexpr std::uint32_t ch0_enabled = 0x1U;
using ch1 = core::field_descriptor<chen, 1U, 1U>;
inline constexpr std::uint32_t ch1_disabled = 0x0U;
inline constexpr std::uint32_t ch1_enabled = 0x1U;
using ch2 = core::field_descriptor<chen, 2U, 1U>;
inline constexpr std::uint32_t ch2_disabled = 0x0U;
inline constexpr std::uint32_t ch2_enabled = 0x1U;
using ch3 = core::field_descriptor<chen, 3U, 1U>;
inline constexpr std::uint32_t ch3_disabled = 0x0U;
inline constexpr std::uint32_t ch3_enabled = 0x1U;
using ch4 = core::field_descriptor<chen, 4U, 1U>;
inline constexpr std::uint32_t ch4_disabled = 0x0U;
inline constexpr std::uint32_t ch4_enabled = 0x1U;
using ch5 = core::field_descriptor<chen, 5U, 1U>;
inline constexpr std::uint32_t ch5_disabled = 0x0U;
inline constexpr std::uint32_t ch5_enabled = 0x1U;
using ch6 = core::field_descriptor<chen, 6U, 1U>;
inline constexpr std::uint32_t ch6_disabled = 0x0U;
inline constexpr std::uint32_t ch6_enabled = 0x1U;
using ch7 = core::field_descriptor<chen, 7U, 1U>;
inline constexpr std::uint32_t ch7_disabled = 0x0U;
inline constexpr std::uint32_t ch7_enabled = 0x1U;
using ch8 = core::field_descriptor<chen, 8U, 1U>;
inline constexpr std::uint32_t ch8_disabled = 0x0U;
inline constexpr std::uint32_t ch8_enabled = 0x1U;
using ch9 = core::field_descriptor<chen, 9U, 1U>;
inline constexpr std::uint32_t ch9_disabled = 0x0U;
inline constexpr std::uint32_t ch9_enabled = 0x1U;
using ch10 = core::field_descriptor<chen, 10U, 1U>;
inline constexpr std::uint32_t ch10_disabled = 0x0U;
inline constexpr std::uint32_t ch10_enabled = 0x1U;
using ch11 = core::field_descriptor<chen, 11U, 1U>;
inline constexpr std::uint32_t ch11_disabled = 0x0U;
inline constexpr std::uint32_t ch11_enabled = 0x1U;
using ch12 = core::field_descriptor<chen, 12U, 1U>;
inline constexpr std::uint32_t ch12_disabled = 0x0U;
inline constexpr std::uint32_t ch12_enabled = 0x1U;
using ch13 = core::field_descriptor<chen, 13U, 1U>;
inline constexpr std::uint32_t ch13_disabled = 0x0U;
inline constexpr std::uint32_t ch13_enabled = 0x1U;
using ch14 = core::field_descriptor<chen, 14U, 1U>;
inline constexpr std::uint32_t ch14_disabled = 0x0U;
inline constexpr std::uint32_t ch14_enabled = 0x1U;
using ch15 = core::field_descriptor<chen, 15U, 1U>;
inline constexpr std::uint32_t ch15_disabled = 0x0U;
inline constexpr std::uint32_t ch15_enabled = 0x1U;
using ch16 = core::field_descriptor<chen, 16U, 1U>;
inline constexpr std::uint32_t ch16_disabled = 0x0U;
inline constexpr std::uint32_t ch16_enabled = 0x1U;
using ch17 = core::field_descriptor<chen, 17U, 1U>;
inline constexpr std::uint32_t ch17_disabled = 0x0U;
inline constexpr std::uint32_t ch17_enabled = 0x1U;
using ch18 = core::field_descriptor<chen, 18U, 1U>;
inline constexpr std::uint32_t ch18_disabled = 0x0U;
inline constexpr std::uint32_t ch18_enabled = 0x1U;
using ch19 = core::field_descriptor<chen, 19U, 1U>;
inline constexpr std::uint32_t ch19_disabled = 0x0U;
inline constexpr std::uint32_t ch19_enabled = 0x1U;
using ch20 = core::field_descriptor<chen, 20U, 1U>;
inline constexpr std::uint32_t ch20_disabled = 0x0U;
inline constexpr std::uint32_t ch20_enabled = 0x1U;
using ch21 = core::field_descriptor<chen, 21U, 1U>;
inline constexpr std::uint32_t ch21_disabled = 0x0U;
inline constexpr std::uint32_t ch21_enabled = 0x1U;
using ch22 = core::field_descriptor<chen, 22U, 1U>;
inline constexpr std::uint32_t ch22_disabled = 0x0U;
inline constexpr std::uint32_t ch22_enabled = 0x1U;
using ch23 = core::field_descriptor<chen, 23U, 1U>;
inline constexpr std::uint32_t ch23_disabled = 0x0U;
inline constexpr std::uint32_t ch23_enabled = 0x1U;
using ch24 = core::field_descriptor<chen, 24U, 1U>;
inline constexpr std::uint32_t ch24_disabled = 0x0U;
inline constexpr std::uint32_t ch24_enabled = 0x1U;
using ch25 = core::field_descriptor<chen, 25U, 1U>;
inline constexpr std::uint32_t ch25_disabled = 0x0U;
inline constexpr std::uint32_t ch25_enabled = 0x1U;
using ch26 = core::field_descriptor<chen, 26U, 1U>;
inline constexpr std::uint32_t ch26_disabled = 0x0U;
inline constexpr std::uint32_t ch26_enabled = 0x1U;
using ch27 = core::field_descriptor<chen, 27U, 1U>;
inline constexpr std::uint32_t ch27_disabled = 0x0U;
inline constexpr std::uint32_t ch27_enabled = 0x1U;
using ch28 = core::field_descriptor<chen, 28U, 1U>;
inline constexpr std::uint32_t ch28_disabled = 0x0U;
inline constexpr std::uint32_t ch28_enabled = 0x1U;
using ch29 = core::field_descriptor<chen, 29U, 1U>;
inline constexpr std::uint32_t ch29_disabled = 0x0U;
inline constexpr std::uint32_t ch29_enabled = 0x1U;
using ch30 = core::field_descriptor<chen, 30U, 1U>;
inline constexpr std::uint32_t ch30_disabled = 0x0U;
inline constexpr std::uint32_t ch30_enabled = 0x1U;
using ch31 = core::field_descriptor<chen, 31U, 1U>;
inline constexpr std::uint32_t ch31_disabled = 0x0U;
inline constexpr std::uint32_t ch31_enabled = 0x1U;
}
struct chenset : core::register_descriptor<std::uint32_t, 0x504U,
    core::access_semantics::set_alias, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace chenset_fields {
using ch0 = core::field_descriptor<chenset, 0U, 1U>;
inline constexpr std::uint32_t ch0_disabled = 0x0U;
inline constexpr std::uint32_t ch0_enabled = 0x1U;
inline constexpr std::uint32_t ch0_set = 0x1U;
using ch1 = core::field_descriptor<chenset, 1U, 1U>;
inline constexpr std::uint32_t ch1_disabled = 0x0U;
inline constexpr std::uint32_t ch1_enabled = 0x1U;
inline constexpr std::uint32_t ch1_set = 0x1U;
using ch2 = core::field_descriptor<chenset, 2U, 1U>;
inline constexpr std::uint32_t ch2_disabled = 0x0U;
inline constexpr std::uint32_t ch2_enabled = 0x1U;
inline constexpr std::uint32_t ch2_set = 0x1U;
using ch3 = core::field_descriptor<chenset, 3U, 1U>;
inline constexpr std::uint32_t ch3_disabled = 0x0U;
inline constexpr std::uint32_t ch3_enabled = 0x1U;
inline constexpr std::uint32_t ch3_set = 0x1U;
using ch4 = core::field_descriptor<chenset, 4U, 1U>;
inline constexpr std::uint32_t ch4_disabled = 0x0U;
inline constexpr std::uint32_t ch4_enabled = 0x1U;
inline constexpr std::uint32_t ch4_set = 0x1U;
using ch5 = core::field_descriptor<chenset, 5U, 1U>;
inline constexpr std::uint32_t ch5_disabled = 0x0U;
inline constexpr std::uint32_t ch5_enabled = 0x1U;
inline constexpr std::uint32_t ch5_set = 0x1U;
using ch6 = core::field_descriptor<chenset, 6U, 1U>;
inline constexpr std::uint32_t ch6_disabled = 0x0U;
inline constexpr std::uint32_t ch6_enabled = 0x1U;
inline constexpr std::uint32_t ch6_set = 0x1U;
using ch7 = core::field_descriptor<chenset, 7U, 1U>;
inline constexpr std::uint32_t ch7_disabled = 0x0U;
inline constexpr std::uint32_t ch7_enabled = 0x1U;
inline constexpr std::uint32_t ch7_set = 0x1U;
using ch8 = core::field_descriptor<chenset, 8U, 1U>;
inline constexpr std::uint32_t ch8_disabled = 0x0U;
inline constexpr std::uint32_t ch8_enabled = 0x1U;
inline constexpr std::uint32_t ch8_set = 0x1U;
using ch9 = core::field_descriptor<chenset, 9U, 1U>;
inline constexpr std::uint32_t ch9_disabled = 0x0U;
inline constexpr std::uint32_t ch9_enabled = 0x1U;
inline constexpr std::uint32_t ch9_set = 0x1U;
using ch10 = core::field_descriptor<chenset, 10U, 1U>;
inline constexpr std::uint32_t ch10_disabled = 0x0U;
inline constexpr std::uint32_t ch10_enabled = 0x1U;
inline constexpr std::uint32_t ch10_set = 0x1U;
using ch11 = core::field_descriptor<chenset, 11U, 1U>;
inline constexpr std::uint32_t ch11_disabled = 0x0U;
inline constexpr std::uint32_t ch11_enabled = 0x1U;
inline constexpr std::uint32_t ch11_set = 0x1U;
using ch12 = core::field_descriptor<chenset, 12U, 1U>;
inline constexpr std::uint32_t ch12_disabled = 0x0U;
inline constexpr std::uint32_t ch12_enabled = 0x1U;
inline constexpr std::uint32_t ch12_set = 0x1U;
using ch13 = core::field_descriptor<chenset, 13U, 1U>;
inline constexpr std::uint32_t ch13_disabled = 0x0U;
inline constexpr std::uint32_t ch13_enabled = 0x1U;
inline constexpr std::uint32_t ch13_set = 0x1U;
using ch14 = core::field_descriptor<chenset, 14U, 1U>;
inline constexpr std::uint32_t ch14_disabled = 0x0U;
inline constexpr std::uint32_t ch14_enabled = 0x1U;
inline constexpr std::uint32_t ch14_set = 0x1U;
using ch15 = core::field_descriptor<chenset, 15U, 1U>;
inline constexpr std::uint32_t ch15_disabled = 0x0U;
inline constexpr std::uint32_t ch15_enabled = 0x1U;
inline constexpr std::uint32_t ch15_set = 0x1U;
using ch16 = core::field_descriptor<chenset, 16U, 1U>;
inline constexpr std::uint32_t ch16_disabled = 0x0U;
inline constexpr std::uint32_t ch16_enabled = 0x1U;
inline constexpr std::uint32_t ch16_set = 0x1U;
using ch17 = core::field_descriptor<chenset, 17U, 1U>;
inline constexpr std::uint32_t ch17_disabled = 0x0U;
inline constexpr std::uint32_t ch17_enabled = 0x1U;
inline constexpr std::uint32_t ch17_set = 0x1U;
using ch18 = core::field_descriptor<chenset, 18U, 1U>;
inline constexpr std::uint32_t ch18_disabled = 0x0U;
inline constexpr std::uint32_t ch18_enabled = 0x1U;
inline constexpr std::uint32_t ch18_set = 0x1U;
using ch19 = core::field_descriptor<chenset, 19U, 1U>;
inline constexpr std::uint32_t ch19_disabled = 0x0U;
inline constexpr std::uint32_t ch19_enabled = 0x1U;
inline constexpr std::uint32_t ch19_set = 0x1U;
using ch20 = core::field_descriptor<chenset, 20U, 1U>;
inline constexpr std::uint32_t ch20_disabled = 0x0U;
inline constexpr std::uint32_t ch20_enabled = 0x1U;
inline constexpr std::uint32_t ch20_set = 0x1U;
using ch21 = core::field_descriptor<chenset, 21U, 1U>;
inline constexpr std::uint32_t ch21_disabled = 0x0U;
inline constexpr std::uint32_t ch21_enabled = 0x1U;
inline constexpr std::uint32_t ch21_set = 0x1U;
using ch22 = core::field_descriptor<chenset, 22U, 1U>;
inline constexpr std::uint32_t ch22_disabled = 0x0U;
inline constexpr std::uint32_t ch22_enabled = 0x1U;
inline constexpr std::uint32_t ch22_set = 0x1U;
using ch23 = core::field_descriptor<chenset, 23U, 1U>;
inline constexpr std::uint32_t ch23_disabled = 0x0U;
inline constexpr std::uint32_t ch23_enabled = 0x1U;
inline constexpr std::uint32_t ch23_set = 0x1U;
using ch24 = core::field_descriptor<chenset, 24U, 1U>;
inline constexpr std::uint32_t ch24_disabled = 0x0U;
inline constexpr std::uint32_t ch24_enabled = 0x1U;
inline constexpr std::uint32_t ch24_set = 0x1U;
using ch25 = core::field_descriptor<chenset, 25U, 1U>;
inline constexpr std::uint32_t ch25_disabled = 0x0U;
inline constexpr std::uint32_t ch25_enabled = 0x1U;
inline constexpr std::uint32_t ch25_set = 0x1U;
using ch26 = core::field_descriptor<chenset, 26U, 1U>;
inline constexpr std::uint32_t ch26_disabled = 0x0U;
inline constexpr std::uint32_t ch26_enabled = 0x1U;
inline constexpr std::uint32_t ch26_set = 0x1U;
using ch27 = core::field_descriptor<chenset, 27U, 1U>;
inline constexpr std::uint32_t ch27_disabled = 0x0U;
inline constexpr std::uint32_t ch27_enabled = 0x1U;
inline constexpr std::uint32_t ch27_set = 0x1U;
using ch28 = core::field_descriptor<chenset, 28U, 1U>;
inline constexpr std::uint32_t ch28_disabled = 0x0U;
inline constexpr std::uint32_t ch28_enabled = 0x1U;
inline constexpr std::uint32_t ch28_set = 0x1U;
using ch29 = core::field_descriptor<chenset, 29U, 1U>;
inline constexpr std::uint32_t ch29_disabled = 0x0U;
inline constexpr std::uint32_t ch29_enabled = 0x1U;
inline constexpr std::uint32_t ch29_set = 0x1U;
using ch30 = core::field_descriptor<chenset, 30U, 1U>;
inline constexpr std::uint32_t ch30_disabled = 0x0U;
inline constexpr std::uint32_t ch30_enabled = 0x1U;
inline constexpr std::uint32_t ch30_set = 0x1U;
using ch31 = core::field_descriptor<chenset, 31U, 1U>;
inline constexpr std::uint32_t ch31_disabled = 0x0U;
inline constexpr std::uint32_t ch31_enabled = 0x1U;
inline constexpr std::uint32_t ch31_set = 0x1U;
}
struct chenclr : core::register_descriptor<std::uint32_t, 0x508U,
    core::access_semantics::clear_alias, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace chenclr_fields {
using ch0 = core::field_descriptor<chenclr, 0U, 1U>;
inline constexpr std::uint32_t ch0_disabled = 0x0U;
inline constexpr std::uint32_t ch0_enabled = 0x1U;
inline constexpr std::uint32_t ch0_clear = 0x1U;
using ch1 = core::field_descriptor<chenclr, 1U, 1U>;
inline constexpr std::uint32_t ch1_disabled = 0x0U;
inline constexpr std::uint32_t ch1_enabled = 0x1U;
inline constexpr std::uint32_t ch1_clear = 0x1U;
using ch2 = core::field_descriptor<chenclr, 2U, 1U>;
inline constexpr std::uint32_t ch2_disabled = 0x0U;
inline constexpr std::uint32_t ch2_enabled = 0x1U;
inline constexpr std::uint32_t ch2_clear = 0x1U;
using ch3 = core::field_descriptor<chenclr, 3U, 1U>;
inline constexpr std::uint32_t ch3_disabled = 0x0U;
inline constexpr std::uint32_t ch3_enabled = 0x1U;
inline constexpr std::uint32_t ch3_clear = 0x1U;
using ch4 = core::field_descriptor<chenclr, 4U, 1U>;
inline constexpr std::uint32_t ch4_disabled = 0x0U;
inline constexpr std::uint32_t ch4_enabled = 0x1U;
inline constexpr std::uint32_t ch4_clear = 0x1U;
using ch5 = core::field_descriptor<chenclr, 5U, 1U>;
inline constexpr std::uint32_t ch5_disabled = 0x0U;
inline constexpr std::uint32_t ch5_enabled = 0x1U;
inline constexpr std::uint32_t ch5_clear = 0x1U;
using ch6 = core::field_descriptor<chenclr, 6U, 1U>;
inline constexpr std::uint32_t ch6_disabled = 0x0U;
inline constexpr std::uint32_t ch6_enabled = 0x1U;
inline constexpr std::uint32_t ch6_clear = 0x1U;
using ch7 = core::field_descriptor<chenclr, 7U, 1U>;
inline constexpr std::uint32_t ch7_disabled = 0x0U;
inline constexpr std::uint32_t ch7_enabled = 0x1U;
inline constexpr std::uint32_t ch7_clear = 0x1U;
using ch8 = core::field_descriptor<chenclr, 8U, 1U>;
inline constexpr std::uint32_t ch8_disabled = 0x0U;
inline constexpr std::uint32_t ch8_enabled = 0x1U;
inline constexpr std::uint32_t ch8_clear = 0x1U;
using ch9 = core::field_descriptor<chenclr, 9U, 1U>;
inline constexpr std::uint32_t ch9_disabled = 0x0U;
inline constexpr std::uint32_t ch9_enabled = 0x1U;
inline constexpr std::uint32_t ch9_clear = 0x1U;
using ch10 = core::field_descriptor<chenclr, 10U, 1U>;
inline constexpr std::uint32_t ch10_disabled = 0x0U;
inline constexpr std::uint32_t ch10_enabled = 0x1U;
inline constexpr std::uint32_t ch10_clear = 0x1U;
using ch11 = core::field_descriptor<chenclr, 11U, 1U>;
inline constexpr std::uint32_t ch11_disabled = 0x0U;
inline constexpr std::uint32_t ch11_enabled = 0x1U;
inline constexpr std::uint32_t ch11_clear = 0x1U;
using ch12 = core::field_descriptor<chenclr, 12U, 1U>;
inline constexpr std::uint32_t ch12_disabled = 0x0U;
inline constexpr std::uint32_t ch12_enabled = 0x1U;
inline constexpr std::uint32_t ch12_clear = 0x1U;
using ch13 = core::field_descriptor<chenclr, 13U, 1U>;
inline constexpr std::uint32_t ch13_disabled = 0x0U;
inline constexpr std::uint32_t ch13_enabled = 0x1U;
inline constexpr std::uint32_t ch13_clear = 0x1U;
using ch14 = core::field_descriptor<chenclr, 14U, 1U>;
inline constexpr std::uint32_t ch14_disabled = 0x0U;
inline constexpr std::uint32_t ch14_enabled = 0x1U;
inline constexpr std::uint32_t ch14_clear = 0x1U;
using ch15 = core::field_descriptor<chenclr, 15U, 1U>;
inline constexpr std::uint32_t ch15_disabled = 0x0U;
inline constexpr std::uint32_t ch15_enabled = 0x1U;
inline constexpr std::uint32_t ch15_clear = 0x1U;
using ch16 = core::field_descriptor<chenclr, 16U, 1U>;
inline constexpr std::uint32_t ch16_disabled = 0x0U;
inline constexpr std::uint32_t ch16_enabled = 0x1U;
inline constexpr std::uint32_t ch16_clear = 0x1U;
using ch17 = core::field_descriptor<chenclr, 17U, 1U>;
inline constexpr std::uint32_t ch17_disabled = 0x0U;
inline constexpr std::uint32_t ch17_enabled = 0x1U;
inline constexpr std::uint32_t ch17_clear = 0x1U;
using ch18 = core::field_descriptor<chenclr, 18U, 1U>;
inline constexpr std::uint32_t ch18_disabled = 0x0U;
inline constexpr std::uint32_t ch18_enabled = 0x1U;
inline constexpr std::uint32_t ch18_clear = 0x1U;
using ch19 = core::field_descriptor<chenclr, 19U, 1U>;
inline constexpr std::uint32_t ch19_disabled = 0x0U;
inline constexpr std::uint32_t ch19_enabled = 0x1U;
inline constexpr std::uint32_t ch19_clear = 0x1U;
using ch20 = core::field_descriptor<chenclr, 20U, 1U>;
inline constexpr std::uint32_t ch20_disabled = 0x0U;
inline constexpr std::uint32_t ch20_enabled = 0x1U;
inline constexpr std::uint32_t ch20_clear = 0x1U;
using ch21 = core::field_descriptor<chenclr, 21U, 1U>;
inline constexpr std::uint32_t ch21_disabled = 0x0U;
inline constexpr std::uint32_t ch21_enabled = 0x1U;
inline constexpr std::uint32_t ch21_clear = 0x1U;
using ch22 = core::field_descriptor<chenclr, 22U, 1U>;
inline constexpr std::uint32_t ch22_disabled = 0x0U;
inline constexpr std::uint32_t ch22_enabled = 0x1U;
inline constexpr std::uint32_t ch22_clear = 0x1U;
using ch23 = core::field_descriptor<chenclr, 23U, 1U>;
inline constexpr std::uint32_t ch23_disabled = 0x0U;
inline constexpr std::uint32_t ch23_enabled = 0x1U;
inline constexpr std::uint32_t ch23_clear = 0x1U;
using ch24 = core::field_descriptor<chenclr, 24U, 1U>;
inline constexpr std::uint32_t ch24_disabled = 0x0U;
inline constexpr std::uint32_t ch24_enabled = 0x1U;
inline constexpr std::uint32_t ch24_clear = 0x1U;
using ch25 = core::field_descriptor<chenclr, 25U, 1U>;
inline constexpr std::uint32_t ch25_disabled = 0x0U;
inline constexpr std::uint32_t ch25_enabled = 0x1U;
inline constexpr std::uint32_t ch25_clear = 0x1U;
using ch26 = core::field_descriptor<chenclr, 26U, 1U>;
inline constexpr std::uint32_t ch26_disabled = 0x0U;
inline constexpr std::uint32_t ch26_enabled = 0x1U;
inline constexpr std::uint32_t ch26_clear = 0x1U;
using ch27 = core::field_descriptor<chenclr, 27U, 1U>;
inline constexpr std::uint32_t ch27_disabled = 0x0U;
inline constexpr std::uint32_t ch27_enabled = 0x1U;
inline constexpr std::uint32_t ch27_clear = 0x1U;
using ch28 = core::field_descriptor<chenclr, 28U, 1U>;
inline constexpr std::uint32_t ch28_disabled = 0x0U;
inline constexpr std::uint32_t ch28_enabled = 0x1U;
inline constexpr std::uint32_t ch28_clear = 0x1U;
using ch29 = core::field_descriptor<chenclr, 29U, 1U>;
inline constexpr std::uint32_t ch29_disabled = 0x0U;
inline constexpr std::uint32_t ch29_enabled = 0x1U;
inline constexpr std::uint32_t ch29_clear = 0x1U;
using ch30 = core::field_descriptor<chenclr, 30U, 1U>;
inline constexpr std::uint32_t ch30_disabled = 0x0U;
inline constexpr std::uint32_t ch30_enabled = 0x1U;
inline constexpr std::uint32_t ch30_clear = 0x1U;
using ch31 = core::field_descriptor<chenclr, 31U, 1U>;
inline constexpr std::uint32_t ch31_disabled = 0x0U;
inline constexpr std::uint32_t ch31_enabled = 0x1U;
inline constexpr std::uint32_t ch31_clear = 0x1U;
}
struct ch0_eep : core::register_descriptor<std::uint32_t, 0x510U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace ch0_eep_fields {
using eep = core::field_descriptor<ch0_eep, 0U, 32U>;
}
struct ch0_tep : core::register_descriptor<std::uint32_t, 0x514U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace ch0_tep_fields {
using tep = core::field_descriptor<ch0_tep, 0U, 32U>;
}
struct ch1_eep : core::register_descriptor<std::uint32_t, 0x518U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace ch1_eep_fields {
using eep = core::field_descriptor<ch1_eep, 0U, 32U>;
}
struct ch1_tep : core::register_descriptor<std::uint32_t, 0x51CU,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace ch1_tep_fields {
using tep = core::field_descriptor<ch1_tep, 0U, 32U>;
}
struct ch2_eep : core::register_descriptor<std::uint32_t, 0x520U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace ch2_eep_fields {
using eep = core::field_descriptor<ch2_eep, 0U, 32U>;
}
struct ch2_tep : core::register_descriptor<std::uint32_t, 0x524U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace ch2_tep_fields {
using tep = core::field_descriptor<ch2_tep, 0U, 32U>;
}
struct ch3_eep : core::register_descriptor<std::uint32_t, 0x528U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace ch3_eep_fields {
using eep = core::field_descriptor<ch3_eep, 0U, 32U>;
}
struct ch3_tep : core::register_descriptor<std::uint32_t, 0x52CU,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace ch3_tep_fields {
using tep = core::field_descriptor<ch3_tep, 0U, 32U>;
}
struct ch4_eep : core::register_descriptor<std::uint32_t, 0x530U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace ch4_eep_fields {
using eep = core::field_descriptor<ch4_eep, 0U, 32U>;
}
struct ch4_tep : core::register_descriptor<std::uint32_t, 0x534U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace ch4_tep_fields {
using tep = core::field_descriptor<ch4_tep, 0U, 32U>;
}
struct ch5_eep : core::register_descriptor<std::uint32_t, 0x538U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace ch5_eep_fields {
using eep = core::field_descriptor<ch5_eep, 0U, 32U>;
}
struct ch5_tep : core::register_descriptor<std::uint32_t, 0x53CU,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace ch5_tep_fields {
using tep = core::field_descriptor<ch5_tep, 0U, 32U>;
}
struct ch6_eep : core::register_descriptor<std::uint32_t, 0x540U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace ch6_eep_fields {
using eep = core::field_descriptor<ch6_eep, 0U, 32U>;
}
struct ch6_tep : core::register_descriptor<std::uint32_t, 0x544U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace ch6_tep_fields {
using tep = core::field_descriptor<ch6_tep, 0U, 32U>;
}
struct ch7_eep : core::register_descriptor<std::uint32_t, 0x548U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace ch7_eep_fields {
using eep = core::field_descriptor<ch7_eep, 0U, 32U>;
}
struct ch7_tep : core::register_descriptor<std::uint32_t, 0x54CU,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace ch7_tep_fields {
using tep = core::field_descriptor<ch7_tep, 0U, 32U>;
}
struct ch8_eep : core::register_descriptor<std::uint32_t, 0x550U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace ch8_eep_fields {
using eep = core::field_descriptor<ch8_eep, 0U, 32U>;
}
struct ch8_tep : core::register_descriptor<std::uint32_t, 0x554U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace ch8_tep_fields {
using tep = core::field_descriptor<ch8_tep, 0U, 32U>;
}
struct ch9_eep : core::register_descriptor<std::uint32_t, 0x558U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace ch9_eep_fields {
using eep = core::field_descriptor<ch9_eep, 0U, 32U>;
}
struct ch9_tep : core::register_descriptor<std::uint32_t, 0x55CU,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace ch9_tep_fields {
using tep = core::field_descriptor<ch9_tep, 0U, 32U>;
}
struct ch10_eep : core::register_descriptor<std::uint32_t, 0x560U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace ch10_eep_fields {
using eep = core::field_descriptor<ch10_eep, 0U, 32U>;
}
struct ch10_tep : core::register_descriptor<std::uint32_t, 0x564U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace ch10_tep_fields {
using tep = core::field_descriptor<ch10_tep, 0U, 32U>;
}
struct ch11_eep : core::register_descriptor<std::uint32_t, 0x568U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace ch11_eep_fields {
using eep = core::field_descriptor<ch11_eep, 0U, 32U>;
}
struct ch11_tep : core::register_descriptor<std::uint32_t, 0x56CU,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace ch11_tep_fields {
using tep = core::field_descriptor<ch11_tep, 0U, 32U>;
}
struct ch12_eep : core::register_descriptor<std::uint32_t, 0x570U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace ch12_eep_fields {
using eep = core::field_descriptor<ch12_eep, 0U, 32U>;
}
struct ch12_tep : core::register_descriptor<std::uint32_t, 0x574U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace ch12_tep_fields {
using tep = core::field_descriptor<ch12_tep, 0U, 32U>;
}
struct ch13_eep : core::register_descriptor<std::uint32_t, 0x578U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace ch13_eep_fields {
using eep = core::field_descriptor<ch13_eep, 0U, 32U>;
}
struct ch13_tep : core::register_descriptor<std::uint32_t, 0x57CU,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace ch13_tep_fields {
using tep = core::field_descriptor<ch13_tep, 0U, 32U>;
}
struct ch14_eep : core::register_descriptor<std::uint32_t, 0x580U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace ch14_eep_fields {
using eep = core::field_descriptor<ch14_eep, 0U, 32U>;
}
struct ch14_tep : core::register_descriptor<std::uint32_t, 0x584U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace ch14_tep_fields {
using tep = core::field_descriptor<ch14_tep, 0U, 32U>;
}
struct ch15_eep : core::register_descriptor<std::uint32_t, 0x588U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace ch15_eep_fields {
using eep = core::field_descriptor<ch15_eep, 0U, 32U>;
}
struct ch15_tep : core::register_descriptor<std::uint32_t, 0x58CU,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace ch15_tep_fields {
using tep = core::field_descriptor<ch15_tep, 0U, 32U>;
}
struct ch16_eep : core::register_descriptor<std::uint32_t, 0x590U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace ch16_eep_fields {
using eep = core::field_descriptor<ch16_eep, 0U, 32U>;
}
struct ch16_tep : core::register_descriptor<std::uint32_t, 0x594U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace ch16_tep_fields {
using tep = core::field_descriptor<ch16_tep, 0U, 32U>;
}
struct ch17_eep : core::register_descriptor<std::uint32_t, 0x598U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace ch17_eep_fields {
using eep = core::field_descriptor<ch17_eep, 0U, 32U>;
}
struct ch17_tep : core::register_descriptor<std::uint32_t, 0x59CU,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace ch17_tep_fields {
using tep = core::field_descriptor<ch17_tep, 0U, 32U>;
}
struct ch18_eep : core::register_descriptor<std::uint32_t, 0x5A0U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace ch18_eep_fields {
using eep = core::field_descriptor<ch18_eep, 0U, 32U>;
}
struct ch18_tep : core::register_descriptor<std::uint32_t, 0x5A4U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace ch18_tep_fields {
using tep = core::field_descriptor<ch18_tep, 0U, 32U>;
}
struct ch19_eep : core::register_descriptor<std::uint32_t, 0x5A8U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace ch19_eep_fields {
using eep = core::field_descriptor<ch19_eep, 0U, 32U>;
}
struct ch19_tep : core::register_descriptor<std::uint32_t, 0x5ACU,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace ch19_tep_fields {
using tep = core::field_descriptor<ch19_tep, 0U, 32U>;
}
struct chg0 : core::register_descriptor<std::uint32_t, 0x800U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace chg0_fields {
using ch0 = core::field_descriptor<chg0, 0U, 1U>;
inline constexpr std::uint32_t ch0_excluded = 0x0U;
inline constexpr std::uint32_t ch0_included = 0x1U;
using ch1 = core::field_descriptor<chg0, 1U, 1U>;
inline constexpr std::uint32_t ch1_excluded = 0x0U;
inline constexpr std::uint32_t ch1_included = 0x1U;
using ch2 = core::field_descriptor<chg0, 2U, 1U>;
inline constexpr std::uint32_t ch2_excluded = 0x0U;
inline constexpr std::uint32_t ch2_included = 0x1U;
using ch3 = core::field_descriptor<chg0, 3U, 1U>;
inline constexpr std::uint32_t ch3_excluded = 0x0U;
inline constexpr std::uint32_t ch3_included = 0x1U;
using ch4 = core::field_descriptor<chg0, 4U, 1U>;
inline constexpr std::uint32_t ch4_excluded = 0x0U;
inline constexpr std::uint32_t ch4_included = 0x1U;
using ch5 = core::field_descriptor<chg0, 5U, 1U>;
inline constexpr std::uint32_t ch5_excluded = 0x0U;
inline constexpr std::uint32_t ch5_included = 0x1U;
using ch6 = core::field_descriptor<chg0, 6U, 1U>;
inline constexpr std::uint32_t ch6_excluded = 0x0U;
inline constexpr std::uint32_t ch6_included = 0x1U;
using ch7 = core::field_descriptor<chg0, 7U, 1U>;
inline constexpr std::uint32_t ch7_excluded = 0x0U;
inline constexpr std::uint32_t ch7_included = 0x1U;
using ch8 = core::field_descriptor<chg0, 8U, 1U>;
inline constexpr std::uint32_t ch8_excluded = 0x0U;
inline constexpr std::uint32_t ch8_included = 0x1U;
using ch9 = core::field_descriptor<chg0, 9U, 1U>;
inline constexpr std::uint32_t ch9_excluded = 0x0U;
inline constexpr std::uint32_t ch9_included = 0x1U;
using ch10 = core::field_descriptor<chg0, 10U, 1U>;
inline constexpr std::uint32_t ch10_excluded = 0x0U;
inline constexpr std::uint32_t ch10_included = 0x1U;
using ch11 = core::field_descriptor<chg0, 11U, 1U>;
inline constexpr std::uint32_t ch11_excluded = 0x0U;
inline constexpr std::uint32_t ch11_included = 0x1U;
using ch12 = core::field_descriptor<chg0, 12U, 1U>;
inline constexpr std::uint32_t ch12_excluded = 0x0U;
inline constexpr std::uint32_t ch12_included = 0x1U;
using ch13 = core::field_descriptor<chg0, 13U, 1U>;
inline constexpr std::uint32_t ch13_excluded = 0x0U;
inline constexpr std::uint32_t ch13_included = 0x1U;
using ch14 = core::field_descriptor<chg0, 14U, 1U>;
inline constexpr std::uint32_t ch14_excluded = 0x0U;
inline constexpr std::uint32_t ch14_included = 0x1U;
using ch15 = core::field_descriptor<chg0, 15U, 1U>;
inline constexpr std::uint32_t ch15_excluded = 0x0U;
inline constexpr std::uint32_t ch15_included = 0x1U;
using ch16 = core::field_descriptor<chg0, 16U, 1U>;
inline constexpr std::uint32_t ch16_excluded = 0x0U;
inline constexpr std::uint32_t ch16_included = 0x1U;
using ch17 = core::field_descriptor<chg0, 17U, 1U>;
inline constexpr std::uint32_t ch17_excluded = 0x0U;
inline constexpr std::uint32_t ch17_included = 0x1U;
using ch18 = core::field_descriptor<chg0, 18U, 1U>;
inline constexpr std::uint32_t ch18_excluded = 0x0U;
inline constexpr std::uint32_t ch18_included = 0x1U;
using ch19 = core::field_descriptor<chg0, 19U, 1U>;
inline constexpr std::uint32_t ch19_excluded = 0x0U;
inline constexpr std::uint32_t ch19_included = 0x1U;
using ch20 = core::field_descriptor<chg0, 20U, 1U>;
inline constexpr std::uint32_t ch20_excluded = 0x0U;
inline constexpr std::uint32_t ch20_included = 0x1U;
using ch21 = core::field_descriptor<chg0, 21U, 1U>;
inline constexpr std::uint32_t ch21_excluded = 0x0U;
inline constexpr std::uint32_t ch21_included = 0x1U;
using ch22 = core::field_descriptor<chg0, 22U, 1U>;
inline constexpr std::uint32_t ch22_excluded = 0x0U;
inline constexpr std::uint32_t ch22_included = 0x1U;
using ch23 = core::field_descriptor<chg0, 23U, 1U>;
inline constexpr std::uint32_t ch23_excluded = 0x0U;
inline constexpr std::uint32_t ch23_included = 0x1U;
using ch24 = core::field_descriptor<chg0, 24U, 1U>;
inline constexpr std::uint32_t ch24_excluded = 0x0U;
inline constexpr std::uint32_t ch24_included = 0x1U;
using ch25 = core::field_descriptor<chg0, 25U, 1U>;
inline constexpr std::uint32_t ch25_excluded = 0x0U;
inline constexpr std::uint32_t ch25_included = 0x1U;
using ch26 = core::field_descriptor<chg0, 26U, 1U>;
inline constexpr std::uint32_t ch26_excluded = 0x0U;
inline constexpr std::uint32_t ch26_included = 0x1U;
using ch27 = core::field_descriptor<chg0, 27U, 1U>;
inline constexpr std::uint32_t ch27_excluded = 0x0U;
inline constexpr std::uint32_t ch27_included = 0x1U;
using ch28 = core::field_descriptor<chg0, 28U, 1U>;
inline constexpr std::uint32_t ch28_excluded = 0x0U;
inline constexpr std::uint32_t ch28_included = 0x1U;
using ch29 = core::field_descriptor<chg0, 29U, 1U>;
inline constexpr std::uint32_t ch29_excluded = 0x0U;
inline constexpr std::uint32_t ch29_included = 0x1U;
using ch30 = core::field_descriptor<chg0, 30U, 1U>;
inline constexpr std::uint32_t ch30_excluded = 0x0U;
inline constexpr std::uint32_t ch30_included = 0x1U;
using ch31 = core::field_descriptor<chg0, 31U, 1U>;
inline constexpr std::uint32_t ch31_excluded = 0x0U;
inline constexpr std::uint32_t ch31_included = 0x1U;
}
struct chg1 : core::register_descriptor<std::uint32_t, 0x804U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace chg1_fields {
using ch0 = core::field_descriptor<chg1, 0U, 1U>;
inline constexpr std::uint32_t ch0_excluded = 0x0U;
inline constexpr std::uint32_t ch0_included = 0x1U;
using ch1 = core::field_descriptor<chg1, 1U, 1U>;
inline constexpr std::uint32_t ch1_excluded = 0x0U;
inline constexpr std::uint32_t ch1_included = 0x1U;
using ch2 = core::field_descriptor<chg1, 2U, 1U>;
inline constexpr std::uint32_t ch2_excluded = 0x0U;
inline constexpr std::uint32_t ch2_included = 0x1U;
using ch3 = core::field_descriptor<chg1, 3U, 1U>;
inline constexpr std::uint32_t ch3_excluded = 0x0U;
inline constexpr std::uint32_t ch3_included = 0x1U;
using ch4 = core::field_descriptor<chg1, 4U, 1U>;
inline constexpr std::uint32_t ch4_excluded = 0x0U;
inline constexpr std::uint32_t ch4_included = 0x1U;
using ch5 = core::field_descriptor<chg1, 5U, 1U>;
inline constexpr std::uint32_t ch5_excluded = 0x0U;
inline constexpr std::uint32_t ch5_included = 0x1U;
using ch6 = core::field_descriptor<chg1, 6U, 1U>;
inline constexpr std::uint32_t ch6_excluded = 0x0U;
inline constexpr std::uint32_t ch6_included = 0x1U;
using ch7 = core::field_descriptor<chg1, 7U, 1U>;
inline constexpr std::uint32_t ch7_excluded = 0x0U;
inline constexpr std::uint32_t ch7_included = 0x1U;
using ch8 = core::field_descriptor<chg1, 8U, 1U>;
inline constexpr std::uint32_t ch8_excluded = 0x0U;
inline constexpr std::uint32_t ch8_included = 0x1U;
using ch9 = core::field_descriptor<chg1, 9U, 1U>;
inline constexpr std::uint32_t ch9_excluded = 0x0U;
inline constexpr std::uint32_t ch9_included = 0x1U;
using ch10 = core::field_descriptor<chg1, 10U, 1U>;
inline constexpr std::uint32_t ch10_excluded = 0x0U;
inline constexpr std::uint32_t ch10_included = 0x1U;
using ch11 = core::field_descriptor<chg1, 11U, 1U>;
inline constexpr std::uint32_t ch11_excluded = 0x0U;
inline constexpr std::uint32_t ch11_included = 0x1U;
using ch12 = core::field_descriptor<chg1, 12U, 1U>;
inline constexpr std::uint32_t ch12_excluded = 0x0U;
inline constexpr std::uint32_t ch12_included = 0x1U;
using ch13 = core::field_descriptor<chg1, 13U, 1U>;
inline constexpr std::uint32_t ch13_excluded = 0x0U;
inline constexpr std::uint32_t ch13_included = 0x1U;
using ch14 = core::field_descriptor<chg1, 14U, 1U>;
inline constexpr std::uint32_t ch14_excluded = 0x0U;
inline constexpr std::uint32_t ch14_included = 0x1U;
using ch15 = core::field_descriptor<chg1, 15U, 1U>;
inline constexpr std::uint32_t ch15_excluded = 0x0U;
inline constexpr std::uint32_t ch15_included = 0x1U;
using ch16 = core::field_descriptor<chg1, 16U, 1U>;
inline constexpr std::uint32_t ch16_excluded = 0x0U;
inline constexpr std::uint32_t ch16_included = 0x1U;
using ch17 = core::field_descriptor<chg1, 17U, 1U>;
inline constexpr std::uint32_t ch17_excluded = 0x0U;
inline constexpr std::uint32_t ch17_included = 0x1U;
using ch18 = core::field_descriptor<chg1, 18U, 1U>;
inline constexpr std::uint32_t ch18_excluded = 0x0U;
inline constexpr std::uint32_t ch18_included = 0x1U;
using ch19 = core::field_descriptor<chg1, 19U, 1U>;
inline constexpr std::uint32_t ch19_excluded = 0x0U;
inline constexpr std::uint32_t ch19_included = 0x1U;
using ch20 = core::field_descriptor<chg1, 20U, 1U>;
inline constexpr std::uint32_t ch20_excluded = 0x0U;
inline constexpr std::uint32_t ch20_included = 0x1U;
using ch21 = core::field_descriptor<chg1, 21U, 1U>;
inline constexpr std::uint32_t ch21_excluded = 0x0U;
inline constexpr std::uint32_t ch21_included = 0x1U;
using ch22 = core::field_descriptor<chg1, 22U, 1U>;
inline constexpr std::uint32_t ch22_excluded = 0x0U;
inline constexpr std::uint32_t ch22_included = 0x1U;
using ch23 = core::field_descriptor<chg1, 23U, 1U>;
inline constexpr std::uint32_t ch23_excluded = 0x0U;
inline constexpr std::uint32_t ch23_included = 0x1U;
using ch24 = core::field_descriptor<chg1, 24U, 1U>;
inline constexpr std::uint32_t ch24_excluded = 0x0U;
inline constexpr std::uint32_t ch24_included = 0x1U;
using ch25 = core::field_descriptor<chg1, 25U, 1U>;
inline constexpr std::uint32_t ch25_excluded = 0x0U;
inline constexpr std::uint32_t ch25_included = 0x1U;
using ch26 = core::field_descriptor<chg1, 26U, 1U>;
inline constexpr std::uint32_t ch26_excluded = 0x0U;
inline constexpr std::uint32_t ch26_included = 0x1U;
using ch27 = core::field_descriptor<chg1, 27U, 1U>;
inline constexpr std::uint32_t ch27_excluded = 0x0U;
inline constexpr std::uint32_t ch27_included = 0x1U;
using ch28 = core::field_descriptor<chg1, 28U, 1U>;
inline constexpr std::uint32_t ch28_excluded = 0x0U;
inline constexpr std::uint32_t ch28_included = 0x1U;
using ch29 = core::field_descriptor<chg1, 29U, 1U>;
inline constexpr std::uint32_t ch29_excluded = 0x0U;
inline constexpr std::uint32_t ch29_included = 0x1U;
using ch30 = core::field_descriptor<chg1, 30U, 1U>;
inline constexpr std::uint32_t ch30_excluded = 0x0U;
inline constexpr std::uint32_t ch30_included = 0x1U;
using ch31 = core::field_descriptor<chg1, 31U, 1U>;
inline constexpr std::uint32_t ch31_excluded = 0x0U;
inline constexpr std::uint32_t ch31_included = 0x1U;
}
struct chg2 : core::register_descriptor<std::uint32_t, 0x808U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace chg2_fields {
using ch0 = core::field_descriptor<chg2, 0U, 1U>;
inline constexpr std::uint32_t ch0_excluded = 0x0U;
inline constexpr std::uint32_t ch0_included = 0x1U;
using ch1 = core::field_descriptor<chg2, 1U, 1U>;
inline constexpr std::uint32_t ch1_excluded = 0x0U;
inline constexpr std::uint32_t ch1_included = 0x1U;
using ch2 = core::field_descriptor<chg2, 2U, 1U>;
inline constexpr std::uint32_t ch2_excluded = 0x0U;
inline constexpr std::uint32_t ch2_included = 0x1U;
using ch3 = core::field_descriptor<chg2, 3U, 1U>;
inline constexpr std::uint32_t ch3_excluded = 0x0U;
inline constexpr std::uint32_t ch3_included = 0x1U;
using ch4 = core::field_descriptor<chg2, 4U, 1U>;
inline constexpr std::uint32_t ch4_excluded = 0x0U;
inline constexpr std::uint32_t ch4_included = 0x1U;
using ch5 = core::field_descriptor<chg2, 5U, 1U>;
inline constexpr std::uint32_t ch5_excluded = 0x0U;
inline constexpr std::uint32_t ch5_included = 0x1U;
using ch6 = core::field_descriptor<chg2, 6U, 1U>;
inline constexpr std::uint32_t ch6_excluded = 0x0U;
inline constexpr std::uint32_t ch6_included = 0x1U;
using ch7 = core::field_descriptor<chg2, 7U, 1U>;
inline constexpr std::uint32_t ch7_excluded = 0x0U;
inline constexpr std::uint32_t ch7_included = 0x1U;
using ch8 = core::field_descriptor<chg2, 8U, 1U>;
inline constexpr std::uint32_t ch8_excluded = 0x0U;
inline constexpr std::uint32_t ch8_included = 0x1U;
using ch9 = core::field_descriptor<chg2, 9U, 1U>;
inline constexpr std::uint32_t ch9_excluded = 0x0U;
inline constexpr std::uint32_t ch9_included = 0x1U;
using ch10 = core::field_descriptor<chg2, 10U, 1U>;
inline constexpr std::uint32_t ch10_excluded = 0x0U;
inline constexpr std::uint32_t ch10_included = 0x1U;
using ch11 = core::field_descriptor<chg2, 11U, 1U>;
inline constexpr std::uint32_t ch11_excluded = 0x0U;
inline constexpr std::uint32_t ch11_included = 0x1U;
using ch12 = core::field_descriptor<chg2, 12U, 1U>;
inline constexpr std::uint32_t ch12_excluded = 0x0U;
inline constexpr std::uint32_t ch12_included = 0x1U;
using ch13 = core::field_descriptor<chg2, 13U, 1U>;
inline constexpr std::uint32_t ch13_excluded = 0x0U;
inline constexpr std::uint32_t ch13_included = 0x1U;
using ch14 = core::field_descriptor<chg2, 14U, 1U>;
inline constexpr std::uint32_t ch14_excluded = 0x0U;
inline constexpr std::uint32_t ch14_included = 0x1U;
using ch15 = core::field_descriptor<chg2, 15U, 1U>;
inline constexpr std::uint32_t ch15_excluded = 0x0U;
inline constexpr std::uint32_t ch15_included = 0x1U;
using ch16 = core::field_descriptor<chg2, 16U, 1U>;
inline constexpr std::uint32_t ch16_excluded = 0x0U;
inline constexpr std::uint32_t ch16_included = 0x1U;
using ch17 = core::field_descriptor<chg2, 17U, 1U>;
inline constexpr std::uint32_t ch17_excluded = 0x0U;
inline constexpr std::uint32_t ch17_included = 0x1U;
using ch18 = core::field_descriptor<chg2, 18U, 1U>;
inline constexpr std::uint32_t ch18_excluded = 0x0U;
inline constexpr std::uint32_t ch18_included = 0x1U;
using ch19 = core::field_descriptor<chg2, 19U, 1U>;
inline constexpr std::uint32_t ch19_excluded = 0x0U;
inline constexpr std::uint32_t ch19_included = 0x1U;
using ch20 = core::field_descriptor<chg2, 20U, 1U>;
inline constexpr std::uint32_t ch20_excluded = 0x0U;
inline constexpr std::uint32_t ch20_included = 0x1U;
using ch21 = core::field_descriptor<chg2, 21U, 1U>;
inline constexpr std::uint32_t ch21_excluded = 0x0U;
inline constexpr std::uint32_t ch21_included = 0x1U;
using ch22 = core::field_descriptor<chg2, 22U, 1U>;
inline constexpr std::uint32_t ch22_excluded = 0x0U;
inline constexpr std::uint32_t ch22_included = 0x1U;
using ch23 = core::field_descriptor<chg2, 23U, 1U>;
inline constexpr std::uint32_t ch23_excluded = 0x0U;
inline constexpr std::uint32_t ch23_included = 0x1U;
using ch24 = core::field_descriptor<chg2, 24U, 1U>;
inline constexpr std::uint32_t ch24_excluded = 0x0U;
inline constexpr std::uint32_t ch24_included = 0x1U;
using ch25 = core::field_descriptor<chg2, 25U, 1U>;
inline constexpr std::uint32_t ch25_excluded = 0x0U;
inline constexpr std::uint32_t ch25_included = 0x1U;
using ch26 = core::field_descriptor<chg2, 26U, 1U>;
inline constexpr std::uint32_t ch26_excluded = 0x0U;
inline constexpr std::uint32_t ch26_included = 0x1U;
using ch27 = core::field_descriptor<chg2, 27U, 1U>;
inline constexpr std::uint32_t ch27_excluded = 0x0U;
inline constexpr std::uint32_t ch27_included = 0x1U;
using ch28 = core::field_descriptor<chg2, 28U, 1U>;
inline constexpr std::uint32_t ch28_excluded = 0x0U;
inline constexpr std::uint32_t ch28_included = 0x1U;
using ch29 = core::field_descriptor<chg2, 29U, 1U>;
inline constexpr std::uint32_t ch29_excluded = 0x0U;
inline constexpr std::uint32_t ch29_included = 0x1U;
using ch30 = core::field_descriptor<chg2, 30U, 1U>;
inline constexpr std::uint32_t ch30_excluded = 0x0U;
inline constexpr std::uint32_t ch30_included = 0x1U;
using ch31 = core::field_descriptor<chg2, 31U, 1U>;
inline constexpr std::uint32_t ch31_excluded = 0x0U;
inline constexpr std::uint32_t ch31_included = 0x1U;
}
struct chg3 : core::register_descriptor<std::uint32_t, 0x80CU,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace chg3_fields {
using ch0 = core::field_descriptor<chg3, 0U, 1U>;
inline constexpr std::uint32_t ch0_excluded = 0x0U;
inline constexpr std::uint32_t ch0_included = 0x1U;
using ch1 = core::field_descriptor<chg3, 1U, 1U>;
inline constexpr std::uint32_t ch1_excluded = 0x0U;
inline constexpr std::uint32_t ch1_included = 0x1U;
using ch2 = core::field_descriptor<chg3, 2U, 1U>;
inline constexpr std::uint32_t ch2_excluded = 0x0U;
inline constexpr std::uint32_t ch2_included = 0x1U;
using ch3 = core::field_descriptor<chg3, 3U, 1U>;
inline constexpr std::uint32_t ch3_excluded = 0x0U;
inline constexpr std::uint32_t ch3_included = 0x1U;
using ch4 = core::field_descriptor<chg3, 4U, 1U>;
inline constexpr std::uint32_t ch4_excluded = 0x0U;
inline constexpr std::uint32_t ch4_included = 0x1U;
using ch5 = core::field_descriptor<chg3, 5U, 1U>;
inline constexpr std::uint32_t ch5_excluded = 0x0U;
inline constexpr std::uint32_t ch5_included = 0x1U;
using ch6 = core::field_descriptor<chg3, 6U, 1U>;
inline constexpr std::uint32_t ch6_excluded = 0x0U;
inline constexpr std::uint32_t ch6_included = 0x1U;
using ch7 = core::field_descriptor<chg3, 7U, 1U>;
inline constexpr std::uint32_t ch7_excluded = 0x0U;
inline constexpr std::uint32_t ch7_included = 0x1U;
using ch8 = core::field_descriptor<chg3, 8U, 1U>;
inline constexpr std::uint32_t ch8_excluded = 0x0U;
inline constexpr std::uint32_t ch8_included = 0x1U;
using ch9 = core::field_descriptor<chg3, 9U, 1U>;
inline constexpr std::uint32_t ch9_excluded = 0x0U;
inline constexpr std::uint32_t ch9_included = 0x1U;
using ch10 = core::field_descriptor<chg3, 10U, 1U>;
inline constexpr std::uint32_t ch10_excluded = 0x0U;
inline constexpr std::uint32_t ch10_included = 0x1U;
using ch11 = core::field_descriptor<chg3, 11U, 1U>;
inline constexpr std::uint32_t ch11_excluded = 0x0U;
inline constexpr std::uint32_t ch11_included = 0x1U;
using ch12 = core::field_descriptor<chg3, 12U, 1U>;
inline constexpr std::uint32_t ch12_excluded = 0x0U;
inline constexpr std::uint32_t ch12_included = 0x1U;
using ch13 = core::field_descriptor<chg3, 13U, 1U>;
inline constexpr std::uint32_t ch13_excluded = 0x0U;
inline constexpr std::uint32_t ch13_included = 0x1U;
using ch14 = core::field_descriptor<chg3, 14U, 1U>;
inline constexpr std::uint32_t ch14_excluded = 0x0U;
inline constexpr std::uint32_t ch14_included = 0x1U;
using ch15 = core::field_descriptor<chg3, 15U, 1U>;
inline constexpr std::uint32_t ch15_excluded = 0x0U;
inline constexpr std::uint32_t ch15_included = 0x1U;
using ch16 = core::field_descriptor<chg3, 16U, 1U>;
inline constexpr std::uint32_t ch16_excluded = 0x0U;
inline constexpr std::uint32_t ch16_included = 0x1U;
using ch17 = core::field_descriptor<chg3, 17U, 1U>;
inline constexpr std::uint32_t ch17_excluded = 0x0U;
inline constexpr std::uint32_t ch17_included = 0x1U;
using ch18 = core::field_descriptor<chg3, 18U, 1U>;
inline constexpr std::uint32_t ch18_excluded = 0x0U;
inline constexpr std::uint32_t ch18_included = 0x1U;
using ch19 = core::field_descriptor<chg3, 19U, 1U>;
inline constexpr std::uint32_t ch19_excluded = 0x0U;
inline constexpr std::uint32_t ch19_included = 0x1U;
using ch20 = core::field_descriptor<chg3, 20U, 1U>;
inline constexpr std::uint32_t ch20_excluded = 0x0U;
inline constexpr std::uint32_t ch20_included = 0x1U;
using ch21 = core::field_descriptor<chg3, 21U, 1U>;
inline constexpr std::uint32_t ch21_excluded = 0x0U;
inline constexpr std::uint32_t ch21_included = 0x1U;
using ch22 = core::field_descriptor<chg3, 22U, 1U>;
inline constexpr std::uint32_t ch22_excluded = 0x0U;
inline constexpr std::uint32_t ch22_included = 0x1U;
using ch23 = core::field_descriptor<chg3, 23U, 1U>;
inline constexpr std::uint32_t ch23_excluded = 0x0U;
inline constexpr std::uint32_t ch23_included = 0x1U;
using ch24 = core::field_descriptor<chg3, 24U, 1U>;
inline constexpr std::uint32_t ch24_excluded = 0x0U;
inline constexpr std::uint32_t ch24_included = 0x1U;
using ch25 = core::field_descriptor<chg3, 25U, 1U>;
inline constexpr std::uint32_t ch25_excluded = 0x0U;
inline constexpr std::uint32_t ch25_included = 0x1U;
using ch26 = core::field_descriptor<chg3, 26U, 1U>;
inline constexpr std::uint32_t ch26_excluded = 0x0U;
inline constexpr std::uint32_t ch26_included = 0x1U;
using ch27 = core::field_descriptor<chg3, 27U, 1U>;
inline constexpr std::uint32_t ch27_excluded = 0x0U;
inline constexpr std::uint32_t ch27_included = 0x1U;
using ch28 = core::field_descriptor<chg3, 28U, 1U>;
inline constexpr std::uint32_t ch28_excluded = 0x0U;
inline constexpr std::uint32_t ch28_included = 0x1U;
using ch29 = core::field_descriptor<chg3, 29U, 1U>;
inline constexpr std::uint32_t ch29_excluded = 0x0U;
inline constexpr std::uint32_t ch29_included = 0x1U;
using ch30 = core::field_descriptor<chg3, 30U, 1U>;
inline constexpr std::uint32_t ch30_excluded = 0x0U;
inline constexpr std::uint32_t ch30_included = 0x1U;
using ch31 = core::field_descriptor<chg3, 31U, 1U>;
inline constexpr std::uint32_t ch31_excluded = 0x0U;
inline constexpr std::uint32_t ch31_included = 0x1U;
}
struct chg4 : core::register_descriptor<std::uint32_t, 0x810U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace chg4_fields {
using ch0 = core::field_descriptor<chg4, 0U, 1U>;
inline constexpr std::uint32_t ch0_excluded = 0x0U;
inline constexpr std::uint32_t ch0_included = 0x1U;
using ch1 = core::field_descriptor<chg4, 1U, 1U>;
inline constexpr std::uint32_t ch1_excluded = 0x0U;
inline constexpr std::uint32_t ch1_included = 0x1U;
using ch2 = core::field_descriptor<chg4, 2U, 1U>;
inline constexpr std::uint32_t ch2_excluded = 0x0U;
inline constexpr std::uint32_t ch2_included = 0x1U;
using ch3 = core::field_descriptor<chg4, 3U, 1U>;
inline constexpr std::uint32_t ch3_excluded = 0x0U;
inline constexpr std::uint32_t ch3_included = 0x1U;
using ch4 = core::field_descriptor<chg4, 4U, 1U>;
inline constexpr std::uint32_t ch4_excluded = 0x0U;
inline constexpr std::uint32_t ch4_included = 0x1U;
using ch5 = core::field_descriptor<chg4, 5U, 1U>;
inline constexpr std::uint32_t ch5_excluded = 0x0U;
inline constexpr std::uint32_t ch5_included = 0x1U;
using ch6 = core::field_descriptor<chg4, 6U, 1U>;
inline constexpr std::uint32_t ch6_excluded = 0x0U;
inline constexpr std::uint32_t ch6_included = 0x1U;
using ch7 = core::field_descriptor<chg4, 7U, 1U>;
inline constexpr std::uint32_t ch7_excluded = 0x0U;
inline constexpr std::uint32_t ch7_included = 0x1U;
using ch8 = core::field_descriptor<chg4, 8U, 1U>;
inline constexpr std::uint32_t ch8_excluded = 0x0U;
inline constexpr std::uint32_t ch8_included = 0x1U;
using ch9 = core::field_descriptor<chg4, 9U, 1U>;
inline constexpr std::uint32_t ch9_excluded = 0x0U;
inline constexpr std::uint32_t ch9_included = 0x1U;
using ch10 = core::field_descriptor<chg4, 10U, 1U>;
inline constexpr std::uint32_t ch10_excluded = 0x0U;
inline constexpr std::uint32_t ch10_included = 0x1U;
using ch11 = core::field_descriptor<chg4, 11U, 1U>;
inline constexpr std::uint32_t ch11_excluded = 0x0U;
inline constexpr std::uint32_t ch11_included = 0x1U;
using ch12 = core::field_descriptor<chg4, 12U, 1U>;
inline constexpr std::uint32_t ch12_excluded = 0x0U;
inline constexpr std::uint32_t ch12_included = 0x1U;
using ch13 = core::field_descriptor<chg4, 13U, 1U>;
inline constexpr std::uint32_t ch13_excluded = 0x0U;
inline constexpr std::uint32_t ch13_included = 0x1U;
using ch14 = core::field_descriptor<chg4, 14U, 1U>;
inline constexpr std::uint32_t ch14_excluded = 0x0U;
inline constexpr std::uint32_t ch14_included = 0x1U;
using ch15 = core::field_descriptor<chg4, 15U, 1U>;
inline constexpr std::uint32_t ch15_excluded = 0x0U;
inline constexpr std::uint32_t ch15_included = 0x1U;
using ch16 = core::field_descriptor<chg4, 16U, 1U>;
inline constexpr std::uint32_t ch16_excluded = 0x0U;
inline constexpr std::uint32_t ch16_included = 0x1U;
using ch17 = core::field_descriptor<chg4, 17U, 1U>;
inline constexpr std::uint32_t ch17_excluded = 0x0U;
inline constexpr std::uint32_t ch17_included = 0x1U;
using ch18 = core::field_descriptor<chg4, 18U, 1U>;
inline constexpr std::uint32_t ch18_excluded = 0x0U;
inline constexpr std::uint32_t ch18_included = 0x1U;
using ch19 = core::field_descriptor<chg4, 19U, 1U>;
inline constexpr std::uint32_t ch19_excluded = 0x0U;
inline constexpr std::uint32_t ch19_included = 0x1U;
using ch20 = core::field_descriptor<chg4, 20U, 1U>;
inline constexpr std::uint32_t ch20_excluded = 0x0U;
inline constexpr std::uint32_t ch20_included = 0x1U;
using ch21 = core::field_descriptor<chg4, 21U, 1U>;
inline constexpr std::uint32_t ch21_excluded = 0x0U;
inline constexpr std::uint32_t ch21_included = 0x1U;
using ch22 = core::field_descriptor<chg4, 22U, 1U>;
inline constexpr std::uint32_t ch22_excluded = 0x0U;
inline constexpr std::uint32_t ch22_included = 0x1U;
using ch23 = core::field_descriptor<chg4, 23U, 1U>;
inline constexpr std::uint32_t ch23_excluded = 0x0U;
inline constexpr std::uint32_t ch23_included = 0x1U;
using ch24 = core::field_descriptor<chg4, 24U, 1U>;
inline constexpr std::uint32_t ch24_excluded = 0x0U;
inline constexpr std::uint32_t ch24_included = 0x1U;
using ch25 = core::field_descriptor<chg4, 25U, 1U>;
inline constexpr std::uint32_t ch25_excluded = 0x0U;
inline constexpr std::uint32_t ch25_included = 0x1U;
using ch26 = core::field_descriptor<chg4, 26U, 1U>;
inline constexpr std::uint32_t ch26_excluded = 0x0U;
inline constexpr std::uint32_t ch26_included = 0x1U;
using ch27 = core::field_descriptor<chg4, 27U, 1U>;
inline constexpr std::uint32_t ch27_excluded = 0x0U;
inline constexpr std::uint32_t ch27_included = 0x1U;
using ch28 = core::field_descriptor<chg4, 28U, 1U>;
inline constexpr std::uint32_t ch28_excluded = 0x0U;
inline constexpr std::uint32_t ch28_included = 0x1U;
using ch29 = core::field_descriptor<chg4, 29U, 1U>;
inline constexpr std::uint32_t ch29_excluded = 0x0U;
inline constexpr std::uint32_t ch29_included = 0x1U;
using ch30 = core::field_descriptor<chg4, 30U, 1U>;
inline constexpr std::uint32_t ch30_excluded = 0x0U;
inline constexpr std::uint32_t ch30_included = 0x1U;
using ch31 = core::field_descriptor<chg4, 31U, 1U>;
inline constexpr std::uint32_t ch31_excluded = 0x0U;
inline constexpr std::uint32_t ch31_included = 0x1U;
}
struct chg5 : core::register_descriptor<std::uint32_t, 0x814U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace chg5_fields {
using ch0 = core::field_descriptor<chg5, 0U, 1U>;
inline constexpr std::uint32_t ch0_excluded = 0x0U;
inline constexpr std::uint32_t ch0_included = 0x1U;
using ch1 = core::field_descriptor<chg5, 1U, 1U>;
inline constexpr std::uint32_t ch1_excluded = 0x0U;
inline constexpr std::uint32_t ch1_included = 0x1U;
using ch2 = core::field_descriptor<chg5, 2U, 1U>;
inline constexpr std::uint32_t ch2_excluded = 0x0U;
inline constexpr std::uint32_t ch2_included = 0x1U;
using ch3 = core::field_descriptor<chg5, 3U, 1U>;
inline constexpr std::uint32_t ch3_excluded = 0x0U;
inline constexpr std::uint32_t ch3_included = 0x1U;
using ch4 = core::field_descriptor<chg5, 4U, 1U>;
inline constexpr std::uint32_t ch4_excluded = 0x0U;
inline constexpr std::uint32_t ch4_included = 0x1U;
using ch5 = core::field_descriptor<chg5, 5U, 1U>;
inline constexpr std::uint32_t ch5_excluded = 0x0U;
inline constexpr std::uint32_t ch5_included = 0x1U;
using ch6 = core::field_descriptor<chg5, 6U, 1U>;
inline constexpr std::uint32_t ch6_excluded = 0x0U;
inline constexpr std::uint32_t ch6_included = 0x1U;
using ch7 = core::field_descriptor<chg5, 7U, 1U>;
inline constexpr std::uint32_t ch7_excluded = 0x0U;
inline constexpr std::uint32_t ch7_included = 0x1U;
using ch8 = core::field_descriptor<chg5, 8U, 1U>;
inline constexpr std::uint32_t ch8_excluded = 0x0U;
inline constexpr std::uint32_t ch8_included = 0x1U;
using ch9 = core::field_descriptor<chg5, 9U, 1U>;
inline constexpr std::uint32_t ch9_excluded = 0x0U;
inline constexpr std::uint32_t ch9_included = 0x1U;
using ch10 = core::field_descriptor<chg5, 10U, 1U>;
inline constexpr std::uint32_t ch10_excluded = 0x0U;
inline constexpr std::uint32_t ch10_included = 0x1U;
using ch11 = core::field_descriptor<chg5, 11U, 1U>;
inline constexpr std::uint32_t ch11_excluded = 0x0U;
inline constexpr std::uint32_t ch11_included = 0x1U;
using ch12 = core::field_descriptor<chg5, 12U, 1U>;
inline constexpr std::uint32_t ch12_excluded = 0x0U;
inline constexpr std::uint32_t ch12_included = 0x1U;
using ch13 = core::field_descriptor<chg5, 13U, 1U>;
inline constexpr std::uint32_t ch13_excluded = 0x0U;
inline constexpr std::uint32_t ch13_included = 0x1U;
using ch14 = core::field_descriptor<chg5, 14U, 1U>;
inline constexpr std::uint32_t ch14_excluded = 0x0U;
inline constexpr std::uint32_t ch14_included = 0x1U;
using ch15 = core::field_descriptor<chg5, 15U, 1U>;
inline constexpr std::uint32_t ch15_excluded = 0x0U;
inline constexpr std::uint32_t ch15_included = 0x1U;
using ch16 = core::field_descriptor<chg5, 16U, 1U>;
inline constexpr std::uint32_t ch16_excluded = 0x0U;
inline constexpr std::uint32_t ch16_included = 0x1U;
using ch17 = core::field_descriptor<chg5, 17U, 1U>;
inline constexpr std::uint32_t ch17_excluded = 0x0U;
inline constexpr std::uint32_t ch17_included = 0x1U;
using ch18 = core::field_descriptor<chg5, 18U, 1U>;
inline constexpr std::uint32_t ch18_excluded = 0x0U;
inline constexpr std::uint32_t ch18_included = 0x1U;
using ch19 = core::field_descriptor<chg5, 19U, 1U>;
inline constexpr std::uint32_t ch19_excluded = 0x0U;
inline constexpr std::uint32_t ch19_included = 0x1U;
using ch20 = core::field_descriptor<chg5, 20U, 1U>;
inline constexpr std::uint32_t ch20_excluded = 0x0U;
inline constexpr std::uint32_t ch20_included = 0x1U;
using ch21 = core::field_descriptor<chg5, 21U, 1U>;
inline constexpr std::uint32_t ch21_excluded = 0x0U;
inline constexpr std::uint32_t ch21_included = 0x1U;
using ch22 = core::field_descriptor<chg5, 22U, 1U>;
inline constexpr std::uint32_t ch22_excluded = 0x0U;
inline constexpr std::uint32_t ch22_included = 0x1U;
using ch23 = core::field_descriptor<chg5, 23U, 1U>;
inline constexpr std::uint32_t ch23_excluded = 0x0U;
inline constexpr std::uint32_t ch23_included = 0x1U;
using ch24 = core::field_descriptor<chg5, 24U, 1U>;
inline constexpr std::uint32_t ch24_excluded = 0x0U;
inline constexpr std::uint32_t ch24_included = 0x1U;
using ch25 = core::field_descriptor<chg5, 25U, 1U>;
inline constexpr std::uint32_t ch25_excluded = 0x0U;
inline constexpr std::uint32_t ch25_included = 0x1U;
using ch26 = core::field_descriptor<chg5, 26U, 1U>;
inline constexpr std::uint32_t ch26_excluded = 0x0U;
inline constexpr std::uint32_t ch26_included = 0x1U;
using ch27 = core::field_descriptor<chg5, 27U, 1U>;
inline constexpr std::uint32_t ch27_excluded = 0x0U;
inline constexpr std::uint32_t ch27_included = 0x1U;
using ch28 = core::field_descriptor<chg5, 28U, 1U>;
inline constexpr std::uint32_t ch28_excluded = 0x0U;
inline constexpr std::uint32_t ch28_included = 0x1U;
using ch29 = core::field_descriptor<chg5, 29U, 1U>;
inline constexpr std::uint32_t ch29_excluded = 0x0U;
inline constexpr std::uint32_t ch29_included = 0x1U;
using ch30 = core::field_descriptor<chg5, 30U, 1U>;
inline constexpr std::uint32_t ch30_excluded = 0x0U;
inline constexpr std::uint32_t ch30_included = 0x1U;
using ch31 = core::field_descriptor<chg5, 31U, 1U>;
inline constexpr std::uint32_t ch31_excluded = 0x0U;
inline constexpr std::uint32_t ch31_included = 0x1U;
}
struct fork0_tep : core::register_descriptor<std::uint32_t, 0x910U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace fork0_tep_fields {
using tep = core::field_descriptor<fork0_tep, 0U, 32U>;
}
struct fork1_tep : core::register_descriptor<std::uint32_t, 0x914U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace fork1_tep_fields {
using tep = core::field_descriptor<fork1_tep, 0U, 32U>;
}
struct fork2_tep : core::register_descriptor<std::uint32_t, 0x918U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace fork2_tep_fields {
using tep = core::field_descriptor<fork2_tep, 0U, 32U>;
}
struct fork3_tep : core::register_descriptor<std::uint32_t, 0x91CU,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace fork3_tep_fields {
using tep = core::field_descriptor<fork3_tep, 0U, 32U>;
}
struct fork4_tep : core::register_descriptor<std::uint32_t, 0x920U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace fork4_tep_fields {
using tep = core::field_descriptor<fork4_tep, 0U, 32U>;
}
struct fork5_tep : core::register_descriptor<std::uint32_t, 0x924U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace fork5_tep_fields {
using tep = core::field_descriptor<fork5_tep, 0U, 32U>;
}
struct fork6_tep : core::register_descriptor<std::uint32_t, 0x928U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace fork6_tep_fields {
using tep = core::field_descriptor<fork6_tep, 0U, 32U>;
}
struct fork7_tep : core::register_descriptor<std::uint32_t, 0x92CU,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace fork7_tep_fields {
using tep = core::field_descriptor<fork7_tep, 0U, 32U>;
}
struct fork8_tep : core::register_descriptor<std::uint32_t, 0x930U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace fork8_tep_fields {
using tep = core::field_descriptor<fork8_tep, 0U, 32U>;
}
struct fork9_tep : core::register_descriptor<std::uint32_t, 0x934U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace fork9_tep_fields {
using tep = core::field_descriptor<fork9_tep, 0U, 32U>;
}
struct fork10_tep : core::register_descriptor<std::uint32_t, 0x938U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace fork10_tep_fields {
using tep = core::field_descriptor<fork10_tep, 0U, 32U>;
}
struct fork11_tep : core::register_descriptor<std::uint32_t, 0x93CU,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace fork11_tep_fields {
using tep = core::field_descriptor<fork11_tep, 0U, 32U>;
}
struct fork12_tep : core::register_descriptor<std::uint32_t, 0x940U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace fork12_tep_fields {
using tep = core::field_descriptor<fork12_tep, 0U, 32U>;
}
struct fork13_tep : core::register_descriptor<std::uint32_t, 0x944U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace fork13_tep_fields {
using tep = core::field_descriptor<fork13_tep, 0U, 32U>;
}
struct fork14_tep : core::register_descriptor<std::uint32_t, 0x948U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace fork14_tep_fields {
using tep = core::field_descriptor<fork14_tep, 0U, 32U>;
}
struct fork15_tep : core::register_descriptor<std::uint32_t, 0x94CU,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace fork15_tep_fields {
using tep = core::field_descriptor<fork15_tep, 0U, 32U>;
}
struct fork16_tep : core::register_descriptor<std::uint32_t, 0x950U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace fork16_tep_fields {
using tep = core::field_descriptor<fork16_tep, 0U, 32U>;
}
struct fork17_tep : core::register_descriptor<std::uint32_t, 0x954U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace fork17_tep_fields {
using tep = core::field_descriptor<fork17_tep, 0U, 32U>;
}
struct fork18_tep : core::register_descriptor<std::uint32_t, 0x958U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace fork18_tep_fields {
using tep = core::field_descriptor<fork18_tep, 0U, 32U>;
}
struct fork19_tep : core::register_descriptor<std::uint32_t, 0x95CU,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace fork19_tep_fields {
using tep = core::field_descriptor<fork19_tep, 0U, 32U>;
}
struct fork20_tep : core::register_descriptor<std::uint32_t, 0x960U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace fork20_tep_fields {
using tep = core::field_descriptor<fork20_tep, 0U, 32U>;
}
struct fork21_tep : core::register_descriptor<std::uint32_t, 0x964U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace fork21_tep_fields {
using tep = core::field_descriptor<fork21_tep, 0U, 32U>;
}
struct fork22_tep : core::register_descriptor<std::uint32_t, 0x968U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace fork22_tep_fields {
using tep = core::field_descriptor<fork22_tep, 0U, 32U>;
}
struct fork23_tep : core::register_descriptor<std::uint32_t, 0x96CU,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace fork23_tep_fields {
using tep = core::field_descriptor<fork23_tep, 0U, 32U>;
}
struct fork24_tep : core::register_descriptor<std::uint32_t, 0x970U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace fork24_tep_fields {
using tep = core::field_descriptor<fork24_tep, 0U, 32U>;
}
struct fork25_tep : core::register_descriptor<std::uint32_t, 0x974U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace fork25_tep_fields {
using tep = core::field_descriptor<fork25_tep, 0U, 32U>;
}
struct fork26_tep : core::register_descriptor<std::uint32_t, 0x978U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace fork26_tep_fields {
using tep = core::field_descriptor<fork26_tep, 0U, 32U>;
}
struct fork27_tep : core::register_descriptor<std::uint32_t, 0x97CU,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace fork27_tep_fields {
using tep = core::field_descriptor<fork27_tep, 0U, 32U>;
}
struct fork28_tep : core::register_descriptor<std::uint32_t, 0x980U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace fork28_tep_fields {
using tep = core::field_descriptor<fork28_tep, 0U, 32U>;
}
struct fork29_tep : core::register_descriptor<std::uint32_t, 0x984U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace fork29_tep_fields {
using tep = core::field_descriptor<fork29_tep, 0U, 32U>;
}
struct fork30_tep : core::register_descriptor<std::uint32_t, 0x988U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace fork30_tep_fields {
using tep = core::field_descriptor<fork30_tep, 0U, 32U>;
}
struct fork31_tep : core::register_descriptor<std::uint32_t, 0x98CU,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0x00000000U, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace fork31_tep_fields {
using tep = core::field_descriptor<fork31_tep, 0U, 32U>;
}
}
