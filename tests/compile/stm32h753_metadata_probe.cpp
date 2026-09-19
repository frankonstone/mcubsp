#include <mcuhw/mcu/stm32h753/description.hpp>

namespace stm = mcuhw::mcu::stm32h753;

static_assert(stm::core_properties.maximum_frequency_hz == 480'000'000U);
static_assert(stm::core_properties.mpu_regions == 16U);
static_assert(stm::memory_regions[2].begin == 0x0000'0000U);
static_assert(stm::memory_regions[3].begin == 0x2000'0000U);
static_assert(stm::mdma_constraints.engine
              == mcuhw::core::dma_engine_kind::stm32_mdma);
