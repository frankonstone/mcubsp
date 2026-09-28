#include <cstdint>

#include <mcuhw/core/register.hpp>
#include <mcuhw/testing/recording_mmio.hpp>

using preserve_register = mcuhw::core::register_descriptor<
    std::uint32_t, 0, mcuhw::core::access_semantics::read_write,
    0xFFU, true, mcuhw::core::reserved_bit_policy::preserve>;

void illegal_direct_write() {
    mcuhw::testing::recording_mmio<4, 2> policy{0x4000'0000U};
    mcuhw::core::register_ref<preserve_register, decltype(policy)> value{
        policy, 0x4000'0000U};
    (void)value.write(0x12U);
}
