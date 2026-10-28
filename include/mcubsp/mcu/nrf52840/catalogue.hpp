#pragma once

// @req-file REQ-0055
#include <array>
#include <cstdint>
#include <string_view>
#include <mcubsp/mcu/nrf52840/registers/ficr.hpp>
#include <mcubsp/mcu/nrf52840/registers/uicr.hpp>
#include <mcubsp/mcu/nrf52840/registers/approtect.hpp>
#include <mcubsp/mcu/nrf52840/registers/clock.hpp>
#include <mcubsp/mcu/nrf52840/registers/power.hpp>
#include <mcubsp/mcu/nrf52840/registers/p0.hpp>
#include <mcubsp/mcu/nrf52840/registers/radio.hpp>
#include <mcubsp/mcu/nrf52840/registers/uart0.hpp>
#include <mcubsp/mcu/nrf52840/registers/uarte0.hpp>
#include <mcubsp/mcu/nrf52840/registers/spi0.hpp>
#include <mcubsp/mcu/nrf52840/registers/spim0.hpp>
#include <mcubsp/mcu/nrf52840/registers/spis0.hpp>
#include <mcubsp/mcu/nrf52840/registers/twi0.hpp>
#include <mcubsp/mcu/nrf52840/registers/twim0.hpp>
#include <mcubsp/mcu/nrf52840/registers/twis0.hpp>
#include <mcubsp/mcu/nrf52840/registers/nfct.hpp>
#include <mcubsp/mcu/nrf52840/registers/gpiote.hpp>
#include <mcubsp/mcu/nrf52840/registers/saadc.hpp>
#include <mcubsp/mcu/nrf52840/registers/timer0.hpp>
#include <mcubsp/mcu/nrf52840/registers/rtc0.hpp>
#include <mcubsp/mcu/nrf52840/registers/temp.hpp>
#include <mcubsp/mcu/nrf52840/registers/rng.hpp>
#include <mcubsp/mcu/nrf52840/registers/ecb.hpp>
#include <mcubsp/mcu/nrf52840/registers/aar.hpp>
#include <mcubsp/mcu/nrf52840/registers/ccm.hpp>
#include <mcubsp/mcu/nrf52840/registers/wdt.hpp>
#include <mcubsp/mcu/nrf52840/registers/qdec.hpp>
#include <mcubsp/mcu/nrf52840/registers/comp.hpp>
#include <mcubsp/mcu/nrf52840/registers/lpcomp.hpp>
#include <mcubsp/mcu/nrf52840/registers/egu0.hpp>
#include <mcubsp/mcu/nrf52840/registers/swi0.hpp>
#include <mcubsp/mcu/nrf52840/registers/pwm0.hpp>
#include <mcubsp/mcu/nrf52840/registers/pdm.hpp>
#include <mcubsp/mcu/nrf52840/registers/acl.hpp>
#include <mcubsp/mcu/nrf52840/registers/nvmc.hpp>
#include <mcubsp/mcu/nrf52840/registers/ppi.hpp>
#include <mcubsp/mcu/nrf52840/registers/mwu.hpp>
#include <mcubsp/mcu/nrf52840/registers/i2s.hpp>
#include <mcubsp/mcu/nrf52840/registers/fpu.hpp>
#include <mcubsp/mcu/nrf52840/registers/usbd.hpp>
#include <mcubsp/mcu/nrf52840/registers/qspi.hpp>
#include <mcubsp/mcu/nrf52840/registers/cc_host_rgf.hpp>
#include <mcubsp/mcu/nrf52840/registers/cryptocell.hpp>

