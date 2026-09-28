#include <mcuhw/backend/ra4m1/gpio.hpp>
#include <mcuhw/core/direct_mmio.hpp>
#include <mcuhw/driver/gpio.hpp>

namespace {
mcuhw::core::direct_mmio policy;
mcuhw::backend::ra4m1::gpio_backend backend{policy};
mcuhw::driver::gpio_pin pin{backend, {1, 11}};
} // namespace

extern "C" __attribute__((noinline)) void mcuhw_gpio_set_high_probe() {
    static_cast<void>(pin.set());
}

extern "C" __attribute__((noinline)) void direct_gpio_set_high_baseline() {
    *reinterpret_cast<volatile unsigned*>(0x4004'0028U) = 1U << 11U;
    __asm__ volatile("dmb sy" ::: "memory");
}
