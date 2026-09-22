// @req-file REQ-0035 REQ-0044

#include <cstdint>
#include <expected>

#include <mcuhw/integration/mcuip/uno_r4_minima_w5100.hpp>

namespace {

struct contract_bus {
    [[nodiscard]] std::expected<std::uint8_t, mcuhw::driver::w5100_error>
    read(std::uint16_t) noexcept {
        return 0U;
    }

    [[nodiscard]] std::expected<void, mcuhw::driver::w5100_error>
    write(std::uint16_t, std::uint8_t) noexcept {
        return {};
    }
};

using device = mcuhw::integration::mcuip::
    uno_r4_minima_w5100_network_device<contract_bus>;

static_assert(mcuhw::integration::mcuip::
    uno_r4_minima_w5100_valid_contract<contract_bus>);
static_assert(::mcuip::NetworkDevice<device>);
static_assert(device::rx_descriptor_count == 2U);
static_assert(device::tx_descriptor_count == 2U);

} // namespace

void mcuhw_uno_r4_minima_w5100_mcuip_contract_probe() noexcept {}
