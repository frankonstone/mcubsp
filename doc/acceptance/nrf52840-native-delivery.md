# nRF52840 native peripheral delivery, 2 November 2026

PLN-0044–PLN-0053 now have native implementations and software verification for
the remaining analog, memory/protection, audio, USB, crypto, radio and NFC blocks.
All ten plan items retain `in_progress`: their physical exits have not run.
The user has a Pro Micro nRF52840, currently unavailable, and no confirmed debug
probe, peripheral peers or measurement equipment. No image was uploaded, no
physical device was reset, and no flash/UICR/lock/recovery operation was executed.
Earlier physical records retain their own image hashes and scope.

The [per-instance/mode matrix](../reference/nrf52840/implementation-matrix.json)
covers all 85 product instantiation rows plus 12 selected Arm core blocks.
It separates source review, typed metadata, working APIs, representative native
cases, cross-builds, linked applications and physical acceptance. The
[software evidence record](evidence/nrf52840-native/2026-11-02-software.json)
contains actual producer results, image identities and storage costs. Passing a
model or compiler probe does not establish physical timing or silicon behavior.

## APIs and contracts

All headers below are under `mcubsp/backend/nrf52840/`. Use one `system_context`,
GPIO/resource owner, clock controller and shared DMA pool per MCU. Controllers,
operations, buffers and callback storage must outlive failure recovery. A timeout
reports missing progress; it never authorizes returning RAM still reachable by
hardware. `detail/peripheral_session.hpp` shares bookkeeping only; each concrete
controller implements its own stop/completion proof. Pool capacity is explicit;
the examples use 24 slots to allow concurrent endpoint/audio directions.

| Plan | Interfaces delivered | Important boundary |
| --- | --- | --- |
| PLN-0044 | Shared peripheral sessions, fixed DMA queues/audio storage, routed RADIO/CCM/AAR ownership, CryptoCell DIN/DOUT constraints | PPI references prevent teardown and returning DMA storage. Cached native task endpoints become unroutable when their final RAM lease is returned. |
| PLN-0045 | Existing UART/UARTE, SPI/SPIM/SPIS, TWI/TWIM/TWIS and compatible facades | The [serial delivery](nrf52840-serial.md) remains authoritative for its modes and limits; SPIM3 RAM-slave isolation remains mandatory. |
| PLN-0046 | `pwm.hpp`, `saadc.hpp`, `comparator.hpp`, `io_analog.hpp`; existing QDEC | PWM0–3 expose four outputs and native sequence/decoder modes. SAADC uses signed samples and explicit calibrated scan/stream buffers. COMP/LPCOMP share ownership. |
| PLN-0047 | `qspi.hpp`, `factory.hpp`, `acl.hpp`, `mwu.hpp`, `provisioning.hpp`, extended `nvmc.hpp`, debugger `recovery.tcl` | QSPI needs attached-device geometry and explicit writable/protected ranges. NVMC/UICR/ACL mutations and CTRL-AP erasure are explicit operations. |
| PLN-0048 | `i2s.hpp`, `pdm.hpp` | Finite/bounded queued audio uses caller payload and tail storage. Service faster than one block; missing handoffs stop with overrun, without invented completed counts. |
| PLN-0049 | `usbd.hpp`, `usb_device.hpp` | All endpoint types/directions use one DMA arbiter. CDC ACM and HID keyboard enumerate through bounded EP0 staging. Bootloader handoff is explicit. |
| PLN-0050 | `ecb.hpp`, `ccm.hpp`, `aar.hpp`, `cryptocell.hpp`; existing standalone RNG | All CC_* subblocks are driven by one owner. Known-answer models and hardware sequencing are distinct evidence. |
| PLN-0051 | `radio.hpp` | Seven PHY encodings, finite TX/RX, CRC/layout/address controls, RSSI/CCA/ED, timing/event and explicit stack exclusion. |
| PLN-0052 | `nfct.hpp` | Native Type A frames, field sensing/wake and read-only Type 2/NDEF. Existing NFC UICR role and an attached antenna are required. |
| PLN-0053 | Mode matrix, actual body/header probes, legal concurrent compositions, source/relocated-package examples and measured budgets | Complete board qualification remains pending. No protocol-stack, RF certification, entropy-quality or exhaustive errata certification is inferred. |

The board facade now enables compatible PWM and ADC selections in addition to
its serial/SPI/I²C support. Default PWM0 uses the P0.15 LED; ADC uses AIN0/P0.02.
A common PWM duty update stops at a period boundary before changing private
samples and restarting; it introduces a gap. Use native PWM sequences when that
behavior is inappropriate. Common ADC scans use private signed DMA storage and
clamp negative offsets to zero at the unsigned facade boundary. Native SAADC
retains differential/signed results and all reference/gain/acquisition choices.

