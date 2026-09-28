#include "uart_common.hpp"

extern "C" void mcuhw_io_cost_entry() noexcept {
    using namespace cost;
    using namespace std::chrono_literals;
#if MCUHW_IO_COST_FACADE
    check(hardware.begin().has_value());
    check(hardware.serial.begin(115200U).has_value());
    auto started = hardware.serial.write_async(message);
    check(started.has_value());
    auto transfer = std::move(*started);
    other_work();
    observation = transfer.finish(100ms).has_value();
#else
    board_type::start(mmio);
    const auto timing = board_type::solve_serial(115200U);
    check(timing.has_value());
    check(serial.initialize(*timing).has_value());
    auto started = serial.transmit(message);
    check(started.has_value());
    auto transfer = std::move(*started);
    other_work();
    finish_driver(transfer);
#endif
    for (;;) __asm__ volatile("");
}
