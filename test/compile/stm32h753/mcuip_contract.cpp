#include <mcubsp/core/direct_mmio.hpp>
#include <mcubsp/integration/mcuip/stm32h753_ethernet.hpp>

namespace {

using mmio = mcubsp::core::direct_mmio;
using protection = mcubsp::integration::mcuip::descriptor_protection<mmio>;
using device = mcubsp::integration::mcuip::network_device<mmio, protection>;

static_assert(mcubsp::integration::mcuip::valid_contract<mmio, protection>);
static_assert(::mcuip::NetworkDevice<device>);
static_assert(::mcuip::DescriptorProtection<protection>);
static_assert(::mcuip::valid_hardware(
    mcubsp::integration::mcuip::nucleo_h753zi_hardware));

} // namespace

void mcubsp_stm32h753_mcuip_contract_probe() noexcept {}
