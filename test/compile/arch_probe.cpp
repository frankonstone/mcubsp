#include <mcubsp/testing/recording_mmio.hpp>

#include <cstdint>
#include <string_view>

#ifndef MCUBSP_ARCH_NAME
#error "MCUBSP_ARCH_NAME must identify the compile probe"
#endif

constexpr std::string_view mcubsp_architecture{MCUBSP_ARCH_NAME};
static_assert(!mcubsp_architecture.empty());

[[nodiscard]] bool instantiate_public_mmio_policy() noexcept {
    mcubsp::testing::recording_mmio<8, 2> mmio{0x4000'0000U};
    const auto write = mmio.write<std::uint32_t>(0x4000'0000U, 0xA5A5'5A5AU);
    const auto read = mmio.read<std::uint32_t>(0x4000'0000U);
    return write.has_value() && read.has_value() && *read == 0xA5A5'5A5AU;
}
