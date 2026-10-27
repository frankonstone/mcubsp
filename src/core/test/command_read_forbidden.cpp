#include <cstdint>

#include <mcubsp/core/register.hpp>
#include <mcubsp/testing/recording_mmio.hpp>

using command_register = mcubsp::core::register_descriptor<
    std::uint32_t, 0, mcubsp::core::access_semantics::command>;

void illegal_read() {
    mcubsp::testing::recording_mmio<4, 1> policy{0x4000'0000U};
    mcubsp::core::register_ref<command_register, decltype(policy)> command{
        policy, 0x4000'0000U};
    (void)command.read();
}
