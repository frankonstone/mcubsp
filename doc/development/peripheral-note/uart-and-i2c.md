# UART and I2C implementation notes for RA4M1

The UART and I2C interfaces separate portable driver contracts from the
RA4M1 register backend. The caller owns all transfer storage, and the returned
move-only transfer handle represents the single in-flight operation. No heap,
exceptions, RTTI, scheduler, or hidden thread is required.

## Compile-time board binding

The UNO R4 Minima uses SCI2 on D0/D1 and IIC1 on A4/A5:

```cpp
using uart_binding = mcubsp::backend::ra4m1::sci_uart_binding<
    2, mcubsp::board::arduino::uno_r4_minima_bindings::d0_rx,
    mcubsp::board::arduino::uno_r4_minima_bindings::d1_tx>;
using i2c_binding = mcubsp::backend::ra4m1::iic_binding<
    1, mcubsp::board::arduino::uno_r4_minima_bindings::a5_scl,
    mcubsp::board::arduino::uno_r4_minima_bindings::a4_sda>;
```

Invalid instance/pin combinations fail at compile time. A backend claims its
peripheral instance at initialization and releases it at shutdown.

## Timing

`solve_uart_baud` searches the documented RA SCI asynchronous sampling modes
and all four CKS divisors. Its result records the requested and achieved baud,
error in ppm, and the exact SMR/BRR/SEMR encoding. A result is rejected when
the requested tolerance cannot be met.

`solve_i2c_timing` supports Standard mode through 100 kHz and Fast mode through
400 kHz. The caller supplies estimated bus rise and fall times. The solver
checks the mode limits and minimum high/low periods, then returns the concrete
CKS, BRL, and BRH values plus achieved frequency and error. The current RA4M1
backend programs normal IIC mode with SCLE enabled and the noise filter off;
these assumptions are part of the solver rather than hidden register defaults.

## UART transfers

Start receive before transmit for a loopback or request/response exchange:

```cpp
std::array<std::uint8_t, 8> rx{};
const std::array<std::uint8_t, 8> tx{0x55, 0xaa, 1, 2, 3, 4, 5, 6};

auto receiving = uart.receive(rx);
auto transmitting = uart.transmit(tx);
auto rx_transfer = std::move(*receiving);
auto tx_transfer = std::move(*transmitting);

auto rx_result = rx_transfer.wait_until(deadline);
auto tx_result = tx_transfer.wait_until(deadline);
```

RX completes only after the requested finite span is full. It never writes
beyond that span. TX completes from the transmit-end interrupt only after the
shift register reports TEND, so completion means the final stop bit is on the
wire, not merely copied to TDR. Overrun, framing, and parity faults are distinct
`uart_error` values. While transmission is disabled, the RA4M1 backend uses
SPTR to drive TXD at the asynchronous mark level instead of leaving it
high-impedance; this prevents a receiver from interpreting startup as a frame.

The application must route SCI RXI, TXI, TEI, and ERI events to interrupt
vectors and call the corresponding backend handlers. Polling a transfer with
`ready()` and `take()`, or blocking with `wait_until`, uses the same operation;
blocking is supplied by the execution policy and does not change the backend.

## I2C transactions and fixed storage

Addresses are unshifted seven-bit values. `make_i2c_address` rejects reserved
ranges. `write`, `read`, and `write_read` reject empty phases explicitly. A
combined `write_read` retains the bus and its operation slot across the repeated
START and issues one STOP at the end.

`i2c_device` binds an address without owning or copying the controller. Its
fixed-capacity request object owns both the register command bytes and response
storage:

```cpp
mcubsp::driver::i2c_device<decltype(bus), 0x50> eeprom{bus};
mcubsp::driver::i2c_register_request<16, 2> request;
auto started = eeprom.read_register(0x0120, request, 8);
auto transfer = std::move(*started);
auto result = transfer.wait_until(deadline);
```

`request` and its buffers must outlive the transfer handle. The same device
source works with a fake bus in native tests and the real RA4M1 controller.

## Errors, cancellation, and recovery

The I2C backend distinguishes address NACK, data NACK, arbitration loss,
hardware timeout, and a stuck bus. A normal failure requests STOP and does not
reset shared hardware. Cancellation and software timeout synchronously disable
the operation's interrupts, reset/reprogram the controller, and only then make
the operation slot reusable.

`recover_bus(exclusive_peripheral_reset)` is an explicit, bounded recovery for
an exclusively owned peripheral. It performs an internal IIC reset and restores
the saved timing. It does not bit-bang SCL, steal GPIO ownership, or reset an
unrelated peripheral. With policy `none`, a stuck bus is reported to the caller.

I2C is open-drain and requires pull-ups sized for the bus capacitance and target
rate. The UNO A4/A5 hardware test therefore uses an external device or fixture
with pull-ups; the driver does not silently enable weak internal pull-ups.

The reproducible board fixture uses a dedicated 24LC256-compatible EEPROM at
unshifted address `0x50`: connect A5 to SCL, A4 to SDA, VCC to 3.3 V, GND to
GND, hold WP low, and fit external pull-ups from SCL and SDA to 3.3 V. The
`i2c` board image preserves the eight bytes at EEPROM address `0x0010`, writes
and reads a test pattern with a combined repeated-START transaction, restores
the original bytes, and verifies the restoration. It repeats the scenario at
100 kHz and 400 kHz. Run it with:

```sh
cmake --preset arm-cortex-m4 -B output/build/uno-r4-minima -DMCUBSP_BOARD_TESTS=arduino/uno_r4_minima
cmake --build output/build/uno-r4-minima --target board-tests
tool/run-uno-r4-minima-tests.sh i2c
```

Capture A4 and A5 with a logic analyzer during the run. The RTT result records
the configured and achieved rates, but it is not an independent waveform or
protocol measurement.

The standalone [UART](../../../src/board/arduino/uno_r4_minima/doc/example/uart.md) and
[I2C](../../../src/board/arduino/uno_r4_minima/doc/example/i2c.md) guides include their code, wiring, lifecycle, and
bring-up hints. Native fixtures additionally inject every IRQ transition,
timeout, NACK class, arbitration loss, cancellation, and slot reuse without
wall-clock sleeps.
