#include <mcubsp/io.hpp>
#include <mcubsp/io/board/uno_r4_minima.hpp>
#include <mcubsp/execution/fake_policy.hpp>
void forbidden(mcubsp::execution::fake_policy& execution) {
    auto hw = mcubsp::io::make_board<mcubsp::io::boards::uno_r4_minima>(execution, 1000U);
    auto moved = std::move(hw);
}
