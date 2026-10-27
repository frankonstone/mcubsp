#include <mcubsp/core/metadata.hpp>
#include <mcubsp/mcu/ra4m1/description.hpp>

using invalid_output = mcubsp::core::capability_request<
    mcubsp::mcu::ra4m1::gpt32_capabilities,
    static_cast<mcubsp::mcu::ra4m1::peripheral_capability>(0xFF)>;

invalid_output rejected;
