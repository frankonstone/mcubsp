#include <array>
#include <chrono>
#include <mcuhw/io.hpp>
#include <mcuhw/io/board/uno_r4_minima.hpp>
#include <mcuhw/io/board/nucleo_h753zi.hpp>
#include <mcuhw/io/board/esp32p4_nano.hpp>
#include <mcuhw/execution/fake_policy.hpp>

namespace {
using namespace std::chrono_literals;
namespace io = mcuhw::io;
struct inert_mmio {
    template<class T> std::expected<T, mcuhw::core::mmio_error> read(std::uintptr_t) noexcept { return T{}; }
    template<class T> std::expected<void, mcuhw::core::mmio_error> write(std::uintptr_t, T) noexcept { return {}; }
};
template<class Board>
int instantiate() {
    constexpr io::board_config config{.serial=true, .i2c=true, .spi=true};
    inert_mmio mmio;
    mcuhw::execution::fake_policy policy;
    auto hw = io::make_board<Board, config>(policy, mmio, 1000U);
    if (!hw.begin()) return 1;
    if (!hw.serial.begin()) return 2;
    if (!hw.serial.println("hello", 1ms)) return 3;
    std::array<std::uint8_t, 1> bytes{};
    auto started = hw.serial.read_async(bytes);
    if (started) {
        static_cast<void>(started->cancel());
        static_cast<void>(started->take());
    }
    if (!hw.i2c.begin()) return 4;
    if (!hw.spi.begin()) return 5;
    hw.poll();
    return hw.shutdown() ? 0 : 6;
}
}

// Compile probes only: no simulated memory is dereferenced on any target.
int io_uno_probe() { return instantiate<io::boards::uno_r4_minima>(); }
int io_stm_probe() { return instantiate<io::boards::nucleo_h753zi>(); }
int io_esp_probe() { return instantiate<io::boards::esp32p4_nano>(); }
