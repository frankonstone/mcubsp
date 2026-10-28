#pragma once

// @req-file REQ-0055
// Nordic Semiconductor BSD-3-Clause derived register facts.
// Source: nrfx v3.2.0 nrf52840.svd; license: doc/reference/nrf52840/NORDIC-LICENSE.txt.
// Raw source facts; see system.hpp and doc/reference/nrf52840/README.md for limits.
#include <mcubsp/core/register.hpp>

namespace mcubsp::mcu::nrf52840::registers::uicr {
struct nrffw0 : core::register_descriptor<std::uint32_t, 0x14U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0xFFFFFFFFU, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace nrffw0_fields {
using nrffw = core::field_descriptor<nrffw0, 0U, 32U>;
}
struct nrffw1 : core::register_descriptor<std::uint32_t, 0x18U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0xFFFFFFFFU, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace nrffw1_fields {
using nrffw = core::field_descriptor<nrffw1, 0U, 32U>;
}
struct nrffw2 : core::register_descriptor<std::uint32_t, 0x1CU,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0xFFFFFFFFU, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace nrffw2_fields {
using nrffw = core::field_descriptor<nrffw2, 0U, 32U>;
}
struct nrffw3 : core::register_descriptor<std::uint32_t, 0x20U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0xFFFFFFFFU, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace nrffw3_fields {
using nrffw = core::field_descriptor<nrffw3, 0U, 32U>;
}
struct nrffw4 : core::register_descriptor<std::uint32_t, 0x24U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0xFFFFFFFFU, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace nrffw4_fields {
using nrffw = core::field_descriptor<nrffw4, 0U, 32U>;
}
struct nrffw5 : core::register_descriptor<std::uint32_t, 0x28U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0xFFFFFFFFU, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace nrffw5_fields {
using nrffw = core::field_descriptor<nrffw5, 0U, 32U>;
}
struct nrffw6 : core::register_descriptor<std::uint32_t, 0x2CU,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0xFFFFFFFFU, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace nrffw6_fields {
using nrffw = core::field_descriptor<nrffw6, 0U, 32U>;
}
struct nrffw7 : core::register_descriptor<std::uint32_t, 0x30U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0xFFFFFFFFU, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace nrffw7_fields {
using nrffw = core::field_descriptor<nrffw7, 0U, 32U>;
}
struct nrffw8 : core::register_descriptor<std::uint32_t, 0x34U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0xFFFFFFFFU, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace nrffw8_fields {
using nrffw = core::field_descriptor<nrffw8, 0U, 32U>;
}
struct nrffw9 : core::register_descriptor<std::uint32_t, 0x38U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0xFFFFFFFFU, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace nrffw9_fields {
using nrffw = core::field_descriptor<nrffw9, 0U, 32U>;
}
struct nrffw10 : core::register_descriptor<std::uint32_t, 0x3CU,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0xFFFFFFFFU, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace nrffw10_fields {
using nrffw = core::field_descriptor<nrffw10, 0U, 32U>;
}
struct nrffw11 : core::register_descriptor<std::uint32_t, 0x40U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0xFFFFFFFFU, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace nrffw11_fields {
using nrffw = core::field_descriptor<nrffw11, 0U, 32U>;
}
struct nrffw12 : core::register_descriptor<std::uint32_t, 0x44U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0xFFFFFFFFU, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace nrffw12_fields {
using nrffw = core::field_descriptor<nrffw12, 0U, 32U>;
}
struct nrfhw0 : core::register_descriptor<std::uint32_t, 0x50U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0xFFFFFFFFU, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace nrfhw0_fields {
using nrfhw = core::field_descriptor<nrfhw0, 0U, 32U>;
}
struct nrfhw1 : core::register_descriptor<std::uint32_t, 0x54U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0xFFFFFFFFU, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace nrfhw1_fields {
using nrfhw = core::field_descriptor<nrfhw1, 0U, 32U>;
}
struct nrfhw2 : core::register_descriptor<std::uint32_t, 0x58U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0xFFFFFFFFU, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace nrfhw2_fields {
using nrfhw = core::field_descriptor<nrfhw2, 0U, 32U>;
}
struct nrfhw3 : core::register_descriptor<std::uint32_t, 0x5CU,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0xFFFFFFFFU, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace nrfhw3_fields {
using nrfhw = core::field_descriptor<nrfhw3, 0U, 32U>;
}
struct nrfhw4 : core::register_descriptor<std::uint32_t, 0x60U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0xFFFFFFFFU, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace nrfhw4_fields {
using nrfhw = core::field_descriptor<nrfhw4, 0U, 32U>;
}
struct nrfhw5 : core::register_descriptor<std::uint32_t, 0x64U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0xFFFFFFFFU, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace nrfhw5_fields {
using nrfhw = core::field_descriptor<nrfhw5, 0U, 32U>;
}
struct nrfhw6 : core::register_descriptor<std::uint32_t, 0x68U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0xFFFFFFFFU, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace nrfhw6_fields {
using nrfhw = core::field_descriptor<nrfhw6, 0U, 32U>;
}
struct nrfhw7 : core::register_descriptor<std::uint32_t, 0x6CU,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0xFFFFFFFFU, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace nrfhw7_fields {
using nrfhw = core::field_descriptor<nrfhw7, 0U, 32U>;
}
struct nrfhw8 : core::register_descriptor<std::uint32_t, 0x70U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0xFFFFFFFFU, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace nrfhw8_fields {
using nrfhw = core::field_descriptor<nrfhw8, 0U, 32U>;
}
struct nrfhw9 : core::register_descriptor<std::uint32_t, 0x74U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0xFFFFFFFFU, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace nrfhw9_fields {
using nrfhw = core::field_descriptor<nrfhw9, 0U, 32U>;
}
struct nrfhw10 : core::register_descriptor<std::uint32_t, 0x78U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0xFFFFFFFFU, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace nrfhw10_fields {
using nrfhw = core::field_descriptor<nrfhw10, 0U, 32U>;
}
struct nrfhw11 : core::register_descriptor<std::uint32_t, 0x7CU,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0xFFFFFFFFU, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace nrfhw11_fields {
using nrfhw = core::field_descriptor<nrfhw11, 0U, 32U>;
}
struct customer0 : core::register_descriptor<std::uint32_t, 0x80U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0xFFFFFFFFU, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace customer0_fields {
using customer = core::field_descriptor<customer0, 0U, 32U>;
}
struct customer1 : core::register_descriptor<std::uint32_t, 0x84U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0xFFFFFFFFU, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace customer1_fields {
using customer = core::field_descriptor<customer1, 0U, 32U>;
}
struct customer2 : core::register_descriptor<std::uint32_t, 0x88U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0xFFFFFFFFU, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace customer2_fields {
using customer = core::field_descriptor<customer2, 0U, 32U>;
}
struct customer3 : core::register_descriptor<std::uint32_t, 0x8CU,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0xFFFFFFFFU, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace customer3_fields {
using customer = core::field_descriptor<customer3, 0U, 32U>;
}
struct customer4 : core::register_descriptor<std::uint32_t, 0x90U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0xFFFFFFFFU, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace customer4_fields {
using customer = core::field_descriptor<customer4, 0U, 32U>;
}
struct customer5 : core::register_descriptor<std::uint32_t, 0x94U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0xFFFFFFFFU, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace customer5_fields {
using customer = core::field_descriptor<customer5, 0U, 32U>;
}
struct customer6 : core::register_descriptor<std::uint32_t, 0x98U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0xFFFFFFFFU, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace customer6_fields {
using customer = core::field_descriptor<customer6, 0U, 32U>;
}
struct customer7 : core::register_descriptor<std::uint32_t, 0x9CU,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0xFFFFFFFFU, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace customer7_fields {
using customer = core::field_descriptor<customer7, 0U, 32U>;
}
struct customer8 : core::register_descriptor<std::uint32_t, 0xA0U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0xFFFFFFFFU, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace customer8_fields {
using customer = core::field_descriptor<customer8, 0U, 32U>;
}
struct customer9 : core::register_descriptor<std::uint32_t, 0xA4U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0xFFFFFFFFU, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace customer9_fields {
using customer = core::field_descriptor<customer9, 0U, 32U>;
}
struct customer10 : core::register_descriptor<std::uint32_t, 0xA8U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0xFFFFFFFFU, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace customer10_fields {
using customer = core::field_descriptor<customer10, 0U, 32U>;
}
struct customer11 : core::register_descriptor<std::uint32_t, 0xACU,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0xFFFFFFFFU, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace customer11_fields {
using customer = core::field_descriptor<customer11, 0U, 32U>;
}
struct customer12 : core::register_descriptor<std::uint32_t, 0xB0U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0xFFFFFFFFU, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace customer12_fields {
using customer = core::field_descriptor<customer12, 0U, 32U>;
}
struct customer13 : core::register_descriptor<std::uint32_t, 0xB4U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0xFFFFFFFFU, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace customer13_fields {
using customer = core::field_descriptor<customer13, 0U, 32U>;
}
struct customer14 : core::register_descriptor<std::uint32_t, 0xB8U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0xFFFFFFFFU, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace customer14_fields {
using customer = core::field_descriptor<customer14, 0U, 32U>;
}
struct customer15 : core::register_descriptor<std::uint32_t, 0xBCU,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0xFFFFFFFFU, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace customer15_fields {
using customer = core::field_descriptor<customer15, 0U, 32U>;
}
struct customer16 : core::register_descriptor<std::uint32_t, 0xC0U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0xFFFFFFFFU, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace customer16_fields {
using customer = core::field_descriptor<customer16, 0U, 32U>;
}
struct customer17 : core::register_descriptor<std::uint32_t, 0xC4U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0xFFFFFFFFU, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace customer17_fields {
using customer = core::field_descriptor<customer17, 0U, 32U>;
}
struct customer18 : core::register_descriptor<std::uint32_t, 0xC8U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0xFFFFFFFFU, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace customer18_fields {
using customer = core::field_descriptor<customer18, 0U, 32U>;
}
struct customer19 : core::register_descriptor<std::uint32_t, 0xCCU,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0xFFFFFFFFU, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace customer19_fields {
using customer = core::field_descriptor<customer19, 0U, 32U>;
}
struct customer20 : core::register_descriptor<std::uint32_t, 0xD0U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0xFFFFFFFFU, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace customer20_fields {
using customer = core::field_descriptor<customer20, 0U, 32U>;
}
struct customer21 : core::register_descriptor<std::uint32_t, 0xD4U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0xFFFFFFFFU, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace customer21_fields {
using customer = core::field_descriptor<customer21, 0U, 32U>;
}
struct customer22 : core::register_descriptor<std::uint32_t, 0xD8U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0xFFFFFFFFU, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace customer22_fields {
using customer = core::field_descriptor<customer22, 0U, 32U>;
}
struct customer23 : core::register_descriptor<std::uint32_t, 0xDCU,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0xFFFFFFFFU, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace customer23_fields {
using customer = core::field_descriptor<customer23, 0U, 32U>;
}
struct customer24 : core::register_descriptor<std::uint32_t, 0xE0U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0xFFFFFFFFU, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace customer24_fields {
using customer = core::field_descriptor<customer24, 0U, 32U>;
}
struct customer25 : core::register_descriptor<std::uint32_t, 0xE4U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0xFFFFFFFFU, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace customer25_fields {
using customer = core::field_descriptor<customer25, 0U, 32U>;
}
struct customer26 : core::register_descriptor<std::uint32_t, 0xE8U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0xFFFFFFFFU, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace customer26_fields {
using customer = core::field_descriptor<customer26, 0U, 32U>;
}
struct customer27 : core::register_descriptor<std::uint32_t, 0xECU,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0xFFFFFFFFU, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace customer27_fields {
using customer = core::field_descriptor<customer27, 0U, 32U>;
}
struct customer28 : core::register_descriptor<std::uint32_t, 0xF0U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0xFFFFFFFFU, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace customer28_fields {
using customer = core::field_descriptor<customer28, 0U, 32U>;
}
struct customer29 : core::register_descriptor<std::uint32_t, 0xF4U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0xFFFFFFFFU, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace customer29_fields {
using customer = core::field_descriptor<customer29, 0U, 32U>;
}
struct customer30 : core::register_descriptor<std::uint32_t, 0xF8U,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0xFFFFFFFFU, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace customer30_fields {
using customer = core::field_descriptor<customer30, 0U, 32U>;
}
struct customer31 : core::register_descriptor<std::uint32_t, 0xFCU,
    core::access_semantics::read_write, 0xFFFFFFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0xFFFFFFFFU, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace customer31_fields {
using customer = core::field_descriptor<customer31, 0U, 32U>;
}
struct pselreset0 : core::register_descriptor<std::uint32_t, 0x200U,
    core::access_semantics::read_write, 0x8000003FU, false,
    core::reserved_bit_policy::write_zero, 0U, 0xFFFFFFFFU, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace pselreset0_fields {
using pin = core::field_descriptor<pselreset0, 0U, 5U>;
using port = core::field_descriptor<pselreset0, 5U, 1U>;
using connect = core::field_descriptor<pselreset0, 31U, 1U>;
inline constexpr std::uint32_t connect_disconnected = 0x1U;
inline constexpr std::uint32_t connect_connected = 0x0U;
}
struct pselreset1 : core::register_descriptor<std::uint32_t, 0x204U,
    core::access_semantics::read_write, 0x8000003FU, false,
    core::reserved_bit_policy::write_zero, 0U, 0xFFFFFFFFU, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace pselreset1_fields {
using pin = core::field_descriptor<pselreset1, 0U, 5U>;
using port = core::field_descriptor<pselreset1, 5U, 1U>;
using connect = core::field_descriptor<pselreset1, 31U, 1U>;
inline constexpr std::uint32_t connect_disconnected = 0x1U;
inline constexpr std::uint32_t connect_connected = 0x0U;
}
struct approtect : core::register_descriptor<std::uint32_t, 0x208U,
    core::access_semantics::read_write, 0x000000FFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0xFFFFFFFFU, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace approtect_fields {
using pall = core::field_descriptor<approtect, 0U, 8U>;
inline constexpr std::uint32_t pall_disabled = 0xFFU;
inline constexpr std::uint32_t pall_hwdisabled = 0x5AU;
inline constexpr std::uint32_t pall_enabled = 0x0U;
}
struct nfcpins : core::register_descriptor<std::uint32_t, 0x20CU,
    core::access_semantics::read_write, 0x00000001U, false,
    core::reserved_bit_policy::write_zero, 0U, 0xFFFFFFFFU, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace nfcpins_fields {
using protect = core::field_descriptor<nfcpins, 0U, 1U>;
inline constexpr std::uint32_t protect_disabled = 0x0U;
inline constexpr std::uint32_t protect_nfc = 0x1U;
}
struct debugctrl : core::register_descriptor<std::uint32_t, 0x210U,
    core::access_semantics::read_write, 0x0000FFFFU, false,
    core::reserved_bit_policy::write_zero, 0U, 0xFFFFFFFFU, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace debugctrl_fields {
using cpuniden = core::field_descriptor<debugctrl, 0U, 8U>;
inline constexpr std::uint32_t cpuniden_enabled = 0xFFU;
inline constexpr std::uint32_t cpuniden_disabled = 0x0U;
using cpufpben = core::field_descriptor<debugctrl, 8U, 8U>;
inline constexpr std::uint32_t cpufpben_enabled = 0xFFU;
inline constexpr std::uint32_t cpufpben_disabled = 0x0U;
}
struct regout0 : core::register_descriptor<std::uint32_t, 0x304U,
    core::access_semantics::read_write, 0x00000007U, false,
    core::reserved_bit_policy::write_zero, 0U, 0xFFFFFFFFU, 0xFFFFFFFFU> { static constexpr bool write_once = false; };
namespace regout0_fields {
using vout = core::field_descriptor<regout0, 0U, 3U>;
inline constexpr std::uint32_t vout_reg_1v8 = 0x0U;
inline constexpr std::uint32_t vout_reg_2v1 = 0x1U;
inline constexpr std::uint32_t vout_reg_2v4 = 0x2U;
inline constexpr std::uint32_t vout_reg_2v7 = 0x3U;
inline constexpr std::uint32_t vout_reg_3v0 = 0x4U;
inline constexpr std::uint32_t vout_reg_3v3 = 0x5U;
inline constexpr std::uint32_t vout_reg_default = 0x7U;
}
}
