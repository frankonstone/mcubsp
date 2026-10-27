#include <mcubsp/mcu/esp32p4/description.hpp>

using unavailable_capability = mcubsp::core::capability_request<
    mcubsp::mcu::esp32p4::i3c_capabilities,
    mcubsp::mcu::esp32p4::peripheral_capability::i2c_multi_master>;

unavailable_capability rejected;
