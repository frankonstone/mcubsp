#include <mcuhw/core/direct_mmio.hpp>
#include <mcuhw/integration/mcuip/stm32h753_ethernet.hpp>

namespace {

using mmio = mcuhw::core::direct_mmio;
using protection = mcuhw::integration::mcuip::descriptor_protection<mmio>;
using device = mcuhw::integration::mcuip::network_device<mmio, protection>;

static_assert(mcuhw::integration::mcuip::valid_contract<mmio, protection>);
static_assert(::mcuip::NetworkDevice<device>);
static_assert(::mcuip::DescriptorProtection<protection>);
static_assert(::mcuip::valid_hardware(
    mcuhw::integration::mcuip::nucleo_h753zi_hardware));

} // namespace

void mcuhw_stm32h753_mcuip_contract_probe() noexcept {}
