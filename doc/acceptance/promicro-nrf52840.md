# Pro Micro nRF52840 initial verification and Blink observation

Recorded 27 October 2026 for REQ-0053 / PLN-0040. This record qualifies the
initial GPIO and UF2 software path and the scoped physical Blink observation
below. It does not establish full MCU support.

## Delivered scope

- GPIO/core metadata slice and two-bank native GPIO backend.
- GPIO-only board facade, active-high P0.15 LED, 64 MHz DWT execution policy.
- UF2/S140-v6 application layout at `0x26000..0xEC000`, with explicit
  factory-bootloader handoff startup and no UICR/SoftDevice activation.
- Repository and standalone Blink, generated nRF52840-family UF2 output,
  installed board resources, and explicit mounted-drive runner planning/upload.
- A CI board entry was added; hosted CI has not been executed from this session.

## Executed local checks

Host: macOS arm64; AppleClang 21.0.0.21000334. Target compiler:
`arm-none-eabi-g++` GNU 15.2.1. CMake/Ninja builds use Python 3.12.2 locally.

| Check | Observation |
| --- | --- |
| Complete native build | 49/49 CTest entries; merged artifact contains 304 cases, zero failures |
| Focused Nordic backend | Seven cases: electrical ordering/encodings, both banks, invalid/protected/duplicate pins, release, MMIO failures and facade lifetime |
| UF2 format | Six cases: headers/family, payload reconstruction, sparse loads, RAM VMA/flash LMA, malformed input and partition bounds |
| Runner | Temporary mounted-directory fixture validates Board-ID and exact ELF/UF2 correspondence; dry run creates no upload |
| Host headers | All 115 generated isolated-header translation units compile |
| Cortex-M4 probes | All 115 isolated headers and architecture probes compile; DMB/DSB/ISB instruction checks pass |
| Repository firmware/examples | Both Blink targets link and generate UF2 |
| Standalone source consumer | `example/blink` builds through the public helper |
| Relocated package | Installed package moved to a path with spaces; copied installed starter links, generates UF2 and runs installed runner dry-run |
| Existing target regression | Arduino UNO R4 Minima firmware still links |
| Trace validation | Focused REQ-0053 validation reports no diagnostics after native evidence regeneration |

For repository `firmware.elf`, `arm-none-eabi-readelf -l` reports vector/text
flash load start `0x26000`, initialized RAM at VMA `0x20000000` with flash LMA
`0x26CB4`, and Thumb entry `0x26871`. `arm-none-eabi-size` reports text 3252,
data 4, BSS 60 bytes. These are one build's section sizes; they exclude the
existing bootloader/SoftDevice, reserved stack, peak stack and runtime overhead.

Reproduce native checks with `cmake --preset macos-arm64` and
`cmake --build --preset macos-arm64`. Reproduce firmware using
`cmake --preset promicro-nrf52840` and
`cmake --build --preset promicro-nrf52840 --target firmware`.
The [board guide](../../src/board/others/promicro_nrf52840/README.md) documents
the source baseline, standalone application and exact upload prerequisites.

## Physical Blink observation, 27 October 2026

After the software checks, the connected board mounted as `/Volumes/NICENANO`
with serial interface `/dev/cu.usbmodem101`. Its `INFO_UF2.TXT` reported:

```text
UF2 Bootloader 0.6.0
Model: nice!nano
Board-ID: nRF52840-nicenano
SoftDevice: S140 version 6.1.1
Date: Jun 19 2021
```

The mounted identity and runner dry-run passed. On the user's explicit request,
the application was uploaded using `tool/hardware.py flash` with the repository
ELF and `--mount /Volumes/NICENANO`. The first sandboxed attempt was denied;
the authorized retry completed with exit code zero. The UF2 drive and bootloader
serial interface subsequently disappeared, consistent with application handoff.
The user then explicitly confirmed that the onboard LED was blinking.

| Flashed input | Identity |
| --- | --- |
| ELF | `output/build/promicro-nrf52840/src/board/firmware.elf` |
| ELF SHA-256 | `86b91ceadb0dbdf82fc01b8d7207284ca003461362e046932c92d1c0a22ea466` |
| UF2 SHA-256 | `ff59946e3cce956409e6e7f81c97e65f543d7330581137fdf1230b591c8c7284` |
| Selected-input identity | `f07761e1a95e9d8c530ec2f775bb406d398d2ebad58bb30d0f49bdf9c818d58a` |
| Profile / startup | `others/promicro_nrf52840`, `uf2-s140-v6`, supplied |

This is a user-observed LED Blink result following an executed UF2 upload, not
an instrumented on-device test suite or independent waveform measurement.
The application requests a toggle every 500 ms; the actual interval was not
measured. Successful execution exercises the supplied startup, board facade,
GPIO output and delay path on this one connected board.

## Remaining physical scope

The user's V1940-marked clone's exact PCB and FICR/silicon stepping remain
unidentified. Post-application double-reset recovery has not yet been exercised.
The bootloader and SoftDevice identities above were read from the mounted drive;
the complete reserved-flash contents were not independently read back.
The supplied application assumes the bootloader performed Nordic silicon/errata
initialization and disabled SoftDevice before handoff. Raw reset, a complete
MCU description/errata audit, remaining peripherals, radio stacks, USB application
classes, low-power behavior, electrical limits and timing are not qualified here.
