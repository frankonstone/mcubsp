#pragma once
// @req-file REQ-0055
#include <array>
#include <cstdint>
#include <string_view>
#include <mcubsp/mcu/nrf52840/product/nvmc.hpp>
#include <mcubsp/mcu/nrf52840/product/ficr.hpp>
#include <mcubsp/mcu/nrf52840/product/uicr.hpp>
#include <mcubsp/mcu/nrf52840/product/approtect.hpp>
#include <mcubsp/mcu/nrf52840/product/ctrl_ap.hpp>
#include <mcubsp/mcu/nrf52840/product/power.hpp>
#include <mcubsp/mcu/nrf52840/product/clock.hpp>
#include <mcubsp/mcu/nrf52840/product/aar.hpp>
#include <mcubsp/mcu/nrf52840/product/acl.hpp>
#include <mcubsp/mcu/nrf52840/product/ccm.hpp>
#include <mcubsp/mcu/nrf52840/product/comp.hpp>
#include <mcubsp/mcu/nrf52840/product/cryptocell.hpp>
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
#include <mcubsp/mcu/nrf52840/product/ecb.hpp>
#include <mcubsp/mcu/nrf52840/product/egu0.hpp>
#include <mcubsp/mcu/nrf52840/product/gpio.hpp>
#include <mcubsp/mcu/nrf52840/product/gpiote.hpp>
#include <mcubsp/mcu/nrf52840/product/i2s.hpp>
#include <mcubsp/mcu/nrf52840/product/lpcomp.hpp>
#include <mcubsp/mcu/nrf52840/product/mwu.hpp>
#include <mcubsp/mcu/nrf52840/product/nfct.hpp>
#include <mcubsp/mcu/nrf52840/product/pdm.hpp>
#include <mcubsp/mcu/nrf52840/product/ppi.hpp>
#include <mcubsp/mcu/nrf52840/product/pwm0.hpp>
#include <mcubsp/mcu/nrf52840/product/qdec.hpp>
#include <mcubsp/mcu/nrf52840/product/qspi.hpp>
#include <mcubsp/mcu/nrf52840/product/radio.hpp>
#include <mcubsp/mcu/nrf52840/product/rng.hpp>
#include <mcubsp/mcu/nrf52840/product/rtc0.hpp>
#include <mcubsp/mcu/nrf52840/product/saadc.hpp>
#include <mcubsp/mcu/nrf52840/product/spi0.hpp>
#include <mcubsp/mcu/nrf52840/product/spim0.hpp>
#include <mcubsp/mcu/nrf52840/product/spis0.hpp>
#include <mcubsp/mcu/nrf52840/product/swi0.hpp>
#include <mcubsp/mcu/nrf52840/product/temp.hpp>
#include <mcubsp/mcu/nrf52840/product/twi0.hpp>
#include <mcubsp/mcu/nrf52840/product/timer0.hpp>
#include <mcubsp/mcu/nrf52840/product/twim0.hpp>
#include <mcubsp/mcu/nrf52840/product/twis0.hpp>
#include <mcubsp/mcu/nrf52840/product/uart0.hpp>
#include <mcubsp/mcu/nrf52840/product/uarte0.hpp>
#include <mcubsp/mcu/nrf52840/product/usbd.hpp>
#include <mcubsp/mcu/nrf52840/product/wdt.hpp>
#include <mcubsp/mcu/nrf52840/product/pins.hpp>
namespace mcubsp::mcu::nrf52840::product {
struct product_block { std::string_view name; unsigned registers, fields; bool debug_access_port; };
inline constexpr std::array block_inventory{
    product_block{"NVMC", 11U, 12U, false},
    product_block{"FICR", 52U, 64U, false},
    product_block{"UICR", 63U, 68U, false},
    product_block{"APPROTECT", 2U, 2U, false},
    product_block{"CTRL_AP", 5U, 9U, true},
    product_block{"POWER", 47U, 908U, false},
    product_block{"CLOCK", 25U, 41U, false},
    product_block{"AAR", 13U, 17U, false},
    product_block{"ACL", 24U, 32U, false},
    product_block{"CCM", 19U, 25U, false},
    product_block{"COMP", 19U, 34U, false},
    product_block{"CRYPTOCELL", 1U, 1U, false},
    product_block{"CC_AES", 25U, 39U, false},
    product_block{"CC_CHACHA", 27U, 39U, false},
    product_block{"CC_HASH", 19U, 30U, false},
    product_block{"CC_PKA", 52U, 73U, false},
    product_block{"CC_RNG", 26U, 57U, false},
    product_block{"CC_CTL", 4U, 4U, false},
    product_block{"CC_DIN", 13U, 15U, false},
    product_block{"CC_DOUT", 11U, 13U, false},
    product_block{"CC_HOST_RGF", 13U, 62U, false},
    product_block{"CC_RNG_SRAM", 3U, 3U, false},
    product_block{"CC_MISC", 6U, 10U, false},
    product_block{"ECB", 7U, 9U, false},
    product_block{"EGU0", 35U, 80U, false},
    product_block{"GPIO", 41U, 417U, false},
    product_block{"GPIOTE", 43U, 91U, false},
    product_block{"I2S", 27U, 43U, false},
    product_block{"LPCOMP", 17U, 27U, false},
    product_block{"MWU", 39U, 324U, false},
    product_block{"NFCT", 46U, 115U, false},
    product_block{"PDM", 18U, 29U, false},
    product_block{"PPI", 93U, 372U, false},
    product_block{"PWM0", 33U, 64U, false},
    product_block{"QDEC", 27U, 47U, false},
    product_block{"QSPI", 34U, 74U, false},
    product_block{"RADIO", 89U, 197U, false},
    product_block{"RNG", 8U, 8U, false},
    product_block{"RTC0", 21U, 46U, false},
    product_block{"SAADC", 69U, 189U, false},
    product_block{"SPI0", 11U, 19U, false},
    product_block{"SPIM0", 34U, 55U, false},
    product_block{"SPIS0", 26U, 41U, false},
    product_block{"SWI0", 0U, 0U, false},
    product_block{"TEMP", 23U, 23U, false},
    product_block{"TWI0", 22U, 39U, false},
    product_block{"TIMER0", 29U, 50U, false},
    product_block{"TWIM0", 30U, 59U, false},
    product_block{"TWIS0", 32U, 55U, false},
    product_block{"UART0", 24U, 48U, false},
    product_block{"UARTE0", 34U, 78U, false},
    product_block{"USBD", 154U, 287U, false},
    product_block{"WDT", 17U, 32U, false},
};
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
    crypto_block{"CC_HOST_RGF", 0x5002B000U, 13U, 62U},
    crypto_block{"CC_RNG_SRAM", 0x5002B000U, 3U, 3U},
    crypto_block{"CC_MISC", 0x5002B000U, 6U, 10U},
};
namespace instances {
struct approtect { static constexpr std::uintptr_t base = 0x40000000U; static constexpr int irq = -1; };
struct clock { static constexpr std::uintptr_t base = 0x40000000U; static constexpr int irq = 0; };
struct power { static constexpr std::uintptr_t base = 0x40000000U; static constexpr int irq = 0; };
struct gpio { static constexpr std::uintptr_t base = 0x50000000U; static constexpr int irq = -1; };
struct p0 { static constexpr std::uintptr_t base = 0x50000000U; static constexpr int irq = -1; };
struct p1 { static constexpr std::uintptr_t base = 0x50000300U; static constexpr int irq = -1; };
struct radio { static constexpr std::uintptr_t base = 0x40001000U; static constexpr int irq = 1; };
struct uart0 { static constexpr std::uintptr_t base = 0x40002000U; static constexpr int irq = 2; };
struct uarte0 { static constexpr std::uintptr_t base = 0x40002000U; static constexpr int irq = 2; };
struct spi0 { static constexpr std::uintptr_t base = 0x40003000U; static constexpr int irq = 3; };
struct spim0 { static constexpr std::uintptr_t base = 0x40003000U; static constexpr int irq = 3; };
struct spis0 { static constexpr std::uintptr_t base = 0x40003000U; static constexpr int irq = 3; };
struct twi0 { static constexpr std::uintptr_t base = 0x40003000U; static constexpr int irq = 3; };
struct twim0 { static constexpr std::uintptr_t base = 0x40003000U; static constexpr int irq = 3; };
struct twis0 { static constexpr std::uintptr_t base = 0x40003000U; static constexpr int irq = 3; };
struct spi1 { static constexpr std::uintptr_t base = 0x40004000U; static constexpr int irq = 4; };
struct spim1 { static constexpr std::uintptr_t base = 0x40004000U; static constexpr int irq = 4; };
struct spis1 { static constexpr std::uintptr_t base = 0x40004000U; static constexpr int irq = 4; };
struct twi1 { static constexpr std::uintptr_t base = 0x40004000U; static constexpr int irq = 4; };
struct twim1 { static constexpr std::uintptr_t base = 0x40004000U; static constexpr int irq = 4; };
struct twis1 { static constexpr std::uintptr_t base = 0x40004000U; static constexpr int irq = 4; };
struct nfct { static constexpr std::uintptr_t base = 0x40005000U; static constexpr int irq = 5; };
struct gpiote { static constexpr std::uintptr_t base = 0x40006000U; static constexpr int irq = 6; };
struct saadc { static constexpr std::uintptr_t base = 0x40007000U; static constexpr int irq = 7; };
struct timer0 { static constexpr std::uintptr_t base = 0x40008000U; static constexpr int irq = 8; };
struct timer1 { static constexpr std::uintptr_t base = 0x40009000U; static constexpr int irq = 9; };
struct timer2 { static constexpr std::uintptr_t base = 0x4000A000U; static constexpr int irq = 10; };
struct rtc0 { static constexpr std::uintptr_t base = 0x4000B000U; static constexpr int irq = 11; };
struct temp { static constexpr std::uintptr_t base = 0x4000C000U; static constexpr int irq = 12; };
struct rng { static constexpr std::uintptr_t base = 0x4000D000U; static constexpr int irq = 13; };
struct ecb { static constexpr std::uintptr_t base = 0x4000E000U; static constexpr int irq = 14; };
struct aar { static constexpr std::uintptr_t base = 0x4000F000U; static constexpr int irq = 15; };
struct ccm { static constexpr std::uintptr_t base = 0x4000F000U; static constexpr int irq = 15; };
struct wdt { static constexpr std::uintptr_t base = 0x40010000U; static constexpr int irq = 16; };
struct rtc1 { static constexpr std::uintptr_t base = 0x40011000U; static constexpr int irq = 17; };
struct qdec { static constexpr std::uintptr_t base = 0x40012000U; static constexpr int irq = 18; };
struct comp { static constexpr std::uintptr_t base = 0x40013000U; static constexpr int irq = 19; };
struct lpcomp { static constexpr std::uintptr_t base = 0x40013000U; static constexpr int irq = 19; };
struct egu0 { static constexpr std::uintptr_t base = 0x40014000U; static constexpr int irq = 20; };
struct swi0 { static constexpr std::uintptr_t base = 0x40014000U; static constexpr int irq = 20; };
struct egu1 { static constexpr std::uintptr_t base = 0x40015000U; static constexpr int irq = 21; };
struct swi1 { static constexpr std::uintptr_t base = 0x40015000U; static constexpr int irq = 21; };
struct egu2 { static constexpr std::uintptr_t base = 0x40016000U; static constexpr int irq = 22; };
struct swi2 { static constexpr std::uintptr_t base = 0x40016000U; static constexpr int irq = 22; };
struct egu3 { static constexpr std::uintptr_t base = 0x40017000U; static constexpr int irq = 23; };
struct swi3 { static constexpr std::uintptr_t base = 0x40017000U; static constexpr int irq = 23; };
struct egu4 { static constexpr std::uintptr_t base = 0x40018000U; static constexpr int irq = 24; };
struct swi4 { static constexpr std::uintptr_t base = 0x40018000U; static constexpr int irq = 24; };
struct egu5 { static constexpr std::uintptr_t base = 0x40019000U; static constexpr int irq = 25; };
struct swi5 { static constexpr std::uintptr_t base = 0x40019000U; static constexpr int irq = 25; };
struct timer3 { static constexpr std::uintptr_t base = 0x4001A000U; static constexpr int irq = 26; };
struct timer4 { static constexpr std::uintptr_t base = 0x4001B000U; static constexpr int irq = 27; };
struct pwm0 { static constexpr std::uintptr_t base = 0x4001C000U; static constexpr int irq = 28; };
struct pdm { static constexpr std::uintptr_t base = 0x4001D000U; static constexpr int irq = 29; };
struct acl { static constexpr std::uintptr_t base = 0x4001E000U; static constexpr int irq = -1; };
struct nvmc { static constexpr std::uintptr_t base = 0x4001E000U; static constexpr int irq = -1; };
struct ppi { static constexpr std::uintptr_t base = 0x4001F000U; static constexpr int irq = -1; };
struct mwu { static constexpr std::uintptr_t base = 0x40020000U; static constexpr int irq = 32; };
struct pwm1 { static constexpr std::uintptr_t base = 0x40021000U; static constexpr int irq = 33; };
struct pwm2 { static constexpr std::uintptr_t base = 0x40022000U; static constexpr int irq = 34; };
struct spi2 { static constexpr std::uintptr_t base = 0x40023000U; static constexpr int irq = 35; };
struct spim2 { static constexpr std::uintptr_t base = 0x40023000U; static constexpr int irq = 35; };
struct spis2 { static constexpr std::uintptr_t base = 0x40023000U; static constexpr int irq = 35; };
struct rtc2 { static constexpr std::uintptr_t base = 0x40024000U; static constexpr int irq = 36; };
struct i2s { static constexpr std::uintptr_t base = 0x40025000U; static constexpr int irq = 37; };
struct fpu { static constexpr std::uintptr_t base = 0x40026000U; static constexpr int irq = 38; };
struct usbd { static constexpr std::uintptr_t base = 0x40027000U; static constexpr int irq = 39; };
struct uarte1 { static constexpr std::uintptr_t base = 0x40028000U; static constexpr int irq = 40; };
struct qspi { static constexpr std::uintptr_t base = 0x40029000U; static constexpr int irq = 41; };
struct cryptocell { static constexpr std::uintptr_t base = 0x5002A000U; static constexpr int irq = 42; };
struct cc_aes { static constexpr std::uintptr_t base = 0x5002B000U; static constexpr int irq = -1; };
struct cc_chacha { static constexpr std::uintptr_t base = 0x5002B000U; static constexpr int irq = -1; };
struct cc_ctl { static constexpr std::uintptr_t base = 0x5002B000U; static constexpr int irq = -1; };
struct cc_din { static constexpr std::uintptr_t base = 0x5002B000U; static constexpr int irq = -1; };
struct cc_dout { static constexpr std::uintptr_t base = 0x5002B000U; static constexpr int irq = -1; };
struct cc_hash { static constexpr std::uintptr_t base = 0x5002B000U; static constexpr int irq = -1; };
struct cc_host_rgf { static constexpr std::uintptr_t base = 0x5002B000U; static constexpr int irq = -1; };
struct cc_misc { static constexpr std::uintptr_t base = 0x5002B000U; static constexpr int irq = -1; };
struct cc_pka { static constexpr std::uintptr_t base = 0x5002B000U; static constexpr int irq = -1; };
struct cc_rng { static constexpr std::uintptr_t base = 0x5002B000U; static constexpr int irq = -1; };
struct cc_rng_sram { static constexpr std::uintptr_t base = 0x5002B000U; static constexpr int irq = -1; };
struct pwm3 { static constexpr std::uintptr_t base = 0x4002D000U; static constexpr int irq = 45; };
struct spim3 { static constexpr std::uintptr_t base = 0x4002F000U; static constexpr int irq = 47; };
struct ficr { static constexpr std::uintptr_t base = 0x10000000U; static constexpr int irq = -1; };
struct uicr { static constexpr std::uintptr_t base = 0x10001000U; static constexpr int irq = -1; };
}
}
