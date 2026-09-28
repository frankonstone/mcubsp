#include <cstdint>

#include <mcuhw/core/register.hpp>
#include <mcuhw/testing/recording_mmio.hpp>

using command_register = mcuhw::core::register_descriptor<
    std::uint32_t, 0, mcuhw::core::access_semantics::command>;

void illegal_read() {
    mcuhw::testing::recording_mmio<4, 1> policy{0x4000'0000U};
    mcuhw::core::register_ref<command_register, decltype(policy)> command{
        policy, 0x4000'0000U};
    (void)command.read();
}
