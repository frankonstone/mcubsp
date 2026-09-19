#include <mcuhw/testing/recording_mmio.hpp>

#include <cstdint>
#include <string_view>

#ifndef MCUHW_ARCH_NAME
#error "MCUHW_ARCH_NAME must identify the compile probe"
#endif

constexpr std::string_view mcuhw_architecture{MCUHW_ARCH_NAME};
static_assert(!mcuhw_architecture.empty());

[[nodiscard]] bool instantiate_public_mmio_policy() noexcept {
    mcuhw::testing::recording_mmio<8, 2> mmio{0x4000'0000U};
    const auto write = mmio.write<std::uint32_t>(0x4000'0000U, 0xA5A5'5A5AU);
    const auto read = mmio.read<std::uint32_t>(0x4000'0000U);
    return write.has_value() && read.has_value() && *read == 0xA5A5'5A5AU;
}
