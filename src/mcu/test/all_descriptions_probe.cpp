#include <mcubsp/mcu/esp32p4/description.hpp>
#include <mcubsp/mcu/ra4m1/description.hpp>
#include <mcubsp/mcu/stm32h753/description.hpp>
#include <mcubsp/testing/description_suite.hpp>

#if defined(MCUBSP_TARGET) || defined(MCUBSP_MCU) \
    || defined(MCUBSP_TARGET_RA4M1) || defined(MCUBSP_TARGET_STM32H753) \
    || defined(MCUBSP_TARGET_ESP32P4)
#error "all descriptions must compile without a global MCU-selection macro"
#endif

// @req-file REQ-0003 REQ-0038 REQ-0040
static_assert(mcubsp::testing::for_each_description<
    mcubsp::mcu::ra4m1::description,
    mcubsp::mcu::stm32h753::description,
    mcubsp::mcu::esp32p4::description>(
        mcubsp::testing::complete_description_contract{}));
