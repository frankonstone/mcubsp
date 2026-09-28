#include <mcuhw/io.hpp>
#include <mcuhw/io/board/esp32p4_nano.hpp>
#include <mcuhw/execution/fake_policy.hpp>
void forbidden(mcuhw::execution::fake_policy& execution) {
    constexpr mcuhw::io::board_config config{.dma=mcuhw::io::channels<0>};
    auto hw = mcuhw::io::make_board<mcuhw::io::boards::esp32p4_nano, config>(execution, 1000U);
}
