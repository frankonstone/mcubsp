# GPIO

GPIO is the smallest complete `mcubsp` composition. This example configures the
UNO R4 Minima built-in LED pin as a push-pull output and toggles it.

## Code

```cpp
#include <mcubsp/backend/ra4m1/gpio.hpp>
#include <mcubsp/board/arduino/uno_r4_minima/bindings.hpp>
#include <mcubsp/core/direct_mmio.hpp>
#include <mcubsp/driver/gpio.hpp>

using board = mcubsp::board::arduino::uno_r4_minima_bindings;

void application_delay(); // implement with the application's timer/scheduler

int blink_once() {
    mcubsp::core::direct_mmio mmio;
    mcubsp::backend::ra4m1::gpio_backend gpio_hardware{mmio};
    mcubsp::driver::gpio_pin led{gpio_hardware, board::led_builtin_d13};

    const auto initialized = led.initialize({
        .direction = mcubsp::driver::gpio_direction::output,
        .initial_level = mcubsp::driver::gpio_level::low,
        .output_type = mcubsp::driver::gpio_output_type::push_pull,
        .pull = mcubsp::driver::gpio_pull::none,
        .drive = mcubsp::driver::gpio_drive::middle,
    });
    if (!initialized) {
        return 1;
    }

    if (!led.set()) {
        return 2;
    }
    application_delay();
    if (!led.clear()) {
        return 3;
    }

    return led.release() ? 0 : 4;
}

int main() {
    // Startup has already initialized RAM and the board clock.
    return blink_once();
}
```

`application_delay()` is deliberately outside `mcubsp`; use a timer or the
scheduler owned by your application. Do not implement a production delay from
an uncalibrated empty loop.

## Why initialization matters

`initialize()` claims the pin and applies the complete configuration. The
initial latch level is programmed before output drive is enabled, which avoids
an unwanted pulse during startup. A second owner receives an error instead of
silently changing the pin configuration.

Call `release()` only after the pin no longer controls external hardware. If a
peripheral will take over the pin, release the GPIO object before initializing
that peripheral.

GPIO operations complete inside the call, so this example needs neither an
execution policy nor an interrupt handler.

## Hardware check

No jumper is needed for the built-in LED. Run:

```sh
./tool/run-uno-r4-minima-tests.sh startup-gpio
```

If the LED logic appears inverted, check the board schematic before changing
the code: LEDs on some boards are wired active-low.
