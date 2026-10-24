# PWM

This example produces a 20 kHz PWM signal on UNO D3 with a 25% duty cycle,
then changes it to 75%. D3 is the GPT1 channel B route on the RA4M1.

The `application_execution_policy` used by the example is defined in the
[bare-metal application integration guide](../../../../../doc/concept/application-integration.md).

## Wiring

For normal use, connect D3 to the input of the device being controlled and
connect the grounds. For the repository's measurement procedure, jumper D3 to
D2 so a second GPIO can observe the waveform. Do not drive a motor, relay, or
high-current LED directly from an MCU pin; use a suitable driver stage.

## Code

```cpp
#include <mcubsp/backend/ra4m1/gpt.hpp>
#include <mcubsp/board/arduino_uno_r4_minima.hpp>
#include <mcubsp/core/direct_mmio.hpp>
#include <mcubsp/driver/pwm.hpp>

namespace board = mcubsp::board::arduino_uno_r4_minima;

using binding = mcubsp::backend::ra4m1::gpt_binding<
    1, mcubsp::mcu::ra4m1::gpt_output::b, board::d3>;

constexpr auto timing = mcubsp::backend::ra4m1::solve_pwm_timing({
    .clock_hz = 48'000'000U,
    .target_hz = 20'000U,
    .maximum_error_ppm = 0U,
    .counter_width = binding::metadata.counter_width,
});
static_assert(timing.has_value());

template<class ExecutionPolicy>
int run_pwm(ExecutionPolicy& execution) {
    mcubsp::core::direct_mmio mmio;
    using backend_type = mcubsp::backend::ra4m1::gpt_pwm_backend<
        binding, mcubsp::core::direct_mmio, ExecutionPolicy>;

    backend_type hardware{mmio, execution};
    mcubsp::driver::pwm_channel pwm{hardware};

    if (!pwm.initialize(*timing,
                        mcubsp::driver::pwm_duty::percent(25U))) {
        return 1;
    }
    if (!pwm.start()) {
        return 2;
    }

    // The RA4M1 backend buffers an active-duty update. It takes effect at the
    // period boundary rather than truncating the current pulse.
    if (!pwm.update_duty(mcubsp::driver::pwm_duty::percent(75U))) {
        return 3;
    }

    if (!pwm.stop()) {
        return 4;
    }
    return pwm.release() ? 0 : 5;
}

```

Call the synchronous example from `main()` with the application policy
defined in the integration chapter:

```cpp
int main() {
    application_execution_policy execution;
    return run_pwm(execution);
}
```

The timing solver checks the source clock, counter width, prescaler choices,
and requested error tolerance at compile time when its input is constant.
Duty is expressed as a rational number, so 0% and 100% do not depend on
floating-point support.

If the application must know exactly when a buffered update became active,
start `pwm.next_period()`, service the GPT overflow interrupt, and consume the
returned transfer after the next period boundary.

The basic start, duty-update, stop, and release calls above are synchronous, so
the local backend is safe and no GPT interrupt is used. The backend type still
takes an execution policy because it also provides the asynchronous
`next_period()` operation. If that operation is used, move MMIO, policy,
backend, and driver to application scope and route the GPT overflow ISR to
`hardware.handle_overflow_interrupt()`, following the lifetime pattern in the
[application integration guide](../../../../../doc/concept/application-integration.md).

## Hardware check

```sh
./tool/run-uno-r4-minima-tests.sh pwm
```

The physical test expects D3 jumpered to D2. Basic PWM is supported;
complementary outputs, dead time, synchronization, and fault inputs belong to
the advanced motor-control feature set.
