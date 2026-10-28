# Pro Micro nRF52840: GPIO, I2C and native system services

[Requirements](requirements.md) and [implementation plan](implementation-plan.md)
record this board's scope, work and acceptance boundaries. Full-peripheral support
is now required by REQ-0055–REQ-0074 and planned in PLN-0042–PLN-0053; the
board facade described below remains GPIO and finite I2C. Native system services
and their current limits are described in the [system guide](../../../../doc/reference/nrf52840/README.md).

This profile targets the nice!nano-compatible Pro Micro / SuperMini board family
described by [Zephyr](https://docs.zephyrproject.org/latest/boards/others/promicro_nrf52840/doc/index.html).
It supplies GPIO and finite I2C through `mcubsp::io`, explicit native system services,
a P0.15 active-high LED,
bootloader-handoff startup, and a linked Blink application with UF2 output.
It has no dependency on Zephyr, Nordic HAL libraries or a radio stack.

`V1940` is the user's reported marking. It is not interpreted as a verified
PCB revision or silicon stepping. `uf2-s140-v6` names the supported **boot
layout**, not a physical board revision. Clone-specific wiring, bootloader
version, silicon errata and electrical behavior still need physical qualification.

## Build

```sh
cmake --preset promicro-nrf52840
cmake --build --preset promicro-nrf52840 --target firmware
```

Outputs are `output/build/promicro-nrf52840/src/board/firmware.elf`, its build/input
records, and `firmware.elf.uf2`. The compiler is the existing Arm GNU Cortex-M4F
toolchain; CMake, Ninja and Python 3.10+ are used. No SDK download or board access
occurs during the build. For an independent application, copy `example/blink/`
and select `MCUBSP_BOARD=others/promicro_nrf52840` through
`cmake/board-toolchain.cmake`, as described in the application guide.

```cpp
#include <mcubsp/io/board.hpp>
#include <mcubsp/board/others/promicro_nrf52840.hpp>

using profile = mcubsp::board::others::promicro_nrf52840;
static mcubsp::io::board<profile, {.serial = false}> hw;
```

Use `hw.pins.led_builtin` or `hw.pins.p0_15`. Other GPIOs use native
`core::pin_id{port, index}` values; verify the actual connector against the
board's pinout before wiring. No Arduino-number mapping is assumed. The initial
backend conservatively rejects P0.00/P0.01 (LF crystal), P0.09/P0.10 (NFC) and
P0.18 (reset). It never rewrites UICR to repurpose them.

GPIO supports input/output, read, atomic set/clear, pull-up/down, standard drive
and high drive (`gpio_drive::middle`), and open drain. Output readback keeps the
input buffer connected. Toggle is a foreground read/write operation, not an
atomic hardware toggle. Pin release restores input with no pull or sensing.
Follow Nordic's electrical limits and the actual board wiring.

UART, SPI, ADC, PWM, generic DMA, USB serial and Bluetooth are not implemented
by this profile. Set `.serial = false`; enabling an unsupported feature fails
compilation. USB is used by the existing bootloader only. Blink intentionally
does not enumerate a USB application device.

## I2C and attached sensors

Enable I2C storage with `{.serial = false, .i2c = true}`, then call
`hw.i2c.begin(100'000)` after `hw.begin()`. The default TWI0 route follows the
Zephyr board pin control: **SDA P1.00 (label 100)** and **SCL P0.11 (label 011)**.
These are native port numbers, not Arduino D0/D1 or other connector aliases.
Use a common ground and external pull-ups to 3.3 V appropriate to bus capacitance;
the backend leaves the internal pulls disabled and configures open-drain I/O.
Confirm the actual clone's connector labels before connecting a device.

The CPU-driven native TWI peripheral supports finite writes, reads and one
write/repeated-START/read transaction. Nominal 100, 250 and 400 kHz hardware
encodings are supported against the 16 MHz peripheral clock. The board's default
timing assumptions are 300 ns rise, 100 ns fall and 2% nominal rate tolerance;
these are assumptions, not measured waveforms or calibrated oscillator accuracy.
The fixed CPU clock remains 64 MHz. There is no EasyDMA or generic DMA fallback.

`hw.i2c.device(0x48)` creates the existing move-only address lease. Devices share
one controller and overlapping transfers return busy; raw bus calls cannot
bypass a live address lease. Buffers remain borrowed until terminal completion.
Call `hw.poll()` for asynchronous progress or use the blocking operations, which
poll while waiting. The facade is single-foreground-context, without concurrent
ISR dispatch or multi-controller bus support.

The receiver suspends between bytes and arms the hardware final-byte STOP
shortcut before the last byte. Success is published only after STOPPED and
the expected byte counts. Hardware address/data NACK and overrun/transport faults
remain distinct where observable. Low SDA/SCL before a request returns
`bus_stuck`; no automatic line pulsing or UICR changes occur.

Timeout/cancel requests STOP with a maximum 1024 status-read cleanup budget,
then disables and checks the peripheral before reuse. This bounded cleanup may
reject a slowly stretching device; it is not a measured wall-clock interval.
Failed cleanup or MMIO failure faults the backend. Consume the terminal handle,
successfully shut down to release hardware, then construct a fresh backend/runtime
before reuse; the common operation pool's faulted slots are deliberately retired.

To rebind or add a controller, use `io::i2c_selection` with
`backend::nrf52840::i2c_binding<instance, scl_pin, sda_pin>`. TWI0 and TWI1 are
supported, including simultaneous instances with distinct pins. Binding rejects
duplicate/unavailable/reserved pads at compile time. A hardware instance has one
owner across bindings; the ENABLE check also refuses an already-enabled shared
TWI/TWIM/SPI block. Keep external HAL ownership separate explicitly.

The complete [TMP102 example](../../../../example/tmp102/others/promicro_nrf52840.cpp)
reads address 0x48 and lights the LED above 25 °C. Connect the sensor to the
default pins, power it at 3.3 V, and tie ADD0 to ground:

```sh
cmake --preset promicro-nrf52840
cmake --build --preset promicro-nrf52840 --target others.promicro_nrf52840.example.tmp102
```

This emits `output/build/promicro-nrf52840/example/others.promicro_nrf52840.example.tmp102.elf`
and its `.uf2`/input records. Building does not upload. Physical I2C transactions,
sensor readings and electrical timing have not yet been observed. See the
[I2C software verification record](../../../../doc/acceptance/nrf52840-i2c.md).

## Boot ownership and memory

The default is the Zephyr **UF2/S140-v6** partition layout:

| Region | Start | End (exclusive) |
| --- | --- | --- |
| Existing MBR / SoftDevice | `0x00000000` | `0x00026000` |
| Application | `0x00026000` | `0x000EC000` |
| Existing storage | `0x000EC000` | `0x000F4000` |
| Existing UF2 bootloader/settings | `0x000F4000` | `0x00100000` |
| Application RAM, SoftDevice disabled | `0x20000000` | `0x20040000` |

The linker reserves 4 KiB for the stack. Startup masks/clears inherited NVIC
delivery, disables SysTick, installs all 64 vector positions, enables the FPU,
copies data, zeros BSS, checks FICR.PART, and runs static constructors before
entering `main()`. Unused exceptions trap. No peripheral interrupts are enabled.

This startup requires the existing bootloader to hand off with SoftDevice
disabled after its Nordic silicon/errata initialization. It is **not** a raw
reset-at-zero port and does not implement its own complete silicon errata layer.
Preserving the SoftDevice binary does not enable BLE or reserve its runtime RAM.
If a different SoftDevice/layout is installed, use a separately reviewed profile;
changing only the flash origin is insufficient.

The CPU is fixed at 64 MHz. The default DWT cycle clock supports intervals up
to about 33.6 seconds and busy polling; low-power operation is not physically qualified. The native system APIs provide
a separately initialized RTC time source and guarded idle/retention operations;
see the system guide for ownership, errata and remaining scope.
Use an explicitly initialized borrowed policy/monotonic clock for other timing
or sleep behavior. With `STARTUP external`, the BSP owns silicon initialization,
the fixed-clock contract, vectors and its startup checks.

## Explicit UF2 upload

Bridge RST to GND twice quickly to enter the existing UF2 bootloader. Mount its
drive and inspect `INFO_UF2.TXT` for the actual board/bootloader identity and
SoftDevice information. Confirm that the S140-v6 application layout matches.
Use the actual mount path (the drive name varies):

```sh
python3 tool/hardware.py flash --board others/promicro_nrf52840 \
  --elf output/build/promicro-nrf52840/src/board/firmware.elf \
  --mount /absolute/path/to/bootloader-drive --dry-run
```

The plan checks the firmware/input records, the mounted nRF52840 Board-ID, and
that the generated UF2 exactly matches the ELF and application partition. The
Board-ID establishes MCU family, not clone revision or SoftDevice version.
Remove `--dry-run` to write `FIRMWARE.UF2` to that explicit mount. This is the
flashing action; the bootloader normally disconnects/reboots afterward. Never
copy the ELF itself, a zero-based image, or an unrelated bootloader image.

The expected observation is LED P0.15 toggling every 500 ms. Record the exact
board, FICR identity, `INFO_UF2.TXT`, image/input identities and observation before
claiming physical acceptance. Repeat double-reset entry after Blink to verify
bootloader recovery. On 27 October 2026, the UF2 upload completed on the user's
V1940-marked board (UF2 0.6.0, Board-ID `nRF52840-nicenano`, S140 6.1.1), and the
user confirmed the LED was blinking. See the
[image-bound observation](../../../../doc/acceptance/promicro-nrf52840.md).
Actual timing and post-application double-reset recovery remain unmeasured.

If double reset cannot enter the bootloader, use the back-side SWD pads and the
documented recovery procedure for that exact clone. Preserve a matching factory
bootloader/SoftDevice image first. Recovery erasure is a separate, destructive
operation, not part of the mcubsp build or UF2 upload workflow.

## Source baseline and scope

- [Zephyr board documentation](https://docs.zephyrproject.org/latest/boards/others/promicro_nrf52840/doc/index.html),
  reviewed 27 October 2026; family identity and LED/boot procedure.
- `boards/others/promicro_nrf52840/promicro_nrf52840_nrf52840_common.dts`,
  reviewed Git blob `762b6931b064155b52a93cbe9bc1b0b321ce093a`:
  QIAA device and active-high P0.15 LED. Its peripheral pin choices are not
  presented as Arduino connector numbering.
- [UF2/S140-v6 partition definition](https://github.com/zephyrproject-rtos/zephyr/blob/main/dts/vendor/nordic/nrf52840_partition_uf2_sdv6.dtsi):
  reviewed Git blob `eb9c36b39386a0552f132ea851c5048c4c820ed8`;
  application `0x26000 + 0xC6000`, storage `0xEC000 + 0x8000`, boot
  `0xF4000 + 0xC000`. The C++ binding, linker, board settings and tests retain
  those reviewed values independently of future upstream changes.
- Nordic [nrfx v3.2.0 MDK](https://github.com/NordicSemiconductor/nrfx/tree/v3.2.0/mdk),
  `nrf52840.h` and `nrf52840_bitfields.h`: GPIO bases, offsets and encodings,
  FICR identity; `system_nrf52.c`: fixed CPU frequency and reset ownership.
  GPIO and TWI use this MDK baseline. Transfer sequencing also follows the
  [nrfx v3.2.0 TWI reference](https://github.com/nordicsemi/nrfx/blob/v3.2.0/drivers/src/nrfx_twi.c),
  including RX byte-boundary suspend/STOP and TX/RX restart. The implementation
  is independent and has no nrfx link dependency. This remains a GPIO/core/TWI
  slice, not a complete source-unit or silicon errata audit.
- [Microsoft UF2 specification](https://github.com/microsoft/uf2) and family list:
  nRF52840 family `0xADA52840`, 512-byte blocks with 256-byte payloads.

Focused native tests cover both GPIO banks, ownership and protected-pad rejection,
electrical encodings, MMIO failure rollback, and the facade lifecycle. Python
tests cover UF2 payloads, sparse segments, flash load addresses, malformed ELF,
protected partitions and mounted-drive planning. These establish software
behavior only. The separate physical observation establishes LED Blink on the
connected board; wire timing, current, other GPIO modes and clone-revision
qualification remain unverified.

The 28 October system-service follow-up adds the current CryptoCell register
supplement, all 32 PPI channels, clock/power event interfaces and optional
[debugger procedures](debug/README.md). See the [source/runtime guide](../../../../doc/reference/nrf52840/README.md)
and [acceptance record](../../../../doc/acceptance/nrf52840-system.md) for
software evidence and the separate pending hardware checks.
