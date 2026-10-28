// @req-file REQ-0055 REQ-0058
#include <mcubsp/mcu/nrf52840/catalogue.hpp>
#include <mcubsp/mcu/nrf52840/product.hpp>
#include <mcubsp/mcu/nrf52840/system.hpp>
namespace n = mcubsp::mcu::nrf52840;
static_assert(n::instance_inventory.size() == 74);
static_assert(n::registers::gpiote::config0_fields::port::encode_constant<1>() == 0x2000);
static_assert(n::registers::power::ram8_power::offset == 0x980);
static_assert(n::timer_channels[0] == 4 && n::timer_channels[3] == 6);
static_assert(n::instances::uarte1::base == 0x40028000);
static_assert(n::product::crypto_inventory.size() == 11);
static_assert(n::product::cc_pka::opcode::offset == 0x80);
static_assert(n::product::cc_chacha::chacha_control_fields::use_iv_96bit::mask == 0x400);
