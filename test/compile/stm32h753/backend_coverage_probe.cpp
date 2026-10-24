#if defined(MCUBSP_COVERAGE_PROBE_I2C)
#include <mcubsp/backend/stm32h753/i2c.hpp>
#include <mcubsp/board/nucleo_h753zi.hpp>
#include <mcubsp/core/direct_mmio.hpp>
#include <mcubsp/execution/fake_policy.hpp>

using mcubsp_i2c_coverage_binding = mcubsp::backend::stm32h753::i2c_binding<
    1, mcubsp::board::nucleo_h753zi::arduino_d15_scl,
    mcubsp::board::nucleo_h753zi::arduino_d14_sda>;
template class mcubsp::backend::stm32h753::i2c_controller_backend<
    mcubsp_i2c_coverage_binding, mcubsp::core::direct_mmio,
    mcubsp::execution::fake_policy>;
#elif defined(MCUBSP_COVERAGE_PROBE_MEMORY)
#include <mcubsp/backend/stm32h753/memory.hpp>
#elif defined(MCUBSP_COVERAGE_PROBE_SPI)
#include <mcubsp/backend/stm32h753/spi.hpp>
#elif defined(MCUBSP_COVERAGE_PROBE_TIMER)
#include <mcubsp/backend/stm32h753/timer.hpp>
#elif defined(MCUBSP_COVERAGE_PROBE_USART)
#include <mcubsp/backend/stm32h753/usart.hpp>
#elif defined(MCUBSP_COVERAGE_PROBE_ETHERNET)
#include <mcubsp/backend/stm32h753/ethernet.hpp>
#else
#error "select one STM32H753 coverage manifest probe"
#endif

void mcubsp_stm32h753_backend_coverage_manifest_probe() noexcept {}
