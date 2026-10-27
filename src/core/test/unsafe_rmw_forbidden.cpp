#include <cstdint>

#include <mcubsp/core/register.hpp>
#include <mcubsp/testing/recording_mmio.hpp>

using pending_register = mcubsp::core::register_descriptor<
    std::uint32_t, 0, mcubsp::core::access_semantics::write_one_to_clear>;

void illegal_modify() {
    mcubsp::testing::recording_mmio<4, 2> policy{0x4000'0000U};
    mcubsp::core::register_ref<pending_register, decltype(policy)> pending{
        policy, 0x4000'0000U};
    (void)pending.modify(1U, 0U);
}
