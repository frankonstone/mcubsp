#pragma once
// @req-file REQ-0055
#include <array>
#include <cstdint>
#include <string_view>
#include <mcubsp/mcu/nrf52840/product/cc_aes.hpp>
#include <mcubsp/mcu/nrf52840/product/cc_chacha.hpp>
#include <mcubsp/mcu/nrf52840/product/cc_hash.hpp>
#include <mcubsp/mcu/nrf52840/product/cc_pka.hpp>
#include <mcubsp/mcu/nrf52840/product/cc_rng.hpp>
#include <mcubsp/mcu/nrf52840/product/cc_ctl.hpp>
#include <mcubsp/mcu/nrf52840/product/cc_din.hpp>
#include <mcubsp/mcu/nrf52840/product/cc_dout.hpp>
#include <mcubsp/mcu/nrf52840/product/cc_host_rgf.hpp>
#include <mcubsp/mcu/nrf52840/product/cc_rng_sram.hpp>
#include <mcubsp/mcu/nrf52840/product/cc_misc.hpp>
namespace mcubsp::mcu::nrf52840::product {
struct crypto_block { std::string_view name; std::uintptr_t base; unsigned registers, fields; };
inline constexpr std::array crypto_inventory{
    crypto_block{"CC_AES", 0x5002B000U, 25U, 39U},
    crypto_block{"CC_CHACHA", 0x5002B000U, 27U, 39U},
    crypto_block{"CC_HASH", 0x5002B000U, 19U, 30U},
    crypto_block{"CC_PKA", 0x5002B000U, 52U, 73U},
    crypto_block{"CC_RNG", 0x5002B000U, 26U, 57U},
    crypto_block{"CC_CTL", 0x5002B000U, 4U, 4U},
    crypto_block{"CC_DIN", 0x5002B000U, 13U, 15U},
    crypto_block{"CC_DOUT", 0x5002B000U, 11U, 13U},
    crypto_block{"CC_HOST_RGF", 0x5002B000U, 13U, 60U},
    crypto_block{"CC_RNG_SRAM", 0x5002B000U, 3U, 3U},
    crypto_block{"CC_MISC", 0x5002B000U, 6U, 10U},
};
}
