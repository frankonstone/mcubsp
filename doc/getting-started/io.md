# The `mcubsp::io` application interface

`mcubsp::io` provides board objects, named pins, serial I/O, I2C, SPI, ADC,
PWM, and explicit DMA selection over the existing checked drivers. It uses
C++23, fixed storage, and `std::expected` results. Include both the interface
and your board profile, and link the `mcubsp::io` CMake target.

The current interface runs in one foreground context on one core. Hardware
startup still supplies the clock tree, memory initialization, and vectors;
constructing an `io::board` does not touch hardware. `begin()` activates the
runtime and enables its default time source. It does not boot the MCU or
configure a new system clock tree.

## First program

The [UNO blink example](../../example/blink/arduino/uno_r4_minima.cpp) uses the repository's
UNO startup and checks that its clock initialization succeeded:

```cpp
#include <chrono>
#include <mcubsp/io/board.hpp>
#include <mcubsp/board/arduino/uno_r4_minima.hpp>

namespace io = mcubsp::io;
using namespace std::chrono_literals;

constexpr io::board_config configuration{.serial = false};
static io::board<mcubsp::board::arduino::uno_r4_minima, configuration> hw;

int main() {
    if (!hw.startup_ok()) __builtin_trap();
    io::require(hw.begin());
    auto led = io::require(hw.output(hw.pins.led_builtin));
    for (;;) {
        io::require(led.toggle());
        io::require(hw.delay(500ms));
    }
}
```

`output(pin, initial_high)` and `input(pin, io::pull::up)` return move-only
pin leases. Outputs support `write(bool)` and `toggle()`; inputs and outputs
support `read()`. Destroying a pin lease releases it and configures the pin
as an input. `release()` performs this explicitly and reports any error.

The runtime rejects duplicate pin claims across GPIO and peripherals. For
example, UNO D13 is both the built-in LED and SPI clock: release the LED
lease before starting SPI.

## Select the storage you need

`board_config` is a compile-time configuration. Serial storage is enabled by
default. I2C, SPI, ADC, PWM, and all DMA channels are disabled by default.
GPIO is always available. An enabled peripheral is initialized by its own
`begin()` call; selecting storage does not start the peripheral.

```cpp
constexpr io::board_config configuration{
    .serial = true,
    .i2c = true,
    .spi = true,
    .dma = io::channels<0, 1>,
    .adc = true,
    .pwm = true,
};
static io::board<mcubsp::board::arduino::uno_r4_minima, configuration> hw;
```

Disabled features omit their backend storage. Calling a disabled feature or
selecting an unsupported ADC, PWM, or DMA binding fails at compile time.
The feature flag `.adc` enables the `hw.analog` member.

## Everyday operations

These snippets assume `hw.begin()` has succeeded and the relevant features
were selected. Include `<array>` for their buffers. All buffers remain in
the calling scope until the blocking operation returns.

```cpp
io::require(hw.serial.begin(115200));
io::require(hw.serial.println("ready"));

std::array<std::uint8_t, 1> byte{};
auto received = hw.serial.read(byte, 100ms);
if (received) io::require(hw.serial.write(byte, 100ms));
```

The default serial format is 8N1. Reads request a finite buffer; they do not
provide an Arduino-style background receive queue. `println()` writes its
text and CRLF with one overall deadline. The [serial echo
example](../../example/serial_echo/arduino/uno_r4_minima.cpp) handles idle read timeouts.
On UNO this port is SCI2 on D0/D1; it is not USB CDC.

```cpp
io::require(hw.i2c.begin(100'000));
auto address = mcubsp::driver::make_i2c_address(0x48);
if (!address) __builtin_trap();
const std::array<std::uint8_t, 1> command{0x00};
std::array<std::uint8_t, 2> reply{};
static_cast<void>(io::require(hw.i2c.write_read(*address, command, reply, 100ms)));
```

I2C takes validated, unshifted seven-bit addresses. `write_read()` performs
one repeated-START transaction. `hw.i2c.device(address)` creates a move-only
address lease for an attached chip; raw bus calls cannot bypass that lease.
See the [components guide](components.md) for device drivers and shared buses.

```cpp
io::require(hw.spi.begin(1'000'000));
[[maybe_unused]] auto received_frame =
    io::require(hw.spi.transfer(std::uint16_t{0x9F}, 100ms));
const std::array<std::uint16_t, 2> outgoing{0x9F, 0x00};
std::array<std::uint16_t, 2> incoming{};
static_cast<void>(io::require(hw.spi.transfer(outgoing, incoming, 100ms)));

io::require(hw.analog.begin());
[[maybe_unused]] auto raw_sample = io::require(hw.analog.read(100ms));

io::require(hw.pwm.begin(20'000, io::percent(25)));
io::require(hw.pwm.duty(io::percent(75)));
```

