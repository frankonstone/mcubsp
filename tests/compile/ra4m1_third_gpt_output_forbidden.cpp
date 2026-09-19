#include <mcuhw/core/metadata.hpp>
#include <mcuhw/mcu/ra4m1/description.hpp>

using invalid_output = mcuhw::core::capability_request<
    mcuhw::mcu::ra4m1::gpt32_capabilities,
    static_cast<mcuhw::mcu::ra4m1::peripheral_capability>(0xFF)>;

invalid_output rejected;
