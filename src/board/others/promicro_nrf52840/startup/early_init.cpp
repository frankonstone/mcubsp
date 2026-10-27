// @req-file REQ-0053
#include <cstdint>
#include <mcubsp/mcu/nrf52840/model.hpp>

extern "C" {
std::uint32_t mcubsp_promicro_startup_status = 1U;

void mcubsp_promicro_early_init() noexcept {
    // This entry is an application handoff from the factory UF2 bootloader,
    // whose Nordic SystemInit owns silicon errata and reset-time setup.
    // The nRF52840 CPU clock is fixed at 64 MHz; no LFCLK, crystal, regulator,
    // SoftDevice or UICR reconfiguration is performed by this GPIO increment.
    const auto part = *reinterpret_cast<volatile const std::uint32_t*>(
        mcubsp::mcu::nrf52840::ficr_part);
    mcubsp_promicro_startup_status = part == 0x0005'2840U ? 0U : 2U;
}
}
