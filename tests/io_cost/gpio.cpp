#include "common.hpp"

#include <mcuhw/backend/ra4m1/gpio.hpp>
#include <mcuhw/driver/gpio.hpp>

namespace cost {
void progress() noexcept {}

#if MCUHW_IO_COST_FACADE
constexpr io::board_config configuration{.serial = false};
auto hardware = io::make_board<board_type, configuration>(execution, mmio,
                                                         48'000'000U);
#else
mcuhw::backend::ra4m1::gpio_backend backend{mmio};
#endif
} // namespace cost

extern "C" void mcuhw_io_cost_entry() noexcept {
    using namespace cost;
#if MCUHW_IO_COST_FACADE
    check(hardware.begin().has_value());
    auto opened = hardware.output(hardware.pins.d2);
    check(opened.has_value());
    auto output = std::move(*opened);
    observation = output.write(true).has_value();
    check(output.release().has_value());
    check(hardware.shutdown().has_value());
#else
    board_type::start(mmio);
    mcuhw::driver::gpio_pin output{
        backend, mcuhw::board::arduino_uno_r4_minima::d2};
    check(output.initialize({
        .direction = mcuhw::driver::gpio_direction::output,
    }).has_value());
    observation = output.write(mcuhw::driver::gpio_level::high).has_value();
    check(output.release().has_value());
    board_type::stop(mmio);
#endif
    for (;;) __asm__ volatile("");
}
