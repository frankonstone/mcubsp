# nRF52840 description and system-service checkpoint

Updated 28 October 2026. PLN-0042 and PLN-0043 remain **in progress**. The delivered
software is a substantial foundation, not completion of either full plan.
Requirements and exit criteria retain their original meaning.

## Delivered scope

- Reproducible, content-identified Nordic MDK register facts: 43 families, 74 SVD
  instances, 2,142 expanded register entries and 5,949 field entries. Safe task,
  alias, W1C/event and side-effecting read semantics have distinct types.
- PS v1.11 tables: 53 families, 1,563 detailed registers and 4,476 fields,
  including 11 CryptoCell blocks with 199 registers and 345 fields. All
  1,567 product register-overview entries have an address/reserved/CTRL-AP
  classification; 192 additional CryptoCell entries are absent from the MDK.
- All 85 product instantiation rows, three package bonding/electrical tables,
  checked literal enum encodings and CPU RAM alias/EasyDMA span constraints.
- Selected Arm core descriptions: 12 components, 382 register views and 729
  fields, with keyed writes, byte priorities, explicit read side effects and
  debug comparator restrictions. These do not provide automatic debug ownership.
- Explicit native clock/power, GPIOTE/port sensing, all 32 PPI channels and six
  groups/forks, EGU/SWI, TIMER0–4, RTC0–2 and watchdog APIs. Fixed channels retain
  their immutable endpoint owners, including exclusive AAR/CCM personalities.
- External LF input selection, HFXO debounce, POWER/CLOCK/PORT event routing,
  shared-vector interrupt cleanup and calibration-timer expiry/recovery. RTC
  and watchdog clients accept the same explicit LF configuration.
- Endpoint generation/lifetime checks, failed-cleanup ownership retention,
  shared clock references, borrowed-core reservations and bounded ISR callbacks.
- Cortex-M vector/fault, NVIC, SysTick, FPU, MPU and initial DWT/ITM controls.
  Supplied startup has weak core handlers, a peripheral-dispatch trampoline and
  a RAM fault-status record. External startup retains its own vector ownership.
- Optional debugger inspection of FPB/DWT, faults and FICR/ROM identity, plus
  explicit OpenOCD ITM/SWO or ETM-M4/parallel-TPIU configuration and cleanup.
- A standalone system application whose initial timer/PPI/GPIOTE phase changes
  to RTC ISR/alarm-dependent pulses and watchdog feeding. A separate timeout
  variant deliberately resets; both use the existing UF2 partition.

The [runtime/source guide](../reference/nrf52840/README.md) records sources,
contracts, workarounds, interface limits and unresolved product-description gaps.
The board facade continues to advertise GPIO/I2C; native services are explicit.

## Verification actually run

The 27 October baseline on macOS arm64 with AppleClang 21 passed 56/56 CTest entries and its merged
native report contained 353 cases with zero failures. This includes 31 new
system-service cases, three Nordic metadata cases, three new negative instance
compilations and existing GPIO/I2C regressions. Its 169 public headers compiled
independently on host, Cortex-M4, Cortex-M7 and ESP32-P4; architecture
barrier/metadata probes pass. The Arm toolchain is GNU 15.2.1. The pinned import reproduces byte-for-byte with `--check`.

Repository Blink, TMP102 and system images link and emit UF2. The system example
also links from source and from a relocated installed package in paths with
spaces, has exactly one Reset_Handler, retains the Nordic license, and passes
installed-runner planning against a fake UF2 mount. These runner checks do not
access hardware. That system image had 7,268 bytes text, 16 bytes data and
808 bytes BSS; these are link-map sizes, not peak stack or power measurements.

Software logs are local under `/private/tmp/mcubsp-nrf-system-*`; source/package
checks are under the task-specific temporary consumer directory. Current native
trace evidence is produced by `host-tests`. Coverage was not regenerated and
hosted CI was not executed by this checkpoint.

Follow-up verification on 28 October: 59/59 CTest entries pass, with a fresh
merged report of 369 cases and zero failures. All 182 public headers compile on
host/Cortex-M4/Cortex-M7/ESP32-P4; the fixed-route and product metadata probes
pass on all target compilers. Both MDK and product supplement imports reproduce
with `--check`. The product audit reports 1,366 matching MDK absolute addresses,
192 supplemental registers, four reserved addresses and five debug-port entries.
This is not a complete field/mode/core/pin/errata audit.

Six Tcl mock tests cover the debugger scripts, including failed cleanup and
bounded ETM handshakes. OpenOCD 0.12.0 parses the real configuration and executes
a mocked parallel session under Jim Tcl. Those checks do not access a probe.
New source/relocated-package system builds, installation of debugger resources,
UF2 generation and fake-mount runner planning pass. Current image hashes and
software evidence are recorded in [the follow-up record](evidence/nrf52840-system/2026-10-28-software.json).
The changed system image contains 8,032 bytes text, 16 bytes data and 1,144
bytes BSS. It has not been uploaded or physically verified. These are linked
section sizes, not stack/current measurements.

