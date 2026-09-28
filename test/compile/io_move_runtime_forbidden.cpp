#include <mcuhw/io.hpp>
#include <mcuhw/io/board/uno_r4_minima.hpp>
#include <mcuhw/execution/fake_policy.hpp>
void forbidden(mcuhw::execution::fake_policy& execution) {
    auto hw = mcuhw::io::make_board<mcuhw::io::boards::uno_r4_minima>(execution, 1000U);
    auto moved = std::move(hw);
}