CPU SPI accepts `std::span<const std::uint8_t>` and
`std::span<std::uint8_t>` for eight-bit wire transfers, without staging copies.
Wide-frame spans remain available. `driver::byte_segment(tx, rx)` constructs a
segment for an asynchronous or multi-segment transaction; segment arrays and
buffers stay alive until terminal completion. Byte segments reject 16-bit wire
mode and mixed byte/wide buffers. Direct bus calls use the profile's default chip select, claimed
on first use. `hw.spi.device({.cs = pin, .frequency = hz, .mode = mode})`
creates an endpoint with its own CS and precomputed settings. Endpoints share
one controller; overlapping operations return `busy`. `transaction()` accepts
multiple `driver::spi_segment` entries under one chip select.

ADC reads return a raw sample from the profile's single analog input; the
default resolution is 14 bits. `begin(driver::adc_configuration)` selects
supported resolution and sampling settings. Voltage conversion requires an
application-supplied reference. PWM uses an exact rational duty value;
`percent()` avoids floating point. Frequency is selected at `begin()`;
`stop()`, `start()`, and `shutdown()` control the existing waveform.

## Asynchronous operations and DMA

`write_async()`, `read_async()`, `transaction_async()`, and `sample_async()`
return move-only operations. Starting one does not
create a task or event loop. The application must regularly call `hw.poll()`
while doing other work, or enter a facade blocking operation, `delay()`, or
operation wait that performs polling.

```cpp
std::array<std::uint8_t, 3> packet{1, 2, 3};
auto operation = io::require(hw.serial.write_async(packet));
hw.poll(); // Perform bounded progress between application work.
io::require(operation.finish(100ms));
```

`ready()` only inspects state. `take()` consumes the terminal result once.
`finish(timeout)` waits and consumes it, including a terminal timeout or
cancellation error. `wait_until(deadline)` waits without consuming, and
`cancel()` synchronously quiesces the operation. An invalid timeout or
calling-context error can leave an operation pending: keep it alive and
finish or cancel it before releasing its buffers.

DMA is explicit on UNO. Acquire the selected channel leases and pass
`io::use_dma(channel)` to serial or analog operations. SPI requires receive
and transmit channels, in that order:

```cpp
auto rx_dma = io::require(hw.dma.channel<0>());
auto tx_dma = io::require(hw.dma.channel<1>());
alignas(4) std::array<std::uint8_t, 4> transmit{1, 2, 3, 4};
alignas(4) std::array<std::uint8_t, 4> receive{};

auto operation = io::require(hw.spi.transfer_async(
    {.transmit = transmit, .receive = receive},
    io::use_dma(rx_dma, tx_dma)));
static_cast<void>(io::require(operation.finish(100ms)));
io::require(rx_dma.release());
io::require(tx_dma.release());
```

The complete [SPI DMA example](../../example/spi_dma/arduino/uno_r4_minima.cpp) includes
initialization and wiring. Its buffers reside in RAM. The backend validates
DMA reachability and transfer constraints; requesting DMA does not silently
fall back to CPU transfer. SPI completion includes both DMA transfers and
peripheral idle. A DMA lease also offers `copy_async()` and `copy()` for
supported memory copies.

## Lifetimes, execution, and errors

Keep the board at a stable address for its whole lifetime. It is neither
copyable nor movable. Borrowed execution policies and MMIO objects must
outlive it. Pin leases, DMA leases, peripheral references, and operation
handles must not outlive the board. Buffers must remain alive and accessible
until their operation is terminal; asynchronous SPI segment arrays are
borrowed too. Do not modify transmit buffers or inspect receive buffers
while hardware owns them.

Destroying a terminal operation releases its slot. Destroying a pending
operation is a contract violation and traps. A completed but unconsumed
operation still retains the runtime. Consume or destroy it before shutting
the runtime down. Release GPIO and DMA leases as well. `shutdown()` can stop
idle peripherals before returning `busy` for a later peripheral or retained
handle; it is not an atomic all-or-nothing operation.

Use facade operations from one foreground context. They do not provide
concurrent task access or ISR operations. Do not run native peripheral
interrupt handlers concurrently with facade polling. Polling services DMA
before its consuming peripherals. A custom execution policy alone does not
turn this foreground interface into a thread-safe RTOS driver service.

