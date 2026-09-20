#if defined(MCUHW_COVERAGE_PROBE_I2C)
#include <mcuhw/backend/stm32h753/i2c.hpp>
#include <mcuhw/board/nucleo_h753zi.hpp>
#include <mcuhw/core/direct_mmio.hpp>
#include <mcuhw/execution/fake_policy.hpp>

using mcuhw_i2c_coverage_binding = mcuhw::backend::stm32h753::i2c_binding<
    1, mcuhw::board::nucleo_h753zi::arduino_d15_scl,
    mcuhw::board::nucleo_h753zi::arduino_d14_sda>;
template class mcuhw::backend::stm32h753::i2c_controller_backend<
    mcuhw_i2c_coverage_binding, mcuhw::core::direct_mmio,
    mcuhw::execution::fake_policy>;
#elif defined(MCUHW_COVERAGE_PROBE_MEMORY)
#include <mcuhw/backend/stm32h753/memory.hpp>
#elif defined(MCUHW_COVERAGE_PROBE_SPI)
#include <mcuhw/backend/stm32h753/spi.hpp>
#elif defined(MCUHW_COVERAGE_PROBE_TIMER)
#include <mcuhw/backend/stm32h753/timer.hpp>
#elif defined(MCUHW_COVERAGE_PROBE_USART)
#include <mcuhw/backend/stm32h753/usart.hpp>
#elif defined(MCUHW_COVERAGE_PROBE_ETHERNET)
#include <mcuhw/backend/stm32h753/ethernet.hpp>
#else
#error "select one STM32H753 coverage manifest probe"
#endif

void mcuhw_stm32h753_backend_coverage_manifest_probe() noexcept {}
