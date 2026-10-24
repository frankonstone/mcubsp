#include <mcubsp/io.hpp>
#include <mcubsp/io/board/uno_r4_minima.hpp>
#include <mcubsp/execution/fake_policy.hpp>
void forbidden(mcubsp::execution::fake_policy& execution) {
    constexpr mcubsp::io::board_config config{.serial=false};
    auto hw = mcubsp::io::make_board<mcubsp::io::boards::uno_r4_minima, config>(execution, 1000U);
    static_cast<void>(hw.serial.begin());
}