Most blocking operations default to one second. Use integral chrono durations, including hours, minutes, seconds and
nanoseconds. Negative intervals and intervals exceeding
`2^31 - 1` clock ticks are rejected. The default cycle clocks therefore have
different maximum intervals: approximately 44.7 seconds on UNO, 33.6 seconds
on NUCLEO, and 6.0 seconds on the P4 profile. Cycles must continue advancing
at the configured frequency during waits.

Results are `io::result<T>` (`std::expected<T, io::error>`). Inspect
`error().code` for a common category and `source`/`native` for the underlying
driver cause. `io::require()` returns the successful value or traps. A
custom failure policy is a type with a static, nonreturning function:

```cpp
struct application_failure {
    [[noreturn]] static void fail(io::error error) noexcept;
};
// Implement fail() in the application, for example to record diagnostics
// and reset. It must never return.
io::require<application_failure>(hw.begin());
```

The selected peripheral's `driver()` returns a const view of its existing
backend. `unsafe_driver()` gives mutable access to that same backend.
Direct reinitialization, shutdown, or pin changes bypass the facade's
bookkeeping and require application responsibility for its invariants.

## Board profiles and clocks

| Profile | Serial | SPI | I2C | Analog / PWM | Generic DMA |
|---|---|---|---|---|---|
| `uno_r4_minima` | SCI2: D0 RX, D1 TX | SPI1: D10 CS, D11 MOSI, D12 MISO, D13 SCK | IIC1: A4 SDA, A5 SCL | A0 AN009 / D3 GPT1 B | Channels 0–3 |
| `nucleo_h753zi` | USART3: PD9 RX, PD8 TX, ST-LINK VCP | SPI1: D10 PD14 CS, D11 PB5 MOSI, D12 PA6 MISO, D13 PA5 SCK | I2C1: D14 PB9 SDA, D15 PB8 SCL | A0 PA3 INP15 / PA8 TIM1 CH1 | Unavailable |
| `esp32p4_nano` | UART0: GPIO38 RX, GPIO37 TX | SPI2: GPIO23 CS, GPIO21 MOSI, GPIO20 MISO, GPIO22 SCK | I2C0: GPIO7 SDA, GPIO8 SCL | Unavailable | Unavailable |
| `promicro_nrf52840` | UART0: P0.08 RX, P0.06 TX; UARTE selections | SPIM1: P0.17 CS, P0.22 MOSI, P0.24 MISO, P0.20 SCK; SPI/SPIM selections | TWI0: SDA P1.00, SCL P0.11 | Unavailable | Unavailable |

The [Pro Micro profile](../../src/board/others/promicro_nrf52840/README.md)
supplies GPIO, the active-high P0.15 LED, serial/SPI masters and CPU/DMA I2C, using the existing
UF2/S140-v6 bootloader layout and a 64 MHz CPU. Its cycle-clock limit is about
33.6 seconds. UF2 upload and user-observed LED Blink are recorded on one board;
broader physical verification, USB application serial and radio support remain
separate work. See the [serial family contracts](../acceptance/nrf52840-serial.md)
for explicit native instances, bounded streaming, DMA RAM limits and target APIs.

UNO assumes the repository startup's 48 MHz ICLK/PCLKA and 24 MHz PCLKB.
NUCLEO assumes 64 MHz HSI SYSCLK/HCLK/PCLK. Their default policies enable
DWT cycle counting without resetting its count.
With supplied startup, `hw.startup_ok()` reports the profile's startup status;
call it before `hw.begin()`. A custom profile implements
`static bool startup_ok() noexcept`; an external BSP owns its own startup
checks.

The P4 profile matches the existing NANO development board with v1.0 silicon,
HP CPU0 only, and a fixed 360 MHz CPU clock. UART/SPI/I2C use 40 MHz sources.
Its I2C reset/gate bits and CLIC status register follow that development
profile. This does not qualify production-v3 silicon. Zcmp builds require a
custom execution policy for the additional interrupt-threshold workaround.

Changing these clocks or silicon assumptions requires a corresponding board
composition. Each profile exposes `target_identity`; derived profiles inherit
it so they cannot create two active facade owners for the same hardware.
There are no camera, RTOS, or network adapters in `mcubsp::io` yet.

## Supply execution and MMIO

`make_board()` borrows an existing execution policy and optionally an MMIO
policy. Always pass their actual tick frequency when it differs from the
board's default. A host example uses the deterministic test policy:

