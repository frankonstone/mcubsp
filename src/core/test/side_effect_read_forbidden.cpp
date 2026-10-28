// @req-file REQ-0008 REQ-0055
#include <mcubsp/core/direct_mmio.hpp>
#include <mcubsp/core/register.hpp>
#include <mcubsp/mcu/nrf52840/product/cc_rng_sram.hpp>
void forbidden(mcubsp::core::direct_mmio &mmio) {
    using namespace mcubsp::mcu::nrf52840::product::cc_rng_sram;
    mcubsp::core::register_ref<sram_data, mcubsp::core::direct_mmio> data{mmio, base};
    (void)data.read();
}
