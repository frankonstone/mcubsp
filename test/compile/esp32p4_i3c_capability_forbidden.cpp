#include <mcuhw/mcu/esp32p4/description.hpp>

using unavailable_capability = mcuhw::core::capability_request<
    mcuhw::mcu::esp32p4::i3c_capabilities,
    mcuhw::mcu::esp32p4::peripheral_capability::i2c_multi_master>;

unavailable_capability rejected;
