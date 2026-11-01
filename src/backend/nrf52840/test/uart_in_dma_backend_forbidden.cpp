#include <mcubsp/backend/nrf52840/uarte.hpp>
#include <mcubsp/core/direct_mmio.hpp>
#include <mcubsp/execution/fake_policy.hpp>
using namespace mcubsp;
using namespace backend::nrf52840;
using binding=uart_binding<peripheral::uart0,{0,6},{0,8}>;
using context=system_context<core::direct_mmio,execution::fake_policy>;
using buffers=dma_buffer_pool<execution::fake_policy>;
static_assert(sizeof(uarte_controller<binding,context,buffers>)>0);
