// @req-file REQ-0046
#include <array>
#include <chrono>
#include <utility>
#include <mcuhw/io.hpp>
#include <mcuhw/io/boards/uno_r4_minima.hpp>
#include <mcuhw/io/boards/nucleo_h753zi.hpp>
#include <mcuhw/io/boards/esp32p4_nano.hpp>
#include <mcuhw/device/ethernet/w5100.hpp>
#include <mcuhw/device/sensor/tmp102.hpp>
#include <mcuhw/execution/fake_policy.hpp>

namespace {
namespace io = mcuhw::io;
using namespace std::chrono_literals;
struct inert_mmio {
    template<class T> std::expected<T, mcuhw::core::mmio_error> read(std::uintptr_t) noexcept { return T{}; }
    template<class T> std::expected<void, mcuhw::core::mmio_error> write(std::uintptr_t, T) noexcept { return {}; }
};
template<class Profile>
int instantiate() {
    constexpr io::board_config config{.serial = false, .i2c = true, .spi = true};
    inert_mmio mmio;
    mcuhw::execution::fake_policy policy;
    auto hw = io::make_board<Profile, config>(policy, mmio, 1000U);
    if (!hw.begin() || !hw.i2c.begin() || !hw.spi.begin()) return 1;
    auto i2c = hw.i2c.device(0x48);
    if (!i2c) return 2;
    mcuhw::device::tmp102 sensor{std::move(*i2c)};
    if (sensor.begin()) static_cast<void>(sensor.read(10ms));
    auto spi = hw.spi.device({.cs = Profile::spi_binding::select_pin, .frequency = 1'000'000U});
    if (!spi) return 3;
    mcuhw::device::w5100 ethernet{std::move(*spi)};
    if (ethernet.begin({.mac = {{2, 0, 0, 4, 0x51, 0}}})) {
        std::array<std::byte, decltype(ethernet)::buffer_size> frame{};
        static_cast<void>(ethernet.arm_receive(0, frame));
        ethernet.poll();
        ethernet.cancel_all();
    }
    return 0;
}
}
// Compile-only: never execute target MMIO or this inert peripheral fixture.
int io_devices_uno_probe() { return instantiate<io::boards::uno_r4_minima>(); }
int io_devices_nucleo_probe() { return instantiate<io::boards::nucleo_h753zi>(); }
int io_devices_nano_probe() { return instantiate<io::boards::esp32p4_nano>(); }
