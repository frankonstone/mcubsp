// @req-file REQ-0055 REQ-0058
#include <mcubsp/mcu/nrf52840/catalogue.hpp>
#include <mcubsp/mcu/nrf52840/product.hpp>
#include <mcubsp/mcu/nrf52840/system.hpp>
#include <mcubsp/mcu/nrf52840/memory.hpp>
#include <mcubsp/mcu/nrf52840/core_registers.hpp>
#include <mcubsp/mcu/nrf52840/resources.hpp>
namespace n = mcubsp::mcu::nrf52840;
static_assert(n::instance_inventory.size() == 74);
static_assert(n::registers::gpiote::config0_fields::port::encode_constant<1>() == 0x2000);
static_assert(n::registers::power::ram8_power::offset == 0x980);
static_assert(n::timer_channels[0] == 4 && n::timer_channels[3] == 6);
static_assert(n::instances::uarte1::base == 0x40028000);
static_assert(n::product::crypto_inventory.size() == 11);
static_assert(n::product::cc_pka::opcode::offset == 0x80);
static_assert(n::product::cc_chacha::chacha_control_fields::use_iv_96bit::mask == 0x400);
static_assert(n::product::instances::fpu::irq == 38);
static_assert(n::product::clock::lfrcmode::writable_mask == 1);
static_assert(n::product::gpio::out_fields::pin31::mask == 0x80000000);
static_assert(n::product::package_pin<n::product::package::aqfn73, {0, 15}>::id.index == 15);
static_assert(!n::product::bonded(n::product::package::qfn48, {0, 15}));
static_assert(!n::memory::easydma_buffer(0x00800000, 4));
static_assert(n::core_registers::debug::dcrsr_fields::regsel::width == 7);
static_assert(n::core_registers::tpi::devtype_fields::subtype::mask == 0xF0);
static_assert(n::core_registers::fpb::ctrl::required_one_mask == 2);
static_assert(n::core_registers::itm::port0_u8::width == 1);
static_assert(n::ownership_identity(n::peripheral::twis1) == n::ownership_identity(n::peripheral::spim1));
static_assert(n::ownership_identity(n::peripheral::power) != n::ownership_identity(n::peripheral::clock));
static_assert(n::serial_dma_constraints<n::peripheral::spim3>.maximum_count == 65535);
