// @req-file REQ-0003 REQ-0009 REQ-0031 REQ-0033

#include <cstdint>

#include <mcuhw/backend/stm32h753/clock.hpp>
#include <mcuhw/core/direct_mmio.hpp>
#include <mcuhw/mcu/stm32h753/system.hpp>

extern "C" {
volatile std::uint32_t mcuhw_nucleo_clock_status;
volatile std::uint32_t mcuhw_nucleo_vector_table;
volatile std::uint32_t mcuhw_nucleo_rcc_cr;
volatile std::uint32_t mcuhw_nucleo_rcc_cfgr;

void mcuhw_nucleo_early_init() noexcept {
    mcuhw::core::direct_mmio policy;
    mcuhw::backend::stm32h753::clock_backend clock{policy};
    const auto result = clock.verify_hsi_64mhz();
    mcuhw_nucleo_clock_status = result.has_value()
        ? 0U
        : static_cast<std::uint32_t>(result.error()) + 1U;
    mcuhw_nucleo_vector_table =
        *reinterpret_cast<volatile const std::uint32_t*>(0xE000'ED08U);
    mcuhw_nucleo_rcc_cr =
        *reinterpret_cast<volatile const std::uint32_t*>(
            mcuhw::mcu::stm32h753::rcc_base);
    mcuhw_nucleo_rcc_cfgr =
        *reinterpret_cast<volatile const std::uint32_t*>(
            mcuhw::mcu::stm32h753::rcc_base + 0x10U);
}
}
