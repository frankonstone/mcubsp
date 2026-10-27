// @req-file REQ-0015 REQ-0045
#include <mcubsp/io/board.hpp>
#include <mcubsp/board/arduino/uno_r4_minima.hpp>
#include <mcubsp/board/st/nucleo_h753zi.hpp>
#include <mcubsp/board/waveshare/esp32p4_nano.hpp>
#include <mcubsp/execution/fake_policy.hpp>
#include <mcubsp/execution/monotonic_policy.hpp>
#include <mcubsp/testing/recording_mmio.hpp>
namespace {
using Mmio = mcubsp::testing::recording_mmio<1024, 1024>;
struct Clock {
    static constexpr unsigned ticks_per_second = 1000;
    std::uint64_t now() const noexcept { return 0; }
};
template<class Board> void probe(mcubsp::execution::fake_policy& policy, Mmio& mmio) {
    Clock clock;
    mcubsp::execution::monotonic_policy execution{policy, clock};
    using Selection = mcubsp::io::serial_selection<typename Board::serial_binding>;
    auto hw = mcubsp::io::make_board<Board, {.serial=false}, Selection>(execution, mmio, 1000);
    static_cast<void>(hw.begin());
    static_cast<void>(hw.template peripheral<0>().begin());
    hw.poll();
    static_cast<void>(hw.shutdown());
}
}
void composition_probe(mcubsp::execution::fake_policy& policy, Mmio& mmio) {
    probe<mcubsp::board::arduino::uno_r4_minima>(policy, mmio);
    probe<mcubsp::board::st::nucleo_h753zi>(policy, mmio);
    probe<mcubsp::board::waveshare::esp32p4_nano>(policy, mmio);
}
