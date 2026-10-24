#include <mcubsp/mcu/esp32p4/description.hpp>

namespace esp = mcubsp::mcu::esp32p4;

static_assert(esp::cpu_cores[0].xlen == 32U);
static_assert(esp::cpu_cores[0].maximum_frequency_hz == 400'000'000U);
static_assert(esp::cpu_cores[2].owner == esp::cpu_domain::lp_cpu);
static_assert(esp::memory_regions[4].begin == 0x4FF0'0000U);
static_assert(esp::gdma_ahb_constraints.engine
              == mcubsp::core::dma_engine_kind::esp_gdma_ahb);
static_assert(esp::interrupt_database::valid_cpu_route(esp::event::uart0, 31));