PWM sequence END alone does not release RAM: STOPPED is required. SAADC queue
handoffs use END, with STOPPED required for final/cancelled storage. I2S needs both
enabled direction-pointer events before publishing a previous duplex block;
PDM needs the next STARTED and previous END. Tail buffers absorb the final
hardware handoff and are never counted as payload. Queue the next payload before
the preceding boundary selects the tail. Sampling/bit-clock accuracy, startup
transients and analog errors remain physical observations.

QSPI READY means the controller has completed its command/DMA phase. Program and
erase additionally poll the declared device status/WIP command. Cancellation
drains the operation because QSPI has no STOP task. A power-down timeout retains
an ambiguous-state flag and excludes commands/XIP until status recovery. XIP
borrow tokens exclude indirect commands and teardown; executable mappings require
an explicit linker/ISR relocation contract. `copy_xip_words()` provides the
barrier-separated data access path for anomaly 216. No attached flash, quad-enable
state, capacity, spare sector or executable layout is inferred from the board.

USBD holds IN RAM until both DMA END and the host ACK; ISO has no host ACK and
reports that distinction. EP0 SETUP abort never reuses staging RAM before the old
DMA quiesces. Hardware handles SET_ADDRESS, including its status stage. OUT
backpressure, bus reset, VBUS removal, stall/toggle, SOF deadlines, suspend/resume
and remote wake have explicit state. Low-power polling only reads permitted
status/events; foreground wake reacquires HFXO and completes the erratum-171
handshake. Attached USB continues to block System OFF. Remote wake needs host
enablement and at least 5 ms suspended time. Example VID/PID values are experimental
bench identifiers; applications supply their assigned values. CDC line coding
and DTR/RTS never implicitly reset or select a bootloader. Handoff instead checks
explicit ABI consent, vector bounds, application quiescence and other clock/work
clients before writing GPREGRET and requesting reset.

ECB uses the native 48-byte key/plaintext/ciphertext layout. BLE CCM has explicit
39-bit packet counters, direction/IV layout, default/extended packets and MIC
status. Reject plaintext when `mic_valid` is false. CCM/AAR share hardware and
PPI ownership. CryptoCell drives AES ECB/CBC/CTR/CMAC/CBC-MAC, SHA-1/224/256,
ChaCha20, both DMA engines, interrupt masks/status, PKA arithmetic/modular
instructions, TRNG health/retry handling and SRAM ports. Hardware AES/hash/ChaCha
primitives also supply fixed-storage CCM/CCM*, HMAC and ChaCha20-Poly1305
compositions. PKA callers provide N/Np and operand layouts; RSA/ECC/SRP protocol
wrappers and a DRBG are not supplied. No Nordic library or adapter dependency was
added. DIN/DOUT are CryptoCell DMA, with a physical 65535-byte limit; they are not
a generic EasyDMA engine. Secret buffers are explicit caller/controller storage;
keep them alive through `recover()` after a failed DMA operation. Shutdown wipes
session registers and engine SRAM without programming retained KDR/LCS. No
security-certification or cryptographic side-channel qualification is claimed.

RADIO has explicit direct-versus-stack ownership; it never starts S140. TX/RX
always use DISABLE between packets/directions, and coded/802.15.4 paths use PHYEND
for final activity. Coded RX requires servicing at least every 500 µs; anomaly
172's RSSI/counter processing is foreground state, not a fictitious radio register.
Precise inter-frame scheduling remains a caller timer/PPI composition, with
`prepare_coded_turnaround()` supplying the source-defined correction before
DISABLED. Peer examples default to BLE 1 Mbit at 2402 MHz. Select the same PHY and
packet configuration in both independently running applications for another mode.
No Bluetooth, Thread, Zigbee or ANT protocol stack is implied.

NFCT never rewrites UICR or claims NFC pads as ordinary GPIO. After FIELDDETECTED,
wait at least 1 ms through `activate()` before starting communication. Frame END,
CRC/parity and field-loss handling determine buffer release. RX counts include
CRC bytes; the Type 2 responder accepts validated CRC-stripped commands. READ
wraps through the tag image, writes return a four-bit NAK, and HALT enters sleep.
Antenna matching, NFC-reader interoperability and field wake remain unmeasured.

## Reproduce the software checks

```sh
cmake --preset macos-arm64
cmake --build --preset macos-arm64 -j8
cmake --build --preset macos-arm64 --target target-compile-probes -j8
cmake --preset promicro-nrf52840
cmake --build --preset promicro-nrf52840 --target examples -j8
```

The native build runs the CTest checks and republishes its complete/sharded
mcutest evidence only after success. It includes analog, memory, audio, crypto,
USB, radio, NFC, common facade and legal concurrent-clock compositions, plus
negative PWM/ADC/audio bindings. Crypto tests use independently implemented
byte-oriented AES/SHA/ChaCha models with NIST/RFC expected outputs; AAR and BLE
CCM tests currently exercise layout/ownership/handshake/MIC reporting, not a
physical or independently modeled RPA/BLE-vector calculation.

