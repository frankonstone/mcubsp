# Timer and PWM implementation notes

The RA4M1 backend owns its integer timing solver and GPT register encoding.
Generic driver types describe requests and report timing results. The solver is `constexpr`, uses the instance's real
16- or 32-bit width and the legal GPT divisors (`1, 4, 16, 64, 256, 1024`),
and reports the achieved frequency and error in parts per million. A request is
rejected when its clock or target frequency is zero, its counter width is not
supported, its count cannot fit, or its declared tolerance cannot be met.

The UNO R4 Minima D3 binding is GPT1 output B on P104. This configuration is
evaluated at compile time:

```cpp
using binding = mcubsp::backend::ra4m1::gpt_binding<
    1, mcubsp::mcu::ra4m1::gpt_output::b,
    mcubsp::board::arduino::uno_r4_minima_bindings::d3>;

constexpr auto timing = mcubsp::backend::ra4m1::solve_pwm_timing({
    .clock_hz = 48'000'000,
    .target_hz = 20'000,
    .maximum_error_ppm = 0,
    .counter_width = binding::metadata.counter_width,
});
static_assert(timing.has_value());
static_assert(timing->period_counts == 2400);
```

`pwm_channel` owns one complete GPT instance. Initialization sets the pin to
its source-backed alternate function, programs a low stopped level, and loads
the period and active/buffer compare registers. `start()` and `stop()` are
explicit. Interior `update_duty()` values are written only to the GPT buffer
register and latch at the next overflow. Zero and full duty use the GPT's
dedicated synchronized duty controls instead of relying on out-of-range
compare values.

`next_period()` returns the fixed-storage move-only transfer introduced by
PLN-0006. A target-owned overflow ISR calls `handle_overflow_interrupt()`;
that method acknowledges the GPT overflow flag before publishing exactly one
completion. The backend does not claim a CPU vector or an environment-owned
interrupt controller route.

The native suite covers both counter widths, prescaler limits, zero and invalid
frequencies, rounding, unattainable tolerances, duty endpoints, register
ordering, buffered updates, ownership collisions, and event notification. The
physical fixture is documented with the UNO board procedure.
