#include <mcuhw/mcu/esp32p4/description.hpp>
#include <mcuhw/mcu/ra4m1/description.hpp>
#include <mcuhw/mcu/stm32h753/description.hpp>
#include <mcuhw/testing/description_suite.hpp>

#if defined(MCUHW_TARGET) || defined(MCUHW_MCU) \
    || defined(MCUHW_TARGET_RA4M1) || defined(MCUHW_TARGET_STM32H753) \
    || defined(MCUHW_TARGET_ESP32P4)
#error "all descriptions must compile without a global MCU-selection macro"
#endif

// @req-file REQ-0003 REQ-0038 REQ-0040
static_assert(mcuhw::testing::for_each_description<
    mcuhw::mcu::ra4m1::description,
    mcuhw::mcu::stm32h753::description,
    mcuhw::mcu::esp32p4::description>(
        mcuhw::testing::complete_description_contract{}));
