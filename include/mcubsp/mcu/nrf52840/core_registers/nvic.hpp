#pragma once
// @req-file REQ-0055 REQ-0073
// CMSIS-derived facts: Copyright (c) 2009-2020 Arm Limited. Apache-2.0.
// Source identities and semantic review: doc/reference/nrf52840/core-facts.json.
// See ARM-LICENSE.txt. Selected nRF52840 Cortex-M4F limits apply.
#include <mcubsp/core/register.hpp>
namespace mcubsp::mcu::nrf52840::core_registers::nvic {
inline constexpr std::uintptr_t base = 0xE000E100U;
struct iser0 : core::register_descriptor<std::uint32_t, 0x0U,
    core::access_semantics::set_alias, 0x3FFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
using iser0_status = core::register_descriptor<std::uint32_t, 0x0U,
    core::access_semantics::read_only, 0U, false, core::reserved_bit_policy::write_zero,
    0U, 0x0U, 0x0U, iser0>;
namespace iser0_fields {
using value = core::field_descriptor<iser0, 0U, 32U, core::access_semantics::set_alias>;
}
struct iser1 : core::register_descriptor<std::uint32_t, 0x4U,
    core::access_semantics::set_alias, 0xA7FFU, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
using iser1_status = core::register_descriptor<std::uint32_t, 0x4U,
    core::access_semantics::read_only, 0U, false, core::reserved_bit_policy::write_zero,
    0U, 0x0U, 0x0U, iser1>;
namespace iser1_fields {
using value = core::field_descriptor<iser1, 0U, 32U, core::access_semantics::set_alias>;
}
struct icer0 : core::register_descriptor<std::uint32_t, 0x80U,
    core::access_semantics::clear_alias, 0x3FFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
using icer0_status = core::register_descriptor<std::uint32_t, 0x80U,
    core::access_semantics::read_only, 0U, false, core::reserved_bit_policy::write_zero,
    0U, 0x0U, 0x0U, icer0>;
namespace icer0_fields {
using value = core::field_descriptor<icer0, 0U, 32U, core::access_semantics::clear_alias>;
}
struct icer1 : core::register_descriptor<std::uint32_t, 0x84U,
    core::access_semantics::clear_alias, 0xA7FFU, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
using icer1_status = core::register_descriptor<std::uint32_t, 0x84U,
    core::access_semantics::read_only, 0U, false, core::reserved_bit_policy::write_zero,
    0U, 0x0U, 0x0U, icer1>;
namespace icer1_fields {
using value = core::field_descriptor<icer1, 0U, 32U, core::access_semantics::clear_alias>;
}
struct ispr0 : core::register_descriptor<std::uint32_t, 0x100U,
    core::access_semantics::set_alias, 0x3FFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
using ispr0_status = core::register_descriptor<std::uint32_t, 0x100U,
    core::access_semantics::read_only, 0U, false, core::reserved_bit_policy::write_zero,
    0U, 0x0U, 0x0U, ispr0>;
namespace ispr0_fields {
using value = core::field_descriptor<ispr0, 0U, 32U, core::access_semantics::set_alias>;
}
struct ispr1 : core::register_descriptor<std::uint32_t, 0x104U,
    core::access_semantics::set_alias, 0xA7FFU, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
using ispr1_status = core::register_descriptor<std::uint32_t, 0x104U,
    core::access_semantics::read_only, 0U, false, core::reserved_bit_policy::write_zero,
    0U, 0x0U, 0x0U, ispr1>;
namespace ispr1_fields {
using value = core::field_descriptor<ispr1, 0U, 32U, core::access_semantics::set_alias>;
}
struct icpr0 : core::register_descriptor<std::uint32_t, 0x180U,
    core::access_semantics::clear_alias, 0x3FFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
using icpr0_status = core::register_descriptor<std::uint32_t, 0x180U,
    core::access_semantics::read_only, 0U, false, core::reserved_bit_policy::write_zero,
    0U, 0x0U, 0x0U, icpr0>;
namespace icpr0_fields {
using value = core::field_descriptor<icpr0, 0U, 32U, core::access_semantics::clear_alias>;
}
struct icpr1 : core::register_descriptor<std::uint32_t, 0x184U,
    core::access_semantics::clear_alias, 0xA7FFU, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
using icpr1_status = core::register_descriptor<std::uint32_t, 0x184U,
    core::access_semantics::read_only, 0U, false, core::reserved_bit_policy::write_zero,
    0U, 0x0U, 0x0U, icpr1>;
namespace icpr1_fields {
using value = core::field_descriptor<icpr1, 0U, 32U, core::access_semantics::clear_alias>;
}
struct iabr0 : core::register_descriptor<std::uint32_t, 0x200U,
    core::access_semantics::read_only, 0x0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace iabr0_fields {
using value = core::field_descriptor<iabr0, 0U, 32U, core::access_semantics::read_only>;
}
struct iabr1 : core::register_descriptor<std::uint32_t, 0x204U,
    core::access_semantics::read_only, 0x0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace iabr1_fields {
using value = core::field_descriptor<iabr1, 0U, 32U, core::access_semantics::read_only>;
}
struct ip0 : core::register_descriptor<std::uint8_t, 0x300U,
    core::access_semantics::read_write, 0xE0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace ip0_fields {
using priority = core::field_descriptor<ip0, 5U, 3U, core::access_semantics::read_write>;
}
struct ip1 : core::register_descriptor<std::uint8_t, 0x301U,
    core::access_semantics::read_write, 0xE0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace ip1_fields {
using priority = core::field_descriptor<ip1, 5U, 3U, core::access_semantics::read_write>;
}
struct ip2 : core::register_descriptor<std::uint8_t, 0x302U,
    core::access_semantics::read_write, 0xE0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace ip2_fields {
using priority = core::field_descriptor<ip2, 5U, 3U, core::access_semantics::read_write>;
}
struct ip3 : core::register_descriptor<std::uint8_t, 0x303U,
    core::access_semantics::read_write, 0xE0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace ip3_fields {
using priority = core::field_descriptor<ip3, 5U, 3U, core::access_semantics::read_write>;
}
struct ip4 : core::register_descriptor<std::uint8_t, 0x304U,
    core::access_semantics::read_write, 0xE0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace ip4_fields {
using priority = core::field_descriptor<ip4, 5U, 3U, core::access_semantics::read_write>;
}
struct ip5 : core::register_descriptor<std::uint8_t, 0x305U,
    core::access_semantics::read_write, 0xE0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace ip5_fields {
using priority = core::field_descriptor<ip5, 5U, 3U, core::access_semantics::read_write>;
}
struct ip6 : core::register_descriptor<std::uint8_t, 0x306U,
    core::access_semantics::read_write, 0xE0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace ip6_fields {
using priority = core::field_descriptor<ip6, 5U, 3U, core::access_semantics::read_write>;
}
struct ip7 : core::register_descriptor<std::uint8_t, 0x307U,
    core::access_semantics::read_write, 0xE0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace ip7_fields {
using priority = core::field_descriptor<ip7, 5U, 3U, core::access_semantics::read_write>;
}
struct ip8 : core::register_descriptor<std::uint8_t, 0x308U,
    core::access_semantics::read_write, 0xE0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace ip8_fields {
using priority = core::field_descriptor<ip8, 5U, 3U, core::access_semantics::read_write>;
}
struct ip9 : core::register_descriptor<std::uint8_t, 0x309U,
    core::access_semantics::read_write, 0xE0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace ip9_fields {
using priority = core::field_descriptor<ip9, 5U, 3U, core::access_semantics::read_write>;
}
struct ip10 : core::register_descriptor<std::uint8_t, 0x30AU,
    core::access_semantics::read_write, 0xE0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace ip10_fields {
using priority = core::field_descriptor<ip10, 5U, 3U, core::access_semantics::read_write>;
}
struct ip11 : core::register_descriptor<std::uint8_t, 0x30BU,
    core::access_semantics::read_write, 0xE0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace ip11_fields {
using priority = core::field_descriptor<ip11, 5U, 3U, core::access_semantics::read_write>;
}
struct ip12 : core::register_descriptor<std::uint8_t, 0x30CU,
    core::access_semantics::read_write, 0xE0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace ip12_fields {
using priority = core::field_descriptor<ip12, 5U, 3U, core::access_semantics::read_write>;
}
struct ip13 : core::register_descriptor<std::uint8_t, 0x30DU,
    core::access_semantics::read_write, 0xE0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace ip13_fields {
using priority = core::field_descriptor<ip13, 5U, 3U, core::access_semantics::read_write>;
}
struct ip14 : core::register_descriptor<std::uint8_t, 0x30EU,
    core::access_semantics::read_write, 0xE0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace ip14_fields {
using priority = core::field_descriptor<ip14, 5U, 3U, core::access_semantics::read_write>;
}
struct ip15 : core::register_descriptor<std::uint8_t, 0x30FU,
    core::access_semantics::read_write, 0xE0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace ip15_fields {
using priority = core::field_descriptor<ip15, 5U, 3U, core::access_semantics::read_write>;
}
struct ip16 : core::register_descriptor<std::uint8_t, 0x310U,
    core::access_semantics::read_write, 0xE0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace ip16_fields {
using priority = core::field_descriptor<ip16, 5U, 3U, core::access_semantics::read_write>;
}
struct ip17 : core::register_descriptor<std::uint8_t, 0x311U,
    core::access_semantics::read_write, 0xE0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace ip17_fields {
using priority = core::field_descriptor<ip17, 5U, 3U, core::access_semantics::read_write>;
}
struct ip18 : core::register_descriptor<std::uint8_t, 0x312U,
    core::access_semantics::read_write, 0xE0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace ip18_fields {
using priority = core::field_descriptor<ip18, 5U, 3U, core::access_semantics::read_write>;
}
struct ip19 : core::register_descriptor<std::uint8_t, 0x313U,
    core::access_semantics::read_write, 0xE0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace ip19_fields {
using priority = core::field_descriptor<ip19, 5U, 3U, core::access_semantics::read_write>;
}
struct ip20 : core::register_descriptor<std::uint8_t, 0x314U,
    core::access_semantics::read_write, 0xE0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace ip20_fields {
using priority = core::field_descriptor<ip20, 5U, 3U, core::access_semantics::read_write>;
}
struct ip21 : core::register_descriptor<std::uint8_t, 0x315U,
    core::access_semantics::read_write, 0xE0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace ip21_fields {
using priority = core::field_descriptor<ip21, 5U, 3U, core::access_semantics::read_write>;
}
struct ip22 : core::register_descriptor<std::uint8_t, 0x316U,
    core::access_semantics::read_write, 0xE0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace ip22_fields {
using priority = core::field_descriptor<ip22, 5U, 3U, core::access_semantics::read_write>;
}
struct ip23 : core::register_descriptor<std::uint8_t, 0x317U,
    core::access_semantics::read_write, 0xE0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace ip23_fields {
using priority = core::field_descriptor<ip23, 5U, 3U, core::access_semantics::read_write>;
}
struct ip24 : core::register_descriptor<std::uint8_t, 0x318U,
    core::access_semantics::read_write, 0xE0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace ip24_fields {
using priority = core::field_descriptor<ip24, 5U, 3U, core::access_semantics::read_write>;
}
struct ip25 : core::register_descriptor<std::uint8_t, 0x319U,
    core::access_semantics::read_write, 0xE0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace ip25_fields {
using priority = core::field_descriptor<ip25, 5U, 3U, core::access_semantics::read_write>;
}
struct ip26 : core::register_descriptor<std::uint8_t, 0x31AU,
    core::access_semantics::read_write, 0xE0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace ip26_fields {
using priority = core::field_descriptor<ip26, 5U, 3U, core::access_semantics::read_write>;
}
struct ip27 : core::register_descriptor<std::uint8_t, 0x31BU,
    core::access_semantics::read_write, 0xE0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace ip27_fields {
using priority = core::field_descriptor<ip27, 5U, 3U, core::access_semantics::read_write>;
}
struct ip28 : core::register_descriptor<std::uint8_t, 0x31CU,
    core::access_semantics::read_write, 0xE0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace ip28_fields {
using priority = core::field_descriptor<ip28, 5U, 3U, core::access_semantics::read_write>;
}
struct ip29 : core::register_descriptor<std::uint8_t, 0x31DU,
    core::access_semantics::read_write, 0xE0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace ip29_fields {
using priority = core::field_descriptor<ip29, 5U, 3U, core::access_semantics::read_write>;
}
struct ip32 : core::register_descriptor<std::uint8_t, 0x320U,
    core::access_semantics::read_write, 0xE0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace ip32_fields {
using priority = core::field_descriptor<ip32, 5U, 3U, core::access_semantics::read_write>;
}
struct ip33 : core::register_descriptor<std::uint8_t, 0x321U,
    core::access_semantics::read_write, 0xE0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace ip33_fields {
using priority = core::field_descriptor<ip33, 5U, 3U, core::access_semantics::read_write>;
}
struct ip34 : core::register_descriptor<std::uint8_t, 0x322U,
    core::access_semantics::read_write, 0xE0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace ip34_fields {
using priority = core::field_descriptor<ip34, 5U, 3U, core::access_semantics::read_write>;
}
struct ip35 : core::register_descriptor<std::uint8_t, 0x323U,
    core::access_semantics::read_write, 0xE0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace ip35_fields {
using priority = core::field_descriptor<ip35, 5U, 3U, core::access_semantics::read_write>;
}
struct ip36 : core::register_descriptor<std::uint8_t, 0x324U,
    core::access_semantics::read_write, 0xE0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace ip36_fields {
using priority = core::field_descriptor<ip36, 5U, 3U, core::access_semantics::read_write>;
}
struct ip37 : core::register_descriptor<std::uint8_t, 0x325U,
    core::access_semantics::read_write, 0xE0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace ip37_fields {
using priority = core::field_descriptor<ip37, 5U, 3U, core::access_semantics::read_write>;
}
struct ip38 : core::register_descriptor<std::uint8_t, 0x326U,
    core::access_semantics::read_write, 0xE0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace ip38_fields {
using priority = core::field_descriptor<ip38, 5U, 3U, core::access_semantics::read_write>;
}
struct ip39 : core::register_descriptor<std::uint8_t, 0x327U,
    core::access_semantics::read_write, 0xE0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace ip39_fields {
using priority = core::field_descriptor<ip39, 5U, 3U, core::access_semantics::read_write>;
}
struct ip40 : core::register_descriptor<std::uint8_t, 0x328U,
    core::access_semantics::read_write, 0xE0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace ip40_fields {
using priority = core::field_descriptor<ip40, 5U, 3U, core::access_semantics::read_write>;
}
struct ip41 : core::register_descriptor<std::uint8_t, 0x329U,
    core::access_semantics::read_write, 0xE0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace ip41_fields {
using priority = core::field_descriptor<ip41, 5U, 3U, core::access_semantics::read_write>;
}
struct ip42 : core::register_descriptor<std::uint8_t, 0x32AU,
    core::access_semantics::read_write, 0xE0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace ip42_fields {
using priority = core::field_descriptor<ip42, 5U, 3U, core::access_semantics::read_write>;
}
struct ip45 : core::register_descriptor<std::uint8_t, 0x32DU,
    core::access_semantics::read_write, 0xE0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace ip45_fields {
using priority = core::field_descriptor<ip45, 5U, 3U, core::access_semantics::read_write>;
}
struct ip47 : core::register_descriptor<std::uint8_t, 0x32FU,
    core::access_semantics::read_write, 0xE0U, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace ip47_fields {
using priority = core::field_descriptor<ip47, 5U, 3U, core::access_semantics::read_write>;
}
struct stir : core::register_descriptor<std::uint32_t, 0xE00U,
    core::access_semantics::command, 0x1FFU, false,
    core::reserved_bit_policy::write_zero, 0x0U, 0x0U, 0x0U> {};
namespace stir_fields {
using intid = core::field_descriptor<stir, 0U, 9U, core::access_semantics::command>;
}
}
