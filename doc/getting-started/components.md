# Devices attached to I2C and SPI

<!-- @req REQ-0046 -->

An attached device owns an endpoint on an existing bus. The application starts
the board and bus, chooses the wiring, and passes the endpoint to the device.
The device implements its register protocol and exposes operations such as
reading a temperature or sending an Ethernet frame.

The layers are:

```text
application / mcuip
        |
device::tmp102 or device::w5100
        |
io::i2c_device or io::spi_device
        |
io bus -> controller backend -> MMIO
```

Device code belongs under `include/mcubsp/device/`, grouped by function. Bus
endpoints live in `include/mcubsp/io/`; connections to another framework live
in the sibling `../integration` project. Device implementations receive the narrow
transport interface they need instead of a whole board. The TMP102 can use a
fake endpoint with no `io`, board, or backend headers. The W5100 separately
exposes its register protocol and the SPI transport that connects it to `io`.

The optional `mcubsp::device` CMake target depends on the core library; use
`mcubsp::io` for board composition. The linked UNO examples
[TMP102](../../example/tmp102/arduino/uno_r4_minima.cpp) and
[W5100](../../example/w5100/arduino/uno_r4_minima.cpp) are included with
`MCUBSP_BUILD_IO_EXAMPLES=ON`. Their builds and host simulations do not establish
physical acceptance of these new device compositions.

## Board and bus setup

Include the desired device and board explicitly. For example:

```cpp
#include <mcubsp/io/board.hpp>
#include <mcubsp/board/arduino/uno_r4_minima.hpp>
#include <mcubsp/device/sensor/tmp102.hpp>

namespace io = mcubsp::io;
namespace device = mcubsp::device;
using namespace std::chrono_literals;

constexpr io::board_config peripherals{
    .serial = false,
    .i2c = true,
    .spi = true,
};
io::board<mcubsp::board::arduino::uno_r4_minima, peripherals> hw;
```

After the board's clock/startup code has run, the application calls
`io::require(hw.begin())` and starts the buses it uses. The device constructors
do not configure clocks, start a controller, or allocate memory. See the
[I/O guide](io.md) for board startup and execution policy requirements.

## I2C address ownership

```cpp
io::require(hw.i2c.begin(400'000));
auto thermometer = io::require(hw.i2c.device(0x48));
auto eeprom = io::require(hw.i2c.device(0x50));
```

`device(address)` accepts an unshifted seven-bit address in `0x08..0x77` and
returns `io::result<io::i2c_device<...>>`. Each endpoint exclusively claims its
address. A second claim for `0x48` fails with `io::errc::busy`. Raw addressed
bus methods also reject a claimed address, so another caller cannot silently
change the thermometer's register pointer. Raw bus methods remain available
for unclaimed addresses.

Endpoints offer `write`, `read`, and `write_read`, with `_async` and `_until`
variants. `write_read` is one controller operation with a repeated START:

```cpp
const std::array<std::uint8_t, 1> register_pointer{0x00};
std::array<std::uint8_t, 2> response{};
auto completed = thermometer.write_read(register_pointer, response, 20ms);
```

An async operation borrows both data buffers until it is terminal. Move the
endpoint if needed; the pending transfer state remains in the immovable bus.
Consume the operation with `finish`, `finish_until`, or `take`. Cancellation
makes the operation terminal; consume that result as well.

## TMP102 temperature sensor

`mcubsp/device/sensor/tmp102.hpp` implements `device::tmp102<Endpoint>`. Passing
the endpoint by move transfers its address claim to the sensor:

```cpp
device::tmp102 temperature{std::move(thermometer)};
auto begun = temperature.begin(100ms);
if (!begun) {
    // begun.error().code identifies a device or transport failure.
    // begun.error().transport preserves the endpoint error when present.
    return;
}

auto sample = temperature.read(20ms);
if (sample) {
    const auto fixed = sample->celsius_sixteenths; // 400 means +25 degrees C
    const auto micro = sample->micro_celsius();   // 25,000,000 for +25 degrees C
    // Use fixed or micro in application code.
}
```

These are application fragments; check the returned result before using its
value. Sensor results have their own error type and are not accepted by
`io::require`.

