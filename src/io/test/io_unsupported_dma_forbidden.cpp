#include <mcubsp/io/board.hpp>
#include <mcubsp/board/waveshare/esp32p4_nano.hpp>
#include <mcubsp/execution/fake_policy.hpp>
void forbidden(mcubsp::execution::fake_policy& execution) {
    constexpr mcubsp::io::board_config config{.dma=mcubsp::io::channels<0>};
    auto hw = mcubsp::io::make_board<mcubsp::board::waveshare::esp32p4_nano, config>(execution, 1000U);
}
