#include <cstdint>

#include <mcuhw/core/register.hpp>
#include <mcuhw/testing/recording_mmio.hpp>

using pending_register = mcuhw::core::register_descriptor<
    std::uint32_t, 0, mcuhw::core::access_semantics::write_one_to_clear>;

void illegal_modify() {
    mcuhw::testing::recording_mmio<4, 2> policy{0x4000'0000U};
    mcuhw::core::register_ref<pending_register, decltype(policy)> pending{
        policy, 0x4000'0000U};
    (void)pending.modify(1U, 0U);
}
