#include "uart_common.hpp"

extern "C" void mcuhw_io_cost_entry() noexcept {
    using namespace cost;
    using namespace std::chrono_literals;
#if MCUHW_IO_COST_FACADE
    check(hardware.begin().has_value());
    check(hardware.serial.begin(115200U).has_value());
    observation = hardware.serial.write(message, 100ms).has_value();
#else
    board_type::start(mmio);
    const auto timing = board_type::solve_serial(115200U);
    check(timing.has_value());
    check(serial.initialize(*timing).has_value());
    const auto limit = mcuhw::execution::deadline{
        static_cast<mcuhw::execution::tick>(execution.now() + 4'800'000U)};
    auto started = serial.transmit(message);
    check(started.has_value());
    auto transfer = std::move(*started);
    finish_driver(transfer, limit);
#endif
    for (;;) __asm__ volatile("");
}
