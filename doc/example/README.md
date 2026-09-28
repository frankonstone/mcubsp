# Examples

These pages are reader-facing introductions to the public `mcuhw` API. They
put peripheral code, wiring, lifecycle rules, and troubleshooting hints next
to each other instead of sending the reader to a test implementation.

The peripheral call is only one part of a firmware application. Before using
UART, I2C, SPI, ADC, DMA, or PWM period events, read
[Integrating mcuhw into a bare-metal application](../concept/application-integration.md).
That chapter contains the concrete execution policy and shows how startup,
`main()`, static backend objects, the vector table, IRQ dispatch, deadlines,
buffers, and transfer handles fit together.

Start with GPIO, then read the transfer lifecycle below before using an
interrupt-driven peripheral:

1. [GPIO](gpio.md)
2. [PWM](pwm.md)
3. [UART](uart.md)
4. [I2C](i2c.md)
5. [SPI](spi.md)
6. [ADC](adc.md)
7. [DMA](dma.md)
8. [W5100 Ethernet](ethernet-w5100.md)

## Common setup

The examples target the Arduino UNO R4 Minima and use
`mcuhw::core::direct_mmio`. Asynchronous backends also take an application
execution policy. That policy supplies critical sections, notification,
waiting, a monotonic clock, and ISR-context detection. In the snippets it is a
template parameter named `ExecutionPolicy`:

```cpp
template<class ExecutionPolicy>
int run_example(ExecutionPolicy& execution) {
    mcuhw::core::direct_mmio mmio;
    // Construct the selected backend with mmio and execution here.
    return 0;
}
```

In an actual application, MMIO, the execution policy, and an
interrupt-driven backend normally have static lifetime:

```cpp
mcuhw::core::direct_mmio mmio;
application_execution_policy execution;
peripheral_backend hardware{mmio, execution};
functional_driver peripheral{hardware};

int main() {
    initialize_monotonic_clock(); // implementation is in the integration guide
    configure_interrupt_routes(); // pseudocode: application/board-specific
    // initialize and use peripheral
}

extern "C" void Peripheral_IRQHandler() noexcept {
    static_cast<void>(hardware.handle_event_interrupt());
    clear_interrupt_controller_request(); // pseudocode: MCU-specific
}
```

`application_execution_policy`, `initialize_monotonic_clock()`, interrupt
routing, and the vector name belong to the application. Their complete UNO
shape is shown in the integration chapter. Registering pins and constructing a
backend is not enough to make an asynchronous transfer finish.

`configure_interrupt_routes()`, `Peripheral_IRQHandler`, and
`clear_interrupt_controller_request()` above describe responsibilities; they
are not symbols exported by `mcuhw`. The integration chapter provides real
RA4M1 ICU helpers and SCI2 event routing that can be adapted for each
peripheral.

| Example | Completion model |
| --- | --- |
| GPIO | Synchronous; no execution policy |
| Basic PWM start/stop/duty | Synchronous control; period notification uses policy and GPT IRQ |
| UART | SCI interrupts publish transmit/receive completion |
| I2C | IIC interrupts advance the bus state machine |
| SPI | SPI interrupts advance segments and publish completion |
| ADC | Scan-end interrupt publishes conversion completion |
| DMA | Transfer-end interrupt publishes completion |
| W5100 immediate transport | Bounded polling at the SPI/W5100 boundary |

## Transfer lifecycle

UART, I2C, SPI, ADC, PWM period events, and DMA return move-only transfer
handles. The caller-owned buffers referenced by an operation must remain alive
and unchanged until the handle has been consumed.

```cpp
auto started = peripheral.start_operation(buffer);
if (!started) {
    return start_failed;
}

auto transfer = std::move(*started);
const auto waited = transfer.wait_until(deadline);
if (!waited) {
    // wait_until() quiesces the hardware on timeout. take() retires the slot.
    static_cast<void>(transfer.take());
    return transfer_failed;
}

const auto completed = transfer.take();
if (!completed) {
    return transfer_failed;
}
```

Never discard a live handle. Either wait and call `take()`, or call `cancel()`
and then `take()`. A deadline is an absolute tick value in the execution
policy's monotonic clock, not a duration.

## Running the hardware examples

Build and flash one physical suite at a time:

```sh
cmake --preset arm-cortex-m4 -B build/uno-r4-minima -DMCUHW_BOARD_TESTS=arduino/uno_r4_minima
cmake --build build/uno-r4-minima --target board-tests -j4
./tool/run-uno-r4-minima-tests.sh <suite>
```

The suite name is shown on each page. Confirm the wiring before flashing,
especially when a shield or external supply is attached.
