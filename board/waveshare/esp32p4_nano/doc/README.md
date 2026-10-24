# ESP32-P4-NANO integration

PLN-0019 uses two separately linked images on HP CPU0:

- a custom ESP-IDF v6.1 second-stage bootloader, containing only hardware/image
  initialization and a fixed policy that loads the factory partition; and
- a factory application containing the `mcubsp` GPIO, UART, I2C, I2S/audio,
  SDMMC/SDIO, SPI and Ethernet backends, registered `mcutest` cases, UART
  result adapter and MCUCOV runtime.

The immutable ESP32-P4 ROM remains the first-stage loader. The custom
second-stage loader supplies flash/cache setup, reads the partition table and
loads the application. It contains no functional driver, test or coverage
code. The application uses ordinary ESP-IDF image, libc and constructor
initialization, then intercepts startup immediately before ESP-IDF would create
the main task and start FreeRTOS. The scheduler is therefore never started.
ESP-IDF does initialize its heap and other system services before that
boundary; the board-test path itself performs no allocation.

The connected board identifies as ESP32-P4 revision v1.0 with 32 MiB PSRAM.
That revision is intentionally outside the accepted production-v3.x MCU
description. Its smoke result is development evidence only; it does not
qualify the v3.x release target.

## Flash layout

The build uses a 16 MiB flash image header and the default single-factory
partition table:

| Offset | Contents | Size/limit |
| --- | --- | --- |
| `0x0000` | reserved flash area before the bootloader | 8 KiB |
| `0x2000` | custom second-stage bootloader | must end before `0x8000` |
| `0x8000` | partition table | 3 KiB generated image |
| `0x9000` | NVS partition | 24 KiB |
| `0xf000` | PHY-init partition | 4 KiB |
| `0x10000` | factory application | 1 MiB partition |

The immutable first-stage loader is mask ROM inside the chip, not part of this
flash map. Thus the boot chain is ROM -> custom second-stage loader -> factory
application. There is no third bootloader stage.

## Build and run

Activate ESP-IDF v6.1, then build both images:

```sh
idf.py -C board/waveshare/esp32p4_nano/test/firmware set-target esp32p4
idf.py -C board/waveshare/esp32p4_nano/test/firmware build
```

The repeatable runner builds, flashes the bootloader, partition table and
application, captures the UART frame, checks the exact loader handoff and
test result, and creates JSON and HTML coverage reports under
`baremetal_smoke/build/artifact`:

```sh
tool/run-esp32p4-nano-smoke.sh /dev/cu.usbmodem5A7A0368561
```

The runner requires `expect`, `jq` and `xxd`. It normalizes each generated
MCUCOV translation-unit manifest into one manifest per source module because
the current MCUCOV host reader has a fixed JSON arena. The split does not alter
module or probe identities; MCUCOV merges the directory when producing the
report.

The application emits `MCUBSP_CONTEXT`, the normal `mcutest` result and
summary, `MCUBSP_COVERAGE_HEX`, and `MCUBSP_RESULT`. Before starting the test
runner it configures UART0 through the `mcubsp` backend and transmits the
`MCUBSP_UART` marker through the board's CH343 USB-to-UART bridge. The registered
test verifies initialization, clock/divider readback, transfer start and
TX-done completion. This directly proves target TX; physical RX and injected
line errors still require a loopback or fixture. Host tests cover RX, error,
cancellation and rejected/boundary paths.

The application also initializes I2C0 at 100 kHz on GPIO8/GPIO7, verifies the
source divider, timing registers, production/development clock binding and
GPIO-matrix routes, then reads the on-board ES8311 codec identity registers at
address `0x18` using register-select writes, repeated STARTs, reads and explicit
STOPs. The connected chip is
revision v1.0, whose I2C0 APB-gate and reset bits differ from production v3.x;
those two masks are therefore an explicit development-board binding and do not
change the accepted v3.x MCU description. Host-native tests cover command
sequencing, read/write, repeated START, explicit STOP, FIFO limits, NACK phase
classification, arbitration loss, timeout, cancellation and bounded recovery.
The on-board codec provides physical transaction evidence without an external
fixture; independent logic-analyzer timing/protocol capture remains deferred.