namespace mcubsp::mcu::nrf52840 {
struct instance_description { std::string_view name; std::uintptr_t base; int irq; unsigned registers; unsigned fields; };
inline constexpr std::array instance_inventory{
    instance_description{"FICR", 0x10000000U, -1, 52U, 64U},
    instance_description{"UICR", 0x10001000U, -1, 63U, 68U},
    instance_description{"APPROTECT", 0x40000000U, -1, 2U, 2U},
    instance_description{"CLOCK", 0x40000000U, 0, 25U, 41U},
    instance_description{"POWER", 0x40000000U, 0, 47U, 908U},
    instance_description{"P0", 0x50000000U, -1, 41U, 417U},
    instance_description{"P1", 0x50000300U, -1, 41U, 417U},
    instance_description{"RADIO", 0x40001000U, 1, 89U, 197U},
    instance_description{"UART0", 0x40002000U, 2, 24U, 48U},
    instance_description{"UARTE0", 0x40002000U, 2, 34U, 78U},
    instance_description{"SPI0", 0x40003000U, 3, 11U, 19U},
    instance_description{"SPIM0", 0x40003000U, 3, 34U, 55U},
    instance_description{"SPIS0", 0x40003000U, 3, 26U, 41U},
    instance_description{"TWI0", 0x40003000U, 3, 22U, 39U},
    instance_description{"TWIM0", 0x40003000U, 3, 30U, 59U},
    instance_description{"TWIS0", 0x40003000U, 3, 32U, 55U},
    instance_description{"SPI1", 0x40004000U, 4, 11U, 19U},
    instance_description{"SPIM1", 0x40004000U, 4, 34U, 55U},
    instance_description{"SPIS1", 0x40004000U, 4, 26U, 41U},
    instance_description{"TWI1", 0x40004000U, 4, 22U, 39U},
    instance_description{"TWIM1", 0x40004000U, 4, 30U, 59U},
    instance_description{"TWIS1", 0x40004000U, 4, 32U, 55U},
    instance_description{"NFCT", 0x40005000U, 5, 46U, 115U},
    instance_description{"GPIOTE", 0x40006000U, 6, 43U, 91U},
    instance_description{"SAADC", 0x40007000U, 7, 69U, 189U},
    instance_description{"TIMER0", 0x40008000U, 8, 29U, 50U},
    instance_description{"TIMER1", 0x40009000U, 9, 29U, 50U},
    instance_description{"TIMER2", 0x4000A000U, 10, 29U, 50U},
    instance_description{"RTC0", 0x4000B000U, 11, 21U, 46U},
    instance_description{"TEMP", 0x4000C000U, 12, 23U, 23U},
    instance_description{"RNG", 0x4000D000U, 13, 8U, 8U},
    instance_description{"ECB", 0x4000E000U, 14, 7U, 9U},
    instance_description{"AAR", 0x4000F000U, 15, 13U, 17U},
    instance_description{"CCM", 0x4000F000U, 15, 19U, 25U},
    instance_description{"WDT", 0x40010000U, 16, 17U, 32U},
    instance_description{"RTC1", 0x40011000U, 17, 21U, 46U},
    instance_description{"QDEC", 0x40012000U, 18, 27U, 47U},
    instance_description{"COMP", 0x40013000U, 19, 19U, 34U},
    instance_description{"LPCOMP", 0x40013000U, 19, 17U, 27U},
    instance_description{"EGU0", 0x40014000U, 20, 35U, 80U},
    instance_description{"SWI0", 0x40014000U, 20, 1U, 0U},
    instance_description{"EGU1", 0x40015000U, 21, 35U, 80U},
    instance_description{"SWI1", 0x40015000U, 21, 1U, 0U},
    instance_description{"EGU2", 0x40016000U, 22, 35U, 80U},
    instance_description{"SWI2", 0x40016000U, 22, 1U, 0U},
    instance_description{"EGU3", 0x40017000U, 23, 35U, 80U},
    instance_description{"SWI3", 0x40017000U, 23, 1U, 0U},
    instance_description{"EGU4", 0x40018000U, 24, 35U, 80U},
    instance_description{"SWI4", 0x40018000U, 24, 1U, 0U},
    instance_description{"EGU5", 0x40019000U, 25, 35U, 80U},
    instance_description{"SWI5", 0x40019000U, 25, 1U, 0U},
    instance_description{"TIMER3", 0x4001A000U, 26, 29U, 50U},
    instance_description{"TIMER4", 0x4001B000U, 27, 29U, 50U},
    instance_description{"PWM0", 0x4001C000U, 28, 33U, 64U},
    instance_description{"PDM", 0x4001D000U, 29, 18U, 29U},
    instance_description{"ACL", 0x4001E000U, -1, 24U, 32U},
    instance_description{"NVMC", 0x4001E000U, -1, 13U, 14U},
    instance_description{"PPI", 0x4001F000U, -1, 93U, 372U},
    instance_description{"MWU", 0x40020000U, 32, 39U, 324U},
    instance_description{"PWM1", 0x40021000U, 33, 33U, 64U},
    instance_description{"PWM2", 0x40022000U, 34, 33U, 64U},
    instance_description{"SPI2", 0x40023000U, 35, 11U, 19U},
    instance_description{"SPIM2", 0x40023000U, 35, 34U, 55U},
    instance_description{"SPIS2", 0x40023000U, 35, 26U, 41U},
    instance_description{"RTC2", 0x40024000U, 36, 21U, 46U},
    instance_description{"I2S", 0x40025000U, 37, 27U, 43U},
    instance_description{"FPU", 0x40026000U, 38, 1U, 0U},
    instance_description{"USBD", 0x40027000U, 39, 154U, 287U},
    instance_description{"UARTE1", 0x40028000U, 40, 34U, 78U},
    instance_description{"QSPI", 0x40029000U, 41, 34U, 74U},
    instance_description{"CC_HOST_RGF", 0x5002A000U, -1, 7U, 8U},
    instance_description{"CRYPTOCELL", 0x5002A000U, 42, 1U, 1U},
    instance_description{"PWM3", 0x4002D000U, 45, 33U, 64U},
    instance_description{"SPIM3", 0x4002F000U, 47, 34U, 55U},
};
namespace instances {
struct ficr {
    static constexpr std::uintptr_t base = 0x10000000U;
    static constexpr int irq = -1;
};
struct uicr {
    static constexpr std::uintptr_t base = 0x10001000U;
    static constexpr int irq = -1;
};
struct approtect {
    static constexpr std::uintptr_t base = 0x40000000U;
    static constexpr int irq = -1;
};
struct clock {
    static constexpr std::uintptr_t base = 0x40000000U;
    static constexpr int irq = 0;
};
struct power {
    static constexpr std::uintptr_t base = 0x40000000U;
    static constexpr int irq = 0;
};
struct p0 {
    static constexpr std::uintptr_t base = 0x50000000U;
    static constexpr int irq = -1;
};
struct p1 {
    static constexpr std::uintptr_t base = 0x50000300U;
    static constexpr int irq = -1;
};
struct radio {
    static constexpr std::uintptr_t base = 0x40001000U;
    static constexpr int irq = 1;
};
struct uart0 {
    static constexpr std::uintptr_t base = 0x40002000U;
    static constexpr int irq = 2;
};
struct uarte0 {
    static constexpr std::uintptr_t base = 0x40002000U;
    static constexpr int irq = 2;
};
struct spi0 {
    static constexpr std::uintptr_t base = 0x40003000U;
    static constexpr int irq = 3;
};
struct spim0 {
    static constexpr std::uintptr_t base = 0x40003000U;
    static constexpr int irq = 3;
};
struct spis0 {
    static constexpr std::uintptr_t base = 0x40003000U;
    static constexpr int irq = 3;
};
struct twi0 {
    static constexpr std::uintptr_t base = 0x40003000U;
    static constexpr int irq = 3;
};
struct twim0 {
    static constexpr std::uintptr_t base = 0x40003000U;
    static constexpr int irq = 3;
};
struct twis0 {
    static constexpr std::uintptr_t base = 0x40003000U;
    static constexpr int irq = 3;
};
struct spi1 {
    static constexpr std::uintptr_t base = 0x40004000U;
    static constexpr int irq = 4;
};
struct spim1 {
    static constexpr std::uintptr_t base = 0x40004000U;
    static constexpr int irq = 4;
};
struct spis1 {
    static constexpr std::uintptr_t base = 0x40004000U;
    static constexpr int irq = 4;
};
struct twi1 {
    static constexpr std::uintptr_t base = 0x40004000U;
    static constexpr int irq = 4;
};
struct twim1 {
    static constexpr std::uintptr_t base = 0x40004000U;
    static constexpr int irq = 4;
};
struct twis1 {
    static constexpr std::uintptr_t base = 0x40004000U;
    static constexpr int irq = 4;
};
struct nfct {
    static constexpr std::uintptr_t base = 0x40005000U;
    static constexpr int irq = 5;
};
struct gpiote {
    static constexpr std::uintptr_t base = 0x40006000U;
    static constexpr int irq = 6;
};
struct saadc {
    static constexpr std::uintptr_t base = 0x40007000U;
    static constexpr int irq = 7;
};
struct timer0 {
    static constexpr std::uintptr_t base = 0x40008000U;
    static constexpr int irq = 8;
};
struct timer1 {
    static constexpr std::uintptr_t base = 0x40009000U;
    static constexpr int irq = 9;
};
struct timer2 {
    static constexpr std::uintptr_t base = 0x4000A000U;
    static constexpr int irq = 10;
};
struct rtc0 {
    static constexpr std::uintptr_t base = 0x4000B000U;
    static constexpr int irq = 11;
};
struct temp {
    static constexpr std::uintptr_t base = 0x4000C000U;
    static constexpr int irq = 12;
};
struct rng {
    static constexpr std::uintptr_t base = 0x4000D000U;
    static constexpr int irq = 13;
};
struct ecb {
    static constexpr std::uintptr_t base = 0x4000E000U;
    static constexpr int irq = 14;
};
struct aar {
    static constexpr std::uintptr_t base = 0x4000F000U;
    static constexpr int irq = 15;
};
struct ccm {
    static constexpr std::uintptr_t base = 0x4000F000U;
    static constexpr int irq = 15;
};
struct wdt {
    static constexpr std::uintptr_t base = 0x40010000U;
    static constexpr int irq = 16;
};
struct rtc1 {
    static constexpr std::uintptr_t base = 0x40011000U;
    static constexpr int irq = 17;
};
struct qdec {
    static constexpr std::uintptr_t base = 0x40012000U;
    static constexpr int irq = 18;
};
struct comp {
    static constexpr std::uintptr_t base = 0x40013000U;
    static constexpr int irq = 19;
};
struct lpcomp {
    static constexpr std::uintptr_t base = 0x40013000U;
    static constexpr int irq = 19;
};
struct egu0 {
    static constexpr std::uintptr_t base = 0x40014000U;
    static constexpr int irq = 20;
};
struct swi0 {
    static constexpr std::uintptr_t base = 0x40014000U;
    static constexpr int irq = 20;
};
struct egu1 {
    static constexpr std::uintptr_t base = 0x40015000U;
    static constexpr int irq = 21;
};
struct swi1 {
    static constexpr std::uintptr_t base = 0x40015000U;
    static constexpr int irq = 21;
};
struct egu2 {
    static constexpr std::uintptr_t base = 0x40016000U;
    static constexpr int irq = 22;
};
struct swi2 {
    static constexpr std::uintptr_t base = 0x40016000U;
    static constexpr int irq = 22;
};
struct egu3 {
    static constexpr std::uintptr_t base = 0x40017000U;
    static constexpr int irq = 23;
};
struct swi3 {
    static constexpr std::uintptr_t base = 0x40017000U;
    static constexpr int irq = 23;
};
struct egu4 {
    static constexpr std::uintptr_t base = 0x40018000U;
    static constexpr int irq = 24;
};
struct swi4 {
    static constexpr std::uintptr_t base = 0x40018000U;
    static constexpr int irq = 24;
};
struct egu5 {
    static constexpr std::uintptr_t base = 0x40019000U;
    static constexpr int irq = 25;
};
struct swi5 {
    static constexpr std::uintptr_t base = 0x40019000U;
    static constexpr int irq = 25;
};
struct timer3 {
    static constexpr std::uintptr_t base = 0x4001A000U;
    static constexpr int irq = 26;
};
struct timer4 {
    static constexpr std::uintptr_t base = 0x4001B000U;
    static constexpr int irq = 27;
};
struct pwm0 {
    static constexpr std::uintptr_t base = 0x4001C000U;
    static constexpr int irq = 28;
};
struct pdm {
    static constexpr std::uintptr_t base = 0x4001D000U;
    static constexpr int irq = 29;
};
struct acl {
    static constexpr std::uintptr_t base = 0x4001E000U;
    static constexpr int irq = -1;
};
struct nvmc {
    static constexpr std::uintptr_t base = 0x4001E000U;
    static constexpr int irq = -1;
};
struct ppi {
    static constexpr std::uintptr_t base = 0x4001F000U;
    static constexpr int irq = -1;
};
struct mwu {
    static constexpr std::uintptr_t base = 0x40020000U;
    static constexpr int irq = 32;
};
struct pwm1 {
    static constexpr std::uintptr_t base = 0x40021000U;
    static constexpr int irq = 33;
};
struct pwm2 {
    static constexpr std::uintptr_t base = 0x40022000U;
    static constexpr int irq = 34;
};
struct spi2 {
    static constexpr std::uintptr_t base = 0x40023000U;
    static constexpr int irq = 35;
};
struct spim2 {
    static constexpr std::uintptr_t base = 0x40023000U;
    static constexpr int irq = 35;
};
struct spis2 {
    static constexpr std::uintptr_t base = 0x40023000U;
    static constexpr int irq = 35;
};
struct rtc2 {
    static constexpr std::uintptr_t base = 0x40024000U;
    static constexpr int irq = 36;
};
struct i2s {
    static constexpr std::uintptr_t base = 0x40025000U;
    static constexpr int irq = 37;
};
struct fpu {
    static constexpr std::uintptr_t base = 0x40026000U;
    static constexpr int irq = 38;
};
struct usbd {
    static constexpr std::uintptr_t base = 0x40027000U;
    static constexpr int irq = 39;
};
struct uarte1 {
    static constexpr std::uintptr_t base = 0x40028000U;
    static constexpr int irq = 40;
};
struct qspi {
    static constexpr std::uintptr_t base = 0x40029000U;
    static constexpr int irq = 41;
};
struct cc_host_rgf {
    static constexpr std::uintptr_t base = 0x5002A000U;
    static constexpr int irq = -1;
};
struct cryptocell {
    static constexpr std::uintptr_t base = 0x5002A000U;
    static constexpr int irq = 42;
};
struct pwm3 {
    static constexpr std::uintptr_t base = 0x4002D000U;
    static constexpr int irq = 45;
};
struct spim3 {
    static constexpr std::uintptr_t base = 0x4002F000U;
    static constexpr int irq = 47;
};
}
}