```cpp
#include <mcubsp/execution/fake_policy.hpp>
#include <mcubsp/testing/recording_mmio.hpp>

mcubsp::execution::fake_policy policy;
mcubsp::testing::recording_mmio<0x80000, 4096> mmio{0x4000'0000U};
constexpr io::board_config configuration{.serial = false};
auto hw = io::make_board<mcubsp::board::arduino::uno_r4_minima, configuration>(
    policy, mmio, 1000U); // One tick is one millisecond.
io::require(hw.begin());
io::require(hw.delay(10ms));
io::require(hw.shutdown());
```

This does not access physical hardware. The fake policy advances time during
waits; register storage alone does not simulate a peripheral's completion.
For firmware, `make_board<Profile>(policy, frequency)` uses direct MMIO.
Initialize a borrowed policy yourself before calling `begin()`. Its required
surface is described in [execution and
transfers](../concept/execution-and-transfers.md).

## Build examples

The [example applications](../../example/README.md) share the sources used by
repository targets. The UNO examples use the existing Arm startup and linker image:

```sh
cmake --preset arduino-uno-r4-minima
cmake --build --preset arduino-uno-r4-minima --target examples
```

For application CMake targets, use
`target_link_libraries(your_firmware PRIVATE mcubsp::io)` alongside the
application's startup composition.

## Select peripheral bindings and additional instances

Include `<mcubsp/io/composition.hpp>` (also included by `mcubsp/io/board.hpp`). Native binding
types keep their existing instance/package/pin validation. `io::select` changes
the defaults without requiring a handwritten derived board profile:

```cpp
using inputs = mcubsp::backend::ra4m1::adc14_binding<
    mcubsp::backend::ra4m1::adc_input<1, {0, 1}>,
    mcubsp::backend::ra4m1::adc_input<0, {0, 0}>>;
using selected = io::select<mcubsp::board::arduino::uno_r4_minima,
                            io::analog_selection<inputs>>;
constexpr io::board_config config{.serial = false, .adc = true};
static io::board<selected, config> hw;
```

`hw.analog.sample(samples)` scans all bound inputs in binding order. A
multi-input selection has no scalar `read()`; pass one output element per
channel. DMA sampling requires a single-input binding. `serial_selection`, `i2c_selection`,
`spi_selection` and `pwm_selection` rebind the corresponding defaults.
Selections retain the original physical target identity and fixed clock tree.

For simultaneous instances, add selectors to `composed_board` and use indexed
peripherals. Only those instances consume additional storage:

```cpp
using extra_uart = io::serial_selection<
    mcubsp::backend::ra4m1::sci_uart_binding<0, {1, 0}, {1, 1}>>;
static io::composed_board<mcubsp::board::arduino::uno_r4_minima, {}, extra_uart> hw;
// After hw.begin():
// io::require(hw.serial.begin());          // default SCI2
// io::require(hw.peripheral<0>().begin());  // additional SCI0
```

`make_board<Profile, config, Selectors...>(policy, mmio, frequency)` supplies
borrowed execution/MMIO for the same composition. Polling and shutdown include
all selected instances; they share pin claims with GPIO and default peripherals.
Duplicate hardware instances fail ownership checks even when pins differ.
Only routes and capabilities already supported by the native backend are valid.

## Independent monotonic time

The default cycle counters retain their frequency/sleep assumptions. For longer
intervals, include `<mcubsp/execution/monotonic_policy.hpp>` and borrow a separate
clock with `now()` and static `ticks_per_second`:

```cpp
// application_clock is initialized by the BSP and remains monotonic through
// the sleep modes the application uses. At 1 kHz the maximum interval is 24.8 days.
mcubsp::execution::monotonic_policy policy{synchronization_policy, application_clock};
auto hw = io::make_board<mcubsp::board::arduino::uno_r4_minima>(
    policy, decltype(policy)::ticks_per_second);
```

The adapter separates clock units from synchronization-policy waits, forwarding
only a bounded wait step to the latter. It adds no timer interrupt, oscillator
or sleep policy. The application initializes both borrowed objects and keeps
them alive. Clock values can be wider than 32 bits; deadline ordering uses the
low 32 bits and the existing half-range limit.

## Diagnose and recover lifecycle failures

An execution policy can provide `void contract_violation(io::contract_failure)`.
It receives the component, reason, source location, and (for pending operations)
slot/generation before the mandatory trap. Store these values in application
fault storage or inspect them in a debugger; avoid recursive calls into the
failing peripheral. The default remains a trap with no logging dependency.

The [recovery example](../../example/recovery/arduino/uno_r4_minima.cpp)
shows finite serial reads, idle timeouts, explicit cancellation if a handle
remains pending, peripheral reinitialization and orderly shutdown. Its `q`
command exits the loop. A failed quiesce must be escalated; returning a buffer
to application use while hardware still owns it is not recovery.
