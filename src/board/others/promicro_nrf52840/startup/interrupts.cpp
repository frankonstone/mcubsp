// @req-file REQ-0057 REQ-0073
#include <cstdint>
#include <mcubsp/backend/nrf52840/core.hpp>
extern "C" {
// Kept in RAM for a debugger to inspect. Fault reporting uses no stack-frame
// dereference, I/O, allocation or peripheral callback during a fault.
volatile std::uint32_t mcubsp_promicro_fault_record[7]{};
void mcubsp_promicro_dispatch_irq(unsigned irq) noexcept {
    if (mcubsp_promicro_irq_callback_target) {
        mcubsp_promicro_irq_callback_target(mcubsp_promicro_irq_callback_context, irq);
        return;
    }
    for (;;) {
        asm volatile("bkpt #0");
    }
}
[[noreturn]] void mcubsp_promicro_record_fault(unsigned exception) noexcept {
    mcubsp_promicro_fault_record[0] = exception;
    for (unsigned n = 0; n < 6; ++n)
        mcubsp_promicro_fault_record[n + 1] =
            *reinterpret_cast<volatile std::uint32_t *>(0xE000ED28U + n * 4);
    for (;;) {
        asm volatile("nop");
    }
}
}
