#include <cstdint>

#include <mcuhw/backend/ra4m1/clock.hpp>
#include <mcuhw/core/direct_mmio.hpp>

extern "C" {
std::uint32_t mcuhw_uno_clock_status;

void mcuhw_uno_early_init() noexcept {
    mcuhw::core::direct_mmio policy;
    mcuhw::backend::ra4m1::clock_backend clock{policy};
    const auto result = clock.configure_hoco_48mhz();
    mcuhw_uno_clock_status = result.has_value()
        ? 0U
        : static_cast<std::uint32_t>(result.error()) + 1U;
}
}
