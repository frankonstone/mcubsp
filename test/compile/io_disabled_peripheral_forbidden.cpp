#include <mcuhw/io.hpp>
#include <mcuhw/io/board/uno_r4_minima.hpp>
#include <mcuhw/execution/fake_policy.hpp>
void forbidden(mcuhw::execution::fake_policy& execution) {
    constexpr mcuhw::io::board_config config{.serial=false};
    auto hw = mcuhw::io::make_board<mcuhw::io::boards::uno_r4_minima, config>(execution, 1000U);
    static_cast<void>(hw.serial.begin());
}
