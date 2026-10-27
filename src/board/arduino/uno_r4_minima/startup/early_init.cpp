#include <cstdint>

#include <mcubsp/backend/ra4m1/clock.hpp>
#include <mcubsp/core/direct_mmio.hpp>

extern "C" {
std::uint32_t mcubsp_uno_clock_status;

void mcubsp_uno_early_init() noexcept {
    mcubsp::core::direct_mmio policy;
    mcubsp::backend::ra4m1::clock_backend clock{policy};
    const auto result = clock.configure_hoco_48mhz();
    mcubsp_uno_clock_status = result.has_value()
        ? 0U
        : static_cast<std::uint32_t>(result.error()) + 1U;
}
}
