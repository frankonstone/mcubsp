# Integrating mcuhw into a bare-metal application

`mcuhw` supplies hardware descriptions, peripheral backends, functional
drivers, and fixed-storage transfer state. It deliberately does not take over
startup, clocks, the vector table, interrupt routing, a scheduler, or the
application's notion of time.

The complete control flow is:

```text
reset
  |
  +-- startup initializes RAM, stack, clocks, and C++ static objects
  |
main()
  |
  +-- initializes the monotonic clock
  +-- configures interrupt-controller routes
  +-- initializes mcuhw drivers
  +-- starts a transfer and keeps its buffers alive
  +-- waits, polls, or continues application work
  |
peripheral interrupt
  |
  +-- vector entry calls the backend handle_*_interrupt() method
  +-- backend acknowledges hardware and publishes completion
  +-- execution policy notifies the waiting context
  |
main()
  |
  +-- wait returns
  +-- take() consumes the result and releases the operation slot
```

## What the application and library own

| Concern | Owner |
| --- | --- |
| Register addresses, pin routes, peripheral metadata | `mcuhw` MCU description |
| Native peripheral register sequencing | `mcuhw` backend |
| Functional GPIO/UART/I2C/SPI/ADC/PWM API | `mcuhw` driver |
| Transfer state and typed completion errors | `mcuhw` core |
| Reset handler and C/C++ runtime initialization | Application or board support |
| Linker script and memory placement | Application or board support |
| Clock-tree initialization and solver input clocks | Application or board support |
| Vector table, IRQ routing, priorities, and vector names | Application or board support |
| Critical sections, monotonic time, waiting, notification | Application execution policy |
| Lifetime of backends, drivers, transfers, and buffers | Application |

The library cannot infer the application-owned choices safely. Using SysTick
without permission could conflict with an RTOS, while changing a clock inside
a UART driver could invalidate every other peripheral.

## Why an execution policy is required

An interrupt-driven transfer is accessed from two contexts:

1. `main()` starts, waits for, cancels, or consumes the operation.
2. An interrupt handler advances the peripheral and publishes completion.

Those contexts share a fixed operation slot. The execution policy tells
`mcuhw` how this application protects that state and how a wait observes time
and completion. It is a compile-time dependency, not a base class, scheduler,
thread, or heap-allocated callback.

| Policy operation | Purpose |
| --- | --- |
| `critical(function)` | Run a short state transition without interruption |
| `now()` | Return the wrapping 32-bit monotonic tick count |
| `in_isr()` | Reject blocking waits from interrupt context |
| `prepare_wait()` | Capture notification state before checking readiness |
| `wait_once(token, deadline)` | Make one bounded wait or polling step |
| `notify()` | Publish that completion may now be observable |
| `active_transfer_destroyed()` | Trap or report destruction of a live transfer |

The `prepare_wait -> inspect -> wait_once` sequence closes the race where an
interrupt completes after the caller checks readiness but immediately before
it waits.

Synchronous GPIO does not need an execution policy. UART, I2C, SPI, ADC, DMA,
and PWM period events do. A backend may still accept a policy when an
application uses a synchronous fast path because its type also exposes an
asynchronous API.

## Concrete UNO R4 Minima policy

`mcuhw::execution::bare_metal_policy` implements the policy protocol in terms
of application hooks. These hooks use PRIMASK for critical sections and the
Cortex-M DWT cycle counter as a 48 MHz monotonic clock:

```cpp
#include <cstdint>

#include <mcuhw/execution/bare_metal_policy.hpp>

struct application_execution_hooks {
    using critical_token = std::uint32_t;

    static critical_token enter_critical() noexcept {
        std::uint32_t previous;
        __asm__ volatile(
            "mrs %0, primask\n\t"
            "cpsid i"
            : "=r"(previous) :: "memory");
        return previous;
    }

    static void leave_critical(critical_token previous) noexcept {
        if ((previous & 1U) == 0U) {
            __asm__ volatile("cpsie i" ::: "memory");
        }
    }

    static mcuhw::execution::tick now() noexcept {
        return *reinterpret_cast<volatile const std::uint32_t*>(
            0xE000'1004U); // DWT_CYCCNT
    }

    static bool in_isr() noexcept {
        std::uint32_t exception_number;
        __asm__ volatile("mrs %0, ipsr" : "=r"(exception_number));
        return exception_number != 0U;
    }

    // Called with interrupts enabled. It must make one bounded progress step.
    static void poll() noexcept {
        __asm__ volatile("nop");
    }

    static void notify() noexcept {
        __asm__ volatile("sev" ::: "memory");
    }

    [[noreturn]] static void active_transfer_destroyed() noexcept {
        // Hardware might still use the transfer's buffers. Stop at the error.
        for (;;) {
            __asm__ volatile("bkpt #0");
        }
    }
};

using application_execution_policy =
    mcuhw::execution::bare_metal_policy<application_execution_hooks>;
```

Enable the cycle counter before creating deadlines:

```cpp
void initialize_monotonic_clock() noexcept {
    auto& demcr = *reinterpret_cast<volatile std::uint32_t*>(
        0xE000'EDFCU);
    auto& dwt_control = *reinterpret_cast<volatile std::uint32_t*>(
        0xE000'1000U);
    auto& cycle_counter = *reinterpret_cast<volatile std::uint32_t*>(
        0xE000'1004U);

    demcr |= 1U << 24U;
    cycle_counter = 0U;
    dwt_control |= 1U;
}
```

At 48 MHz, 4,800,000 ticks are 100 ms. The counter may wrap; relative
deadlines are valid up to `2^31 - 1` ticks.

This policy polls and does not put the core to sleep. Replacing `poll()` with
`WFI` requires a target-validated event handshake so completion cannot occur
in the completion-before-sleep window. An RTOS adapter would map the same
responsibilities to RTOS critical sections, ticks, and notifications.

`mcuhw::execution::fake_policy` is for deterministic host tests. It is not a
production board policy.

## Object lifetime and interrupt visibility

Backends called by interrupt handlers need stable storage. Namespace-scope
application objects are the simplest bare-metal composition:

```cpp
namespace {

mcuhw::core::direct_mmio mmio;
application_execution_policy execution;

using uart_binding = mcuhw::backend::ra4m1::sci_uart_binding<
    2,
    mcuhw::board::arduino_uno_r4_minima::d0_rx,
    mcuhw::board::arduino_uno_r4_minima::d1_tx>;

using uart_backend_type = mcuhw::backend::ra4m1::sci_uart_backend<
    uart_binding,
    mcuhw::core::direct_mmio,
    application_execution_policy>;

uart_backend_type uart_hardware{mmio, execution};
mcuhw::driver::uart uart{uart_hardware};

} // namespace
```

The order matters:

1. MMIO and the policy outlive the backend.
2. The backend outlives the driver and every transfer handle.
3. Transfer buffers outlive the transfer.
4. Interrupts are disabled or detached before backend destruction.

Function-local drivers are fine for synchronous GPIO. A function-local
interrupt-driven backend is safe only if IRQ dispatch can reach it and all
IRQs are detached before the function returns. Static application composition
is normally clearer.

## Complete UART shape

This example includes the application-level pieces around a UART operation.
Clock-tree setup normally happens in startup code. These concrete helpers
configure RA4M1 ICU event-link slots 0 through 3 and the corresponding
Cortex-M NVIC lines. Keep the selected lines disabled until
`uart.initialize()` has finished.