The audio case configures the ES8311 for 48 kHz, stereo 16-bit Philips I2S,
routes I2S0 over GPIO13/GPIO12/GPIO10/GPIO9/GPIO11, and uses AHB-PDMA channel
0 with explicit cache ownership. Its cache-line-aligned native run captures
4,032 bytes from the on-board analog microphone, then sends a 1,008 ms, 1 kHz
triangle wave at a peak sample magnitude of 6,000 to the codec DAC. The test
uses 0 dB codec DAC gain, enables GPIO53 while the codec remains muted, waits
for the amplifier to settle, unmutes for 48 completed 4,032-byte DMA blocks,
then mutes before disabling the amplifier. This proves codec configuration,
live microphone data, DMA completion and the commanded digital playback path.
It intentionally produces a human-audible tone, but the automated result is
not an independent acoustic oracle: confirming speaker output level, frequency
and distortion still requires a directly connected microphone or audio
analyzer if those properties are later claimed.

The microSD case powers the dedicated pads from ESP32-P4 LDO channel 4, pulses
the NANO's active-low GPIO45 card-power control, and initializes native SDMMC
slot 0 on GPIO39-GPIO44. It identifies the inserted 30,007 MiB SDHC card in
4-bit mode at 20 MHz, reads sector 0 through the internal DMA controller, then
saves, overwrites, reads back and restores the final sector. A second read-back
verifies the restoration before the test passes, so the procedure does not
leave its test pattern on the card. The current driver intentionally exposes
card initialization and single-sector block I/O only; filesystem mounting and
multi-block transfers remain outside this increment.

The Wi-Fi coprocessor case drives GPIO54 low and then high to reset and enable
the on-board ESP32-C5-MINI-1U, waits the ESP-Hosted default 1.5 s startup
interval, and initializes SDMMC slot 1 through the GPIO matrix on GPIO14-GPIO19.
It issues CMD5, assigns/selects RCA `0x0001`, reads CCCR revision `0x32`, walks
the CIS at `0x01000`, and requires the Espressif manufacturer/card identity
`0x0092`/`0x6666`. The connected C5 firmware advertises two IO functions. The
test enables function 1, observes its ready bit, programs 512-byte function-0
and function-1 block sizes, and switches the card and host to four-bit mode.
This is physical SDIO enumeration and identity evidence. It does not yet run
ESP-Hosted packet queues, exchange control/data packets, initialize the remote
Wi-Fi API, scan, associate, or obtain network connectivity. Slot 0 and slot 1
share the ESP32-P4 SDMMC controller and are deliberately exercised
sequentially under the backend's single-owner rule.

The application also configures GPSPI2 at 1 MHz on the free expansion-header
GPIO20-GPIO23 pins and executes one four-frame, two-segment CPU-buffer command.
It verifies the clock gates, native divider, GPIO-matrix routes, `TRANS_DONE`
completion and cleared `USR` busy bit. Because the NANO's microSD card and
Wi-Fi coprocessor are connected over SDIO rather than SPI, this is controller
execution evidence only: no SPI slave response, signal timing or chip-select
waveform is claimed without an external loopback/device or logic analyzer.

The Ethernet case initializes the ESP32-P4 EMAC, routes the board's RMII and
SMI signals, supplies the external 50 MHz reference-clock mode, resets the
IP101GRI through GPIO51 and addresses it as PHY 1. On the connected board the
physical MDIO reads return ID1 `0x0243`, ID2 `0x0c54`, OUI `0x0090c3`, model
`0x05` and BMSR `0x7849`, which validates MAC/SMI setup and the on-board PHY
identity. The reusable backend provides fixed, caller-owned enhanced TX/RX
descriptor rings and an allocation-free MCUIP adapter contract with explicit
64-byte alignment and cache ownership. During the recorded run no Ethernet
cable/peer was connected, so the PHY reported link down. Raw-frame exchange,
electrical RMII traffic and the physical MCUIP protocol smoke path are not yet
claimed; connect an ordinary Ethernet peer and rerun to complete those checks.

The board test translation units are instrumented with MCUCOV's `FULL` profile.
The runner also writes a `mcutest-results` JSON artifact and a target-filtered
MCUCOV JSON artifact for mcutrace. The latter contains only ESP32-P4 board-test
and backend modules so it does not duplicate shared-header coverage already
imported from the host suite. The board-native happy paths do not close every
branch or MC/DC outcome.

## Recovery

The runner replaces flash contents at `0x2000`, `0x8000` and `0x10000`.
Before using it on another board, preserve the full flash or obtain the vendor
recovery image. The original second-stage bootloader from this board was saved
outside the repository as
`/private/tmp/esp32p4-nano-original-bootloader-20260920.bin`; its SHA-256 is
`3f8c8b47a0bfd3a8d83e1018d3a88db53f05b04be4dc7fc6c11661d76f62fdce`.
That file restores only the bootloader region, not the original partition
table or application.