`begin` reads the configuration and checks its fixed resolution/reserved bits.
It preserves the current settings; the check is not a unique chip identity
probe. `read` returns the latest completed sample, including signed values and
both normal and extended formats. It determines the format from the sample's
mode bit instead of a cached configuration value. It does not request a new
conversion or wake a sensor in shutdown mode. After power-up the hardware can
report zero until its first conversion completes. These semantics follow
[TI's TMP102 datasheet, SBOS397I, sections 6.3.1 and 6.5](https://www.ti.com/lit/ds/symlink/tmp102.pdf).

The sensor is movable and has no packet buffers, dynamic allocation, global
registration, or dependency on a particular board. Destroying it releases its
endpoint's address claim.

## ES8311 audio codec

`mcubsp/device/audio/es8311.hpp` provides `device::es8311` register definitions
and the existing fixed startup sequence for 12.288 MHz MCLK, 48 kHz, 16-bit
Philips I2S, analog microphone input and DAC output. It currently contains
configuration data; the application applies the writes through I2C and
coordinates I2S clocks and amplifier enable. The ESP32-P4-NANO board
metadata supplies the codec address and wiring. The codec definitions have no
board or controller dependency.

## SPI chip select and per-device settings

SPI endpoints own a chip-select pin and the configuration required by one
attached device:

```cpp
io::require(hw.spi.begin());
auto ethernet_spi = io::require(hw.spi.device({
    .cs = hw.pins.d10,
    .frequency = 12'000'000,
    .mode = io::spi_mode::mode0,
}));
```

The controller pins are shared by endpoints. A CS pin is exclusively claimed
through the same registry as ordinary GPIO. Each transaction applies that
endpoint's frequency, mode, bit order, and frame width, then keeps CS asserted
across all segments until the controller reaches wire idle. The next device
can then use its own settings. The configured frequency is validated against
the board's divider capabilities; `frequency()` reports the achieved value.

An endpoint offers `transaction`, `transaction_async`, and
`transaction_until`, plus scalar/span `transfer` conveniences. Async segment
arrays and their data buffers remain borrowed until the operation is terminal.
Explicit DMA is available through `transfer_async(buffers, io::use_dma(...))`
on controllers that support it. A checked synchronous `transaction_immediate`
path is available where supported by the controller; it observes both a
deadline and a finite polling limit.

Direct `hw.spi.transfer(...)` and `hw.spi.transaction(...)` still use the
board's default CS. That CS is claimed on first direct use. Claiming it as an
explicit endpoint first prevents direct bus calls from bypassing the endpoint.

## W5100 Ethernet adapter

`mcubsp/device/ethernet/w5100.hpp` implements an attached `device::w5100` that
owns an SPI endpoint and presents raw Ethernet frames:

```cpp
#include <mcubsp/device/ethernet/w5100.hpp>

device::w5100 ethernet{std::move(ethernet_spi)};
auto begun = ethernet.begin({
    .mac = {{0x02, 0x00, 0x00, 0x04, 0x51, 0x00}},
    .payload_bytes_per_poll = 64,
}, 1s);
```

Check `begun` before using the device. Board/shield wiring stays in the
application: on an Arduino Ethernet Shield R3, keep D4/SD CS high before using
the W5100 on D10. Claim that output with `hw.output(hw.pins.d4, true)` and keep
its GPIO handle alive. It is not a
W5100 protocol responsibility.

`begin()` checks the endpoint for the original W5100's mode 0, eight-bit,
MSB-first, active-low-CS format and a conservative 14 MHz clock ceiling. It
rejects incompatible settings before accessing the chip. The UNO transport
uses its checked immediate SPI path; other profiles use regular transactions.

Provide receive storage through `arm_receive(slot, buffer)` and transmit
storage through `start_transmit(slot, frame)`. Call `poll()` regularly, then
consume results with `take_receive` and `take_transmit_result`. Receive buffers
must provide at least `buffer_size` bytes. Queued transmit frames and armed
receive buffers remain borrowed across polling calls until completion or
cancellation. The number of slots is a template parameter; frame storage is
owned by the caller.

Each polling call copies at most `payload_bytes_per_poll` payload bytes across
RX and TX. Register housekeeping has separate finite limits. The default
`poll()` time budget is 10 ms; `poll_until(deadline)` lets the application share
an existing deadline and returns a typed error. Calls are synchronous within
that budget; a polling quantum is not a worker thread. `cancel_all()` releases
software frame ownership; it cannot retract a frame already copied to the
chip. If a SEND command was already issued, call `recover()` before submitting
another frame. Ambiguous pointer/command commits also require recovery;
partial uncommitted copies resume across polling calls and deadlines.

The device uses socket 0 MACRAW mode. IP/TCP/UDP belong to the network stack.
The sibling [integration project](../../../integration/README.md) provides
`mcuintegration/mcubsp_mcuip/w5100.hpp` and the board-independent
`mcuintegration::mcubsp_mcuip::w5100_network_device` alias. Link its explicit
`mcuintegration::mcubsp_mcuip` target to check the actual `mcuip::NetworkDevice`
contract. The adapter imposes no RTOS and allocates no network storage.
W5100 protocol and transport headers remain under `mcubsp/device/ethernet/`.

`device::w5100` is immovable because its internal protocol objects refer to its
transport. Its board and SPI controller must outlive it. The reusable register
protocol is separately available as `device::w5100_macraw_device<RegisterBus>`
for other transports and protocol tests.

## Deadlines, storage and lifetime

Use one absolute deadline for a component operation comprising several bus
transactions. The endpoint's `deadline_after(duration)` creates a deadline
using the board's clock. Pass that same value to each `_until` operation; do
not restart the full timeout for each register byte. Expired deadlines reject
new endpoint transfers before they touch hardware.

Endpoint objects are move-only handles. The board/controller remains at a
stable address, and outstanding operations refer to that storage. Endpoint
claims keep the board alive; bus/board shutdown returns `busy` while a device
claim exists. Explicit `release()` succeeds only after all outstanding bus
operation handles have been consumed or destroyed after completion. This
currently applies conservatively to other devices' operations on the same
bus too. Even a completed result still held by a caller prevents endpoint
release. Destruction with an outstanding operation violates the lifetime
contract and invokes the failure policy.

There is no hidden transaction queue, allocator, address discovery, or device
registry. I2C ownership uses a fixed 128-bit address map in an enabled I2C bus;
SPI uses endpoint CS claims and the board's pin registry. Controller and DMA
storage remain selected by `board_config`. Additional endpoints consume their
own handle/configuration storage, not a new controller backend.

The current implementation supports one foreground execution context. A busy
controller rejects overlapping work. Execution policy injection provides the
existing time/wait/critical-section seam, but these devices and endpoint
registries do not yet provide thread-safe access from multiple RTOS tasks.
Bus operations from an ISR are rejected. There is no camera, RTOS scheduler, network
socket layer, or global device-discovery framework in this implementation.

SPI shutdown failures may leave controller pins partially restored. New work
is rejected until `shutdown()` succeeds and the bus is started again. A failed
non-DMA CS deassertion can be retried with `hw.spi.recover_select()` after all
operation handles have been consumed. That function does not recycle retired
operation slots or recover failed DMA quiescence; those require target recovery.