```cpp
#include <array>
#include <utility>

#include <mcuhw/backend/ra4m1/sci_uart.hpp>
#include <mcuhw/board/arduino_uno_r4_minima.hpp>
#include <mcuhw/core/direct_mmio.hpp>
#include <mcuhw/driver/uart.hpp>
#include <mcuhw/execution/deadline.hpp>
#include <mcuhw/mcu/ra4m1/peripherals.hpp>

namespace {

constexpr std::uintptr_t nvic_enable = 0xE000'E100U;
constexpr std::uintptr_t nvic_disable = 0xE000'E180U;
constexpr std::uintptr_t nvic_clear_pending = 0xE000'E280U;
constexpr std::uintptr_t nvic_priority = 0xE000'E400U;
constexpr std::uintptr_t icu_event_links = 0x4000'6300U;

void configure_icu_line(
    std::uint32_t line,
    mcuhw::mcu::ra4m1::event event,
    std::uint8_t priority = 0x80U) noexcept {
    const auto mask = static_cast<std::uint32_t>(1U << line);
    *reinterpret_cast<volatile std::uint32_t*>(nvic_disable) = mask;
    *reinterpret_cast<volatile std::uint32_t*>(
        icu_event_links + line * 4U)
        = mcuhw::mcu::ra4m1::event_number(event);
    *reinterpret_cast<volatile std::uint8_t*>(nvic_priority + line)
        = priority;
    *reinterpret_cast<volatile std::uint32_t*>(nvic_clear_pending) = mask;
}

void enable_icu_line(std::uint32_t line) noexcept {
    const auto mask = static_cast<std::uint32_t>(1U << line);
    *reinterpret_cast<volatile std::uint32_t*>(nvic_clear_pending) = mask;
    *reinterpret_cast<volatile std::uint32_t*>(nvic_enable) = mask;
}

void disable_icu_line(std::uint32_t line) noexcept {
    *reinterpret_cast<volatile std::uint32_t*>(nvic_disable)
        = static_cast<std::uint32_t>(1U << line);
}

void clear_icu_interrupt_request(std::uint32_t line) noexcept {
    auto& event_link = *reinterpret_cast<volatile std::uint32_t*>(
        icu_event_links + line * 4U);
    const auto value = event_link;
    event_link = value & ~(1U << 16U); // clear IELSRn.IR
}

void configure_sci2_interrupt_routes() noexcept {
    configure_icu_line(0U, mcuhw::mcu::ra4m1::event::sci2_rxi);
    configure_icu_line(1U, mcuhw::mcu::ra4m1::event::sci2_txi);
    configure_icu_line(2U, mcuhw::mcu::ra4m1::event::sci2_tei);
    configure_icu_line(3U, mcuhw::mcu::ra4m1::event::sci2_eri);
}

void enable_sci2_interrupts() noexcept {
    for (std::uint32_t line = 0U; line < 4U; ++line) {
        enable_icu_line(line);
    }
    __asm__ volatile("dsb sy\n\tisb sy\n\tcpsie i" ::: "memory");
}

void disable_sci2_interrupts() noexcept {
    for (std::uint32_t line = 0U; line < 4U; ++line) {
        disable_icu_line(line);
    }
    __asm__ volatile("dsb sy\n\tisb sy" ::: "memory");
}

namespace board = mcuhw::board::arduino_uno_r4_minima;

mcuhw::core::direct_mmio mmio;
application_execution_policy execution;

using binding = mcuhw::backend::ra4m1::sci_uart_binding<
    2, board::d0_rx, board::d1_tx>;
using backend_type = mcuhw::backend::ra4m1::sci_uart_backend<
    binding, mcuhw::core::direct_mmio, application_execution_policy>;

backend_type uart_hardware{mmio, execution};
mcuhw::driver::uart uart{uart_hardware};

constexpr auto uart_timing = mcuhw::backend::ra4m1::solve_uart_baud({
    .clock_hz = 24'000'000U,
    .target_baud = 115'200U,
    .maximum_error_ppm = 20'000U,
});
static_assert(uart_timing.has_value());

} // namespace

int main() {
    // Startup has initialized RAM, C++ objects, and board clocks.
    initialize_monotonic_clock();
    configure_sci2_interrupt_routes();

    if (!uart.initialize(*uart_timing)) {
        return 1;
    }
    enable_sci2_interrupts();

    const std::array<std::uint8_t, 2> message{'O', 'K'};
    auto started = uart.transmit(message);
    if (!started) {
        disable_sci2_interrupts();
        static_cast<void>(uart.shutdown());
        return 2;
    }

    auto transfer = std::move(*started);
    const auto deadline = mcuhw::execution::make_deadline(
        execution.now(), 4'800'000U);
    if (!deadline) {
        static_cast<void>(transfer.cancel());
        static_cast<void>(transfer.take());
        disable_sci2_interrupts();
        static_cast<void>(uart.shutdown());
        return 3;
    }

    const auto waited = transfer.wait_until(*deadline);
    const auto completed = transfer.take();
    const bool transferred = waited.has_value() && completed.has_value();

    disable_sci2_interrupts();
    const auto stopped = uart.shutdown();
    return transferred && stopped ? 0 : 4;
}

// Vector names and line assignments must match board interrupt routing.
extern "C" void IRQ0_Handler() noexcept {
    static_cast<void>(uart_hardware.handle_receive_interrupt());
    clear_icu_interrupt_request(0U);
}

extern "C" void IRQ1_Handler() noexcept {
    static_cast<void>(uart_hardware.handle_transmit_data_empty_interrupt());
    clear_icu_interrupt_request(1U);
}

extern "C" void IRQ2_Handler() noexcept {
    static_cast<void>(uart_hardware.handle_transmit_end_interrupt());
    clear_icu_interrupt_request(2U);
}

extern "C" void IRQ3_Handler() noexcept {
    static_cast<void>(uart_hardware.handle_receive_error_interrupt());
    clear_icu_interrupt_request(3U);
}
```

