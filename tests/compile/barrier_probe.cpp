#include <mcuhw/arch/barrier.hpp>

extern "C" void mcuhw_data_memory_barrier_probe() {
    mcuhw::arch::data_memory_barrier();
}

extern "C" void mcuhw_data_synchronization_barrier_probe() {
    mcuhw::arch::data_synchronization_barrier();
}

extern "C" void mcuhw_instruction_barrier_probe() {
    mcuhw::arch::instruction_barrier();
}
