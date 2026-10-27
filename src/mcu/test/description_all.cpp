#include <mcubsp/mcu/esp32p4/description.hpp>
#include <mcubsp/mcu/ra4m1/description.hpp>
#include <mcubsp/mcu/stm32h753/description.hpp>
#include <mcubsp/testing/description_suite.hpp>

namespace {
static_assert(mcubsp::testing::for_each_description<
    mcubsp::mcu::ra4m1::description,
    mcubsp::mcu::stm32h753::description,
    mcubsp::mcu::esp32p4::description>(
        mcubsp::testing::complete_description_contract{}));
}
