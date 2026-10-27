# UART

This example configures SCI2 on UNO D0/D1 for 115200 baud, 8 data bits, no
parity, and one stop bit, then transmits `OK`.

The `application_execution_policy` and board-owned initialization functions
used below are defined in the
[bare-metal application integration guide](../../../../../../doc/concept/application-integration.md).

## Wiring

Connect D1/TX to the receiver's RX and D0/RX to its TX, plus a common ground.
Use 3.3 V logic. Do not connect an RS-232 voltage-level port directly; add a
level translator. A local loopback test can jumper D1 to D0.

## Code

The transfer helper receives an already-constructed driver. The backend itself
must have application lifetime so the SCI interrupt handlers can reach it.

```cpp
#include <array>
#include <utility>

#include <mcubsp/backend/ra4m1/sci_uart.hpp>
#include <mcubsp/board/arduino/uno_r4_minima/bindings.hpp>
#include <mcubsp/core/direct_mmio.hpp>
#include <mcubsp/driver/uart.hpp>

using board = mcubsp::board::arduino::uno_r4_minima_bindings;

using binding = mcubsp::backend::ra4m1::sci_uart_binding<
    2, board::d0_rx, board::d1_tx>;

constexpr auto timing = mcubsp::backend::ra4m1::solve_uart_baud({
    .clock_hz = 24'000'000U,
    .target_baud = 115'200U,
    .maximum_error_ppm = 20'000U, // 2%
});
static_assert(timing.has_value());

template<class Uart>
int send_ok(Uart& port, mcubsp::execution::deadline deadline) {
    if (!port.initialize(*timing)) {
        return 1;
    }

    const std::array<std::uint8_t, 2> message{'O', 'K'};
    auto started = port.transmit(message);
    if (!started) {
        return 2;
    }

    auto transfer = std::move(*started);
    const auto waited = transfer.wait_until(deadline);
    const auto completed = transfer.take();
    if (!waited || !completed) {
        return 3;
    }

    return 0;
}
```

Compose the objects outside `main()`, initialize them in `main()`, and
dispatch the four SCI events to the same backend:

```cpp
// application_execution_policy and initialize_monotonic_clock() are defined
// in the bare-metal application integration chapter.
mcubsp::core::direct_mmio mmio;
application_execution_policy execution;

using backend_type = mcubsp::backend::ra4m1::sci_uart_backend<
    binding, mcubsp::core::direct_mmio, application_execution_policy>;
backend_type uart_hardware{mmio, execution};
mcubsp::driver::uart port{uart_hardware};

int main() {
    initialize_monotonic_clock();
    configure_sci2_interrupt_routes();

    if (!port.initialize(*timing)) {
        return 1;
    }
    enable_sci2_interrupts();
    const auto deadline = mcubsp::execution::make_deadline(
        execution.now(), 4'800'000U);
    const bool sent = deadline && send_ok(port, *deadline) == 0;
    disable_sci2_interrupts();
    if (!sent) {
        static_cast<void>(port.shutdown());
        return 2;
    }
    return port.shutdown() ? 0 : 3;
}

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

The IRQ line numbers are examples. `configure_sci2_interrupt_routes()`, the
vector table, and the handler names must agree on the actual board mapping.

The source buffer must remain alive until `take()` returns. A receive works the
same way with a writable `std::span<std::uint8_t>`; the destination cannot be
read or reused while the operation is active.

Route the SCI receive, transmit-empty, transmit-end, and error interrupts to
the backend handlers. A transfer that starts but never becomes ready usually
means an interrupt was not enabled or was routed to the wrong SCI instance.

## Hardware check

Jumper D1 to D0, then run:

```sh
./tool/run-uno-r4-minima-tests.sh uart
```

When talking to another device, verify that its baud tolerance is compatible
with `timing->error_ppm`, not merely that both sides requested “115200”.
