#include <mcubsp/io/board.hpp>
#include <mcubsp/board/arduino/uno_r4_minima.hpp>
#include <mcubsp/execution/fake_policy.hpp>
void forbidden(mcubsp::execution::fake_policy& execution) {
    constexpr mcubsp::io::board_config config{.serial=false};
    auto hw = mcubsp::io::make_board<mcubsp::board::arduino::uno_r4_minima, config>(execution, 1000U);
    static_cast<void>(hw.serial.begin());
}
