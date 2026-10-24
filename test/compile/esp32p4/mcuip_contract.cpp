#include <span>

#include <mcubsp/core/direct_mmio.hpp>
#include <mcubsp/integration/mcuip/esp32p4_ethernet.hpp>

namespace {

struct protection {
    void enter() noexcept {}
    void leave() noexcept {}
    void publish_barrier() noexcept {}
    void acquire_barrier() noexcept {}
    void clean_for_device(std::span<const std::byte>) noexcept {}
    void invalidate_from_device(std::span<std::byte>) noexcept {}
};

using mmio = mcubsp::core::direct_mmio;
using device = mcubsp::integration::mcuip::esp32p4_network_device<
    mmio, protection>;

static_assert(mcubsp::integration::mcuip::esp32p4_valid_contract<
              mmio, protection>);
static_assert(::mcuip::NetworkDevice<device>);
static_assert(::mcuip::DescriptorProtection<protection>);
static_assert(::mcuip::valid_hardware(
    mcubsp::integration::mcuip::esp32p4_nano_hardware));

} // namespace

void mcubsp_esp32p4_mcuip_contract_probe() noexcept {}