The new standalone applications are `analog-pwm`, `audio-capture`, `crypto-vectors`,
`memory-watch`, `qspi-memory`, `radio-tx`, `radio-rx`, `nfc-tag`, `usb-cdc` and
`usb-hid`. Each has its own CMake project using either `MCUBSP_SOURCE_DIR` or
`find_package(mcubsp)`. All ten were built from source and from a freshly installed,
relocated package. The actual template bodies are also compiled against the
installed headers. ELF/UF2/input identities and text/data/BSS costs are in the
linked evidence record. Those costs include the selected startup, context/pool
and example payload storage; BSS/data are not peak stack measurements.

## Measured example budgets

Cortex-M4F GNU 15.2, `-Os`, supplied startup and the preserved S140-v6 boot
layout. Flash is text + data; static RAM is data + BSS. Caller/IRQ peak stack
usage is additional and has not been measured.

| Example | Flash bytes | Static RAM bytes |
| --- | ---: | ---: |
| `analog-pwm` | 21184 | 6380 |
| `audio-capture` | 14664 | 15660 |
| `usb-cdc` | 13066 | 7192 |
| `usb-hid` | 13502 | 7192 |
| `crypto-vectors` | 11708 | 5816 |
| `radio-tx` | 9208 | 5848 |
| `radio-rx` | 9108 | 5856 |
| `nfc-tag` | 9424 | 5656 |
| `qspi-memory` | 10448 | 5708 |
| `memory-watch` | 3952 | 5328 |

Trace validation reports zero errors and zero focused requirement/plan
diagnostics. The ten new firmware examples retain missing-coverage warnings:
no line/branch coverage producer was run for those applications. Project-wide
validation currently has 74 warnings, including older unrelated diagnostics.
Native results, cross/link checks and coverage are separate evidence classes.

## Physical procedures still pending

- PWM outputs, decoder/loop timing and glitches; ADC calibrated single-ended and
  differential sources, limits and comparator/LPCOMP thresholds/wake; QDEC peer.
- QSPI read/program/erase/status/DPM/XIP with a declared external NOR. The supplied
  `qspi-memory` example modifies only an explicitly reserved last 4 KiB sector
  of its declared external 1 MiB geometry. Confirm or adapt that contract first.
- Internal NVMC partial/full erase, ACL permissions, UICR/reset-role/regulator and
  protection transitions, plus bootloader restoration using an available debug
  probe. `recovery.tcl` is an external DAP tool and requires its destructive phrase;
  native Tcl models never touch a probe.
- I2S RX/TX/duplex and PDM microphone capture, external clocks, measured sample
  rates, tails, transients and intentional service overruns.
- Host CDC/HID enumeration/data/control errors, every required endpoint/ISO path,
  suspend current, remote wake, disconnect/reset during DMA and explicit
  bootloader handoff/recovery. The HID example initially reports no keys pressed.
- Independent radio peers for all seven PHYs, CRC/error counts and recorded RF
  conditions; optional external-FEM behavior needs its own attached configuration.
- NFC tuned antenna/reader transactions and field wake; on-device deterministic
  ECB/CCM/AAR/CryptoCell vectors, TRNG health/lifecycle and debug recovery.

These are acceptance work, with missing equipment recorded; their requirement
scope and plan exits have not been reduced.

## Pinned source review

The base register/field/instance facts remain Nordic PS 4413_417 v1.11 and the
hashed nrfx v3.2.0 files in the [source manifest](../reference/nrf52840/source-manifest.json).
Relevant PS sections are 6.4–6.6, 6.9, 6.11–6.17, 6.19–6.20, 6.23 and 6.35.
Native code comments name each applied workaround. The additional review covers
PWM 183; SAADC 212/237/241/252/266/276; COMP 275; QSPI 122/208/215/216/244;
I2S 55/170/194/196; PDM 128 revision gating; USBD 166/171/187/199; RADIO
153/172/191/204/228/236/243/245/248/254/279; NFCT 190/218; and NVMC 242.
The published [RADIO 172 addendum](https://docs-be.nordicsemi.com/bundle/nRF52840_PAN172/raw/resource/enus/nRF52_PAN_172_add_v1.0.pdf)
is document 4413_505 v1.0, January 2019; its Nordic-authored content was reviewed
through a [document mirror](https://device.report/m/fd59b0234b72891c57a6a567f756b5b6e6f114b01ee3fde200e5821c371156c9.pdf)
when the original endpoint returned an unavailable-client message.
Algorithm flows/vectors additionally use [RFC 3610](https://www.rfc-editor.org/rfc/rfc3610),
[RFC 8439](https://www.rfc-editor.org/rfc/rfc8439), RFC 2104/4231,
NIST FIPS 197/180-4 and SP 800-38A/38B. USB class contracts follow USB 2.0 chapter 9,
CDC PSTN 1.2 ACM and [HID 1.11](https://www.usb.org/sites/default/files/hid1_11.pdf).
This review does not convert representative software checks into exhaustive
silicon/physical qualification.
