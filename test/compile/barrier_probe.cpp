#include <mcubsp/arch/barrier.hpp>

extern "C" void mcubsp_data_memory_barrier_probe() {
    mcubsp::arch::data_memory_barrier();
}

extern "C" void mcubsp_data_synchronization_barrier_probe() {
    mcubsp::arch::data_synchronization_barrier();
}

extern "C" void mcubsp_instruction_barrier_probe() {
    mcubsp::arch::instruction_barrier();
}