Further audit/software checkpoint on 28 October: the full product parser
reconciles 4,122 MDK field matches, 346 supplementary fields and eight reviewed
access differences with no unexplained field-shape/reset mismatch. It handles
all 32 diagram IDs, including lower-case IDs that the prior CryptoCell parser
missed. The corrected CryptoCell count is 345 fields. The CLOCK.LFRCMODE.STATUS
prose establishes read-only access; both generated views now reject writes to
it. Arm overlays correct the CMSIS TPI DEVTYPE shift and floating-point debug
selector width, separate read-only/status fields and preserve required keys.
The package GPIO restrictions are tested independently of unverified board
connector exposure. Historical software and physical records above remain
unchanged. Current verification and image hashes are in
[the audit record](evidence/nrf52840-system/2026-10-28-audit-software.json).
All 61 CTest entries and 385 merged native cases pass. All 239 public headers
compile on host, Cortex-M4, Cortex-M7 and ESP32-P4. MDK/product/core imports
reproduce; repository, source and relocated-package builds pass, including an
installed Cortex-M4 metadata probe and both system variants. Focused mcutrace
validation reports no diagnostics; it does not execute physical tests.

The new regular image removes the autonomous LED route after four seconds.
Its repeating double pulses require RTC alarm time progress and ISR delivery;
TIMER2 supplies independent timeout/wake interrupts. A watchdog is fed only
after complete pulse cycles. The timeout variant stops feeding and attempts
reset, using triple pulses after a captured DOG reason. A reason consumed by
the bootloader remains inconclusive. Neither image has a physical result in
this software record. See the [test procedure](../../example/system/README.md).

## Physical observation

The user put the board into UF2 mode. The mounted drive reported UF2 Bootloader
0.6.0, Board-ID `nRF52840-nicenano` and S140 6.1.1. The verified system UF2 upload
completed with exit code zero. The user then confirmed **regular LED blinking**.

[The observation record](evidence/nrf52840-system/2026-10-27-upload.json) binds
this result to ELF SHA-256
`4ff2cd94126d9b191160bf775e9ed00b8dd026207fde3015449a0afe573d0119`
and UF2 SHA-256
`f621c6e70c8d143874af74b657b67d9ebe0730e33cae20be71ce819c5e77ee29`,
with the selected-input identity and sidecar hash recorded at upload. The
27 October metadata-semantics correction rebuilt the same byte-identical ELF.
The 28 October context/identity changes produce a different image. Historical upload input identities are preserved rather than rewritten.

The application changes the LED through TIMER2 → PPI0 → GPIOTE0, without
foreground GPIO toggles. The observation therefore supports that hardware path
for this image. The LED would continue blinking if the foreground RTC alarm
loop failed; it does **not** independently establish RTC interrupt, overflow or
wakeup success. There is no calibrated frequency/duty, power/current, watchdog
reset, MPU/FPU fault, System OFF retention or debug/trace observation. The user
has UF2/USB access only; no SWD probe was available. Exact chip/PCB revision and
post-application bootloader recovery remain unverified.

New upload on 28 October: the user reported UF2 mode and the compatible
NICENANO drive mounted. The regular RTC/watchdog-feeding test matched all hashes
in the audit software record, passed runner planning and uploaded successfully
(exit code zero). The UF2 drive subsequently disconnected. The
[new upload record](evidence/nrf52840-system/2026-10-28-audit-upload.json) binds
this action to ELF `39d62a5d7816ec6336c68d026008da9263aaa5396dc4fde49281f12ed213d12d`.
The user subsequently confirmed **two short flashes, pause, repeating**, after
being asked to watch for about 20 seconds. That matches the RTC1 alarm/ISR-driven
post-startup pattern and supports progress through the watchdog start/feed path
while native System ON idle is exercised. It does not measure timing, sleep
residency or current; TIMER2 independently supplies wake/timeout interrupts.
RTC overflow endurance and broader system acceptance remain open. The earlier
physical LED observation remains tied to its old image.

The user then double-reset the running regular audit image; the compatible UF2
drive mounted and was read successfully, observing bootloader re-entry for that
image. The separate watchdog timeout variant matched its audit hashes, passed
runner planning and uploaded with exit code zero. Its
[upload record](evidence/nrf52840-system/2026-10-28-watchdog-upload.json) binds
the action to ELF `e992daa68616e6fe20751e47e768d25dfaf0d21ac165147b8e83e8043ca34111`.
The user confirmed **restarts; later groups have three flashes**, after being
asked to watch for about 30 seconds. This observes the deliberate no-feed reset
path and the firmware's captured RESETREAS.DOG indicator after restart. Timeout
accuracy was not independently measured; other reload channels/configurations
remain unqualified. The user subsequently double-reset that image and the
compatible UF2 drive mounted, observing recovery after the reset test. The
original regular image was verified against its audit hashes and restored
successfully (exit code zero); the drive disconnected. Its restored application
behavior was not separately observed. Restoration hashes and recovery are
recorded in the watchdog upload record.

## Remaining plan exits

PLN-0042 still requires complete instance/mode/errata sequencing review,
revision-applicable errata identity, board pin mapping and actual silicon
identity readback. Current peripheral/core tables and package bonding are
delivered; `complete_product_description`
remains false. The old MDK facts and current-product supplement retain separate
provenance.

PLN-0043 still requires complete source/errata review and the independent
physical timing, capture/counter, broader RTC/watchdog coverage, core/fault, wake/retention,
debug/trace and current procedures. The fixed PPI, clock/power event and optional
debugger-script increments are implemented and software-tested. Exact board pin
access and stepping remain unverified. Missing implementation review and
unexecuted measurements retain their own pending state; plan status is not
verification evidence.
