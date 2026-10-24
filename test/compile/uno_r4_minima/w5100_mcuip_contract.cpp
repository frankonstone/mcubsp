// @req-file REQ-0035 REQ-0044

#include <cstdint>
#include <expected>

#include <mcubsp/integration/mcuip/w5100.hpp>
#include <mcubsp/io.hpp>
#include <mcubsp/io/board/uno_r4_minima.hpp>
#include <mcubsp/io/board/nucleo_h753zi.hpp>
#include <mcubsp/io/board/esp32p4_nano.hpp>
#include <mcubsp/execution/fake_policy.hpp>

namespace {

struct contract_bus {
    [[nodiscard]] std::expected<std::uint8_t, mcubsp::device::w5100_error>
    read(std::uint16_t) noexcept {
        return 0U;
    }

    [[nodiscard]] std::expected<void, mcubsp::device::w5100_error>
    write(std::uint16_t, std::uint8_t) noexcept {
        return {};
    }
};

using device = mcubsp::integration::mcuip::
    w5100_macraw_network_device<contract_bus>;

static_assert(::mcuip::NetworkDevice<device>);
static_assert(device::rx_descriptor_count == 2U);
static_assert(device::tx_descriptor_count == 2U);

// The generic device endpoint satisfies MCUIP on every supplied board profile.
// @req REQ-0046
template<class Profile>
using endpoint = typename mcubsp::io::spi_bus<
    typename mcubsp::io::board<Profile, {}, mcubsp::execution::fake_policy>::context_type>::device_type;
static_assert(mcubsp::integration::mcuip::w5100_valid_contract<
    endpoint<mcubsp::io::boards::uno_r4_minima>>);
static_assert(mcubsp::integration::mcuip::w5100_valid_contract<
    endpoint<mcubsp::io::boards::nucleo_h753zi>>);
static_assert(mcubsp::integration::mcuip::w5100_valid_contract<
    endpoint<mcubsp::io::boards::esp32p4_nano>>);

} // namespace

void mcubsp_uno_r4_minima_w5100_mcuip_contract_probe() noexcept {}
