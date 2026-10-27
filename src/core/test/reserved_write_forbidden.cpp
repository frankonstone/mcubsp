#include <cstdint>

#include <mcubsp/core/register.hpp>
#include <mcubsp/testing/recording_mmio.hpp>

using preserve_register = mcubsp::core::register_descriptor<
    std::uint32_t, 0, mcubsp::core::access_semantics::read_write,
    0xFFU, true, mcubsp::core::reserved_bit_policy::preserve>;

void illegal_direct_write() {
    mcubsp::testing::recording_mmio<4, 2> policy{0x4000'0000U};
    mcubsp::core::register_ref<preserve_register, decltype(policy)> value{
        policy, 0x4000'0000U};
    (void)value.write(0x12U);
}