Do not copy the IRQ line numbers without the matching interrupt-controller
routing. A vector name alone does not connect SCI2 to that vector.

## Deadlines and transfer consumption

Create an absolute deadline from the same clock returned by the policy:

```cpp
const auto deadline = mcuhw::execution::make_deadline(
    execution.now(), timeout_ticks);
```

Important distinctions:

- A start error means the operation never began.
- A wait error can mean timeout, cancellation, peripheral failure, or an
  invalid wait from ISR context.
- `wait_until()` does not consume the handle.
- `take()` consumes the terminal result and makes the slot reusable.
- On timeout, `wait_until()` first quiesces the hardware.
- Code that does not block may poll `ready()` and later call `take()`.
- Before abandoning an unfinished operation, call `cancel()` and then
  `take()`.

Never read a receive buffer, change a transmit buffer, or let either leave
scope until the transfer is terminal and `take()` has returned.

## Interrupt-handler rules

An application ISR should:

1. call only the handler for the event routed to that IRQ line;
2. let the backend acknowledge the peripheral source;
3. clear the interrupt-controller request when required by the MCU;
4. remain bounded and never wait;
5. leave result processing to normal application context.

`wait_until()` rejects ISR context. The ISR publishes completion; it does not
consume the transfer.

## Startup and shutdown order

Recommended startup order:

1. Initialize data/BSS, stack, clocks, and the C++ runtime.
2. Initialize the monotonic time source.
3. Make MMIO, policy, backend, and driver objects available.
4. Configure IRQ routes with peripheral IRQs disabled.
5. Initialize the driver and its pins.
6. Clear stale pending state and enable the selected IRQ lines.
7. Start application transfers.

Recommended shutdown or reconfiguration order:

1. Stop starting new transfers.
2. Wait for or cancel every transfer and call `take()`.
3. Disable and detach the peripheral IRQ routes.
4. Call the driver/backend shutdown or release function.
5. Only then reuse pins, buffers, DMA channels, or object storage.

Continue with the [examples index](../examples/README.md). Each peripheral page
adds its binding, timing, buffer, and hardware-specific notes to this common
application structure.
