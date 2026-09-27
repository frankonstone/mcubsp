#include <mcuhw/mcu/esp32p4/description.hpp>
#include <mcuhw/mcu/ra4m1/description.hpp>
#include <mcuhw/mcu/stm32h753/description.hpp>
#include <mcuhw/testing/description_suite.hpp>

namespace {
static_assert(mcuhw::testing::for_each_description<
    mcuhw::mcu::ra4m1::description,
    mcuhw::mcu::stm32h753::description,
    mcuhw::mcu::esp32p4::description>(
        mcuhw::testing::complete_description_contract{}));
}
