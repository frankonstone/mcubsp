# mcuhw implementation plan

Revised baseline, 26 September 2026. Implements [requirements.md](requirements.md). The current sequence is: **preserve the completed three-MCU description and RA4M1 driver foundation; accept each bare-metal target from its scoped native, build and directly connected physical evidence; follow PLN-0026 with repository adoption and scalable build workflows; integrate MCUOS last**. Status annotations below track execution; a completed plan is not itself requirement evidence. Existing evidence is retained. Read in the phase order below, not numeric ID order.

## Delivery phases

| Phase | Work | Exit |
| --- | --- | --- |
| A: description foundation | Sources and coverage inventories for RA4M1, STM32H753 and ESP32-P4; core metadata/MMIO contracts; native and cross-compile probes | Shared model can express real differences without vendor register-layout assumptions |
| B: three complete descriptions | Reviewed block-sized increments across all three, followed by remaining documented blocks | All three declared baselines pass the common description and coverage gate, before any backend/driver implementation |
| C: completed bare-metal foundation | Preserve completed RA4M1 startup, GPIO, execution, transfer, timer and basic-PWM work | Stable driver-facing baseline; existing evidence remains valid |
| D: bare-metal target completion | Finish driver implementations and accept each target from scoped native, cross-build and directly connected physical procedures | Evidence identifies what was observed and does not imply unmeasured electrical or complete-board claims |
| D2: repository adoption and scale | Immediately after PLN-0026, deliver independent packaging, local board settings, selected-board builds, reproducible setup and full verification | Clean external applications build through one workflow; adding unrelated board/devices does not expand their build work |
| E: MCUOS | Optional adapter using `../mcuos`, tested natively and then on an executable MCUOS hardware target | RTOS portability demonstrated independently of earlier bare-metal releases |

Motor PWM and other extensions remain later milestones. MCUOS source, API readiness and hardware ports are not prerequisites for phases A–D or D2. Target work may be verified with host models, cross-builds, debug access, built-in board features, documented bridges/direct wiring, connected devices, native connectors with ordinary peers, or internal known-answer tests. Claims stay limited to the behavior those paths independently observe.

Phase D2 uses the existing board integrations and follows PLN-0026 without
waiting for unfinished peripheral acceptance in PLN-0019 or PLN-0023. It does
not reopen completed releases or imply new physical verification.

## Intended structure

```text
include/mcuhw/
  core/                 register/field types, units, concepts and metadata utilities
  arch/arm/cortex_m4/    architecture metadata; execution primitives added with drivers
  arch/arm/cortex_m7/
  arch/riscv/
  mcu/ra4m1/            declarative device, memory, clock, pin, IRQ, DMA and register data
  mcu/stm32h753/
  mcu/esp32p4/
  backend/              peripheral register sequences shared by compatible MCU variants
  driver/               functional interfaces and logic
  device/               MCU-independent attached-chip protocols
  board/                package selection, board identity and connector/pin aliases
  execution/            later portable execution contract and bare-metal policy
  io/                   application facade, board profiles, storage and ownership
board/<vendor>/<board>/  local board settings, startup/build bindings, tests and docs
device/<vendor>/<part>/  local device documentation, focused tests, examples and docs
src/                    non-template behavior where useful
adapters/mcuos/          phase E only
example/               metadata examples first; bare-metal firmware after the gate
test/                 description contracts, then driver models and hardware procedures
doc/                   manuals, provenance and coverage manifests, design decisions
cmake/                 native and architecture/toolchain configurations
```

Templates and constant-evaluated definitions live in headers; other behavior may use compiled translation units. Description headers must be usable together on a host without board startup, OS includes or a global target-selection macro. Do not create empty target trees as evidence of support.

Phase D2 retains the public `include/mcuhw/` entry points and places local
build settings and procedures beside each board. Shared MCU, backend and device
support remains reusable. Board selection reads the chosen `board.cmake`.

## Phase A: sources, contracts and description test infrastructure

## PLN-0001 Pin sources and coverage scope for all three MCUs
@status(done)
@link(REQ-0001, REQ-0005, REQ-0006, REQ-0033, REQ-0034, REQ-0039, REQ-0040)

Prerequisite: none. D01–D08 are recorded, including the revised ordering.

- Inventory RA4M1, STM32H753 and ESP32-P4 documented blocks, part/package variants and source revisions. Create separate description and driver-support columns and explicit source-gap states. Prioritize the same resource and initial-peripheral categories across all three.
- Review the now-local STM32H753 RM0433 Rev.8, DS12117 Rev.10 and ES0392 Rev.15. Their identity/readability checks and all six supplied PDF checksums are recorded in [the source manifest](doc/reference/source-manifest.json). Pin applicable RA4M1 corrections and the ESP32-P4 source/silicon baseline; resolve published updates and preliminary gaps before claiming definitions.
- Use the generated [Markdown manual index](doc/_generated/manual/README.md) for section lookup and text search; follow its exact PDF-page links to verify register tables, diagrams and ambiguous extraction. The conversion records source/output hashes and preserves all 9,192 page locations; it does not replace the detailed source audit.
- Record checksums, section/table provenance, supported package scope and known exclusions. Ask for important scope decisions if a source is unavailable or the selected variants materially change the work; never invent register data.
- Keep MCU family data separate from board choices. UNO is the initial physical driver target, but later STM32/ESP boards are not required to inspect or compile descriptions.

Exit: each MCU has a precise documented baseline and coverage denominator; known missing material has a resolution path. No MCUOS dependency or driver API implementation is introduced.

## PLN-0002 Establish description-only builds and verification
@status(done)
@link(REQ-0002, REQ-0003, REQ-0008, REQ-0038, REQ-0039, REQ-0040)

Prerequisite: PLN-0001 for target/source scope; compiler feature probes may begin earlier.

- Establish CMake native builds and metadata compile probes for Cortex-M4, Cortex-M7 and the selected ESP32-P4 RISC-V toolchain. Check the C++23 features actually required and record compiler/standard-library identities.
- Enforce the restricted runtime. Add independent header inclusion tests, compile-pass/fail tests and an instrumented host MMIO policy for core access primitives. Host tests must not touch target addresses.
- Add the shared parameterized description-test harness and a translation unit including all three MCU namespaces. Begin with real definitions as they are delivered, not placeholder target types.
- Register real source/build/evidence inputs in mcutrace only when they exist. Preserve its evidence policy. No RTOS checkout or RTOS-linked native test is required.

Exit: description contract checks can run natively and each architecture has a reproducible compile probe; positive/negative cases and test-fixture failures are observable.

## PLN-0003 Implement core register, field and metadata contracts
@status(done)
@link(REQ-0004, REQ-0007, REQ-0008, REQ-0009, REQ-0010, REQ-0011, REQ-0012, REQ-0013, REQ-0040)

Prerequisite: PLN-0002.

- Implement typed register/field metadata and legal low-level access operations, including access width/alignment, full-width masks, reserved bits, side effects and checked enum encodings. Prevent unsafe read-modify-write.
- Define peripheral identities, capabilities, package pins, routing, clock graphs, IRQ/event relations and DMA/memory constraints. Allow multiple clocks, interrupt sources and routes and preserve vendor-specific encodings.
- Add checked units/arithmetic and declarative constraint validation. Actual PWM/I2C/baud solver behavior and peripheral initialization remain driver-phase work.
- Trial each proposed abstraction against source-backed examples from all three MCUs before stabilizing it. Use host register traces only to validate the core access primitive; do not implement peripheral sequencing or transfers here.

Exit: each contract represents contrasting real hardware cases; invalid fields/accesses/routes fail clearly, and no fake uniform peripheral register model is required.

## Phase B: descriptions for all three, then a mandatory gate

The next description items may be interleaved by block category. For example, describe clocks for all three before proceeding to pins for all three. This is scheduling of work, not a requirement to delegate to multiple agents. Do not finish a RA4M1 driver while the other descriptions remain hypothetical.

## PLN-0004 Describe RA4M1 foundational resources and initial peripherals
@status(done)
@link(REQ-0005, REQ-0006, REQ-0007, REQ-0009, REQ-0010, REQ-0011, REQ-0012, REQ-0013, REQ-0040)

Prerequisite: PLN-0003.

- Describe device/package identity, memory, clock/module-stop/reset/protection data, ICU events, DMAC/DTC constraints and PFS routing.
- Describe GPIO, GPT, IIC, SCI/UART, SPI, ADC and DMAC registers, fields, capabilities and dependencies. Separate GPT instances from actual A/B output identities.
- Audit addresses, widths, reset masks, side effects, protection rules and package availability against source tables and usage notes. Record unresolved discrepancies explicitly.
- Instantiate the common tests with real RA4M1 data. Board aliases may be declarative, but do not add startup, ISR dispatch or functional peripheral code.

Exit: RA4M1's foundational and initial-peripheral descriptions are reviewed and pass metadata tests; the remaining inventory is assigned to PLN-0013.

## PLN-0014 Describe STM32H753 using the shared model
@status(done)
@link(REQ-0003, REQ-0004, REQ-0005, REQ-0006, REQ-0007, REQ-0010, REQ-0011, REQ-0012, REQ-0013, REQ-0033, REQ-0040)

Prerequisite: PLN-0003 and the STM32 sources from PLN-0001. No RA4M1 driver or MCUOS prerequisite.

- Implement real STM32H753 part/package, memory/core metadata, clocks/reset, pins, interrupts, DMA and the same initial peripheral categories using the common contracts.
- Represent native differences explicitly, including alternate-function routing, clock-domain relations, timer capabilities and DMA-accessible memory/cache properties. Revise shared contracts across all descriptions when needed.
- Extend in reviewed blocks through the remaining documented STM32H753 inventory. Validate with the shared tests and Cortex-M7 metadata compile probes.

Exit: the declared STM32 baseline is described and reviewed, with source gaps explicit. Bare-metal runtime backends and board execution are deferred to PLN-0018.

## PLN-0015 Describe ESP32-P4 using the shared model
@status(done)
@link(REQ-0003, REQ-0004, REQ-0005, REQ-0006, REQ-0007, REQ-0010, REQ-0011, REQ-0012, REQ-0013, REQ-0034, REQ-0040)

Prerequisite: PLN-0003 and the ESP32-P4 source baseline from PLN-0001. May be interleaved with RA4M1 and STM32 description work.

- Describe HP/LP and memory domains, GPIO matrix/IO MUX, reset/clocks, interrupt matrix, DMA engines and the same initial peripheral categories. Represent real basic/motor PWM hardware rather than copying GPT semantics.
- Use RISC-V/ESP-specific metadata where appropriate while preserving common concepts. Make unsupported capabilities and source gaps explicit.
- Extend through remaining documented blocks in reviewed increments. Compile real definitions against the same tests and the RISC-V metadata probes; do not equate metadata with working boot or multicore execution.

Exit: the declared ESP32-P4 baseline is described and reviewed. Unpublished content is handled under the common gate. Bare-metal backends and board execution belong to PLN-0019.

## PLN-0013 Complete and reconcile the description inventories
@status(done)
@link(REQ-0005, REQ-0006, REQ-0007, REQ-0010, REQ-0011, REQ-0012, REQ-0013, REQ-0033, REQ-0034, REQ-0040)

Prerequisite: PLN-0004; reconciliation uses completed increments from PLN-0014/PLN-0015.

- Complete remaining RA4M1 blocks, including documented system, memory/flash, DTC/ELC, serial, analog, USB/CAN and other inventory items. Work in reviewed increments using the established descriptions.
- Reconcile all three coverage inventories against source chapters/tables and declared variants. Account for every available documented block/register/field/route, even when no driver is planned.
- Distinguish unavailable source from unfinished implementation. Resolve source conflicts or missing material affecting the common resource/peripheral model. Raise a scope decision for genuinely unpublished residual blocks before gate acceptance.

Exit: all three source baselines have auditable coverage and no unexplained omissions. No driver is required to finish a description.

## PLN-0017 Validate all three descriptions before starting drivers
@status(done)
@link(REQ-0001, REQ-0003, REQ-0004, REQ-0006, REQ-0033, REQ-0034, REQ-0038, REQ-0039, REQ-0040)

Prerequisite: PLN-0004, PLN-0013, PLN-0014 and PLN-0015.

- Run the same metadata/constraint suite on all three descriptions. Include positive/negative tests for fields/access modes, pins/routes, clocks, IRQ/events, DMA constraints and real instance capabilities.
- Compile all three descriptions together and their headers independently, then run the architecture compile probes without startup, drivers or MCUOS. Verify namespace/type isolation and no global MCU-selection macro.
- Produce a comparison showing how the model represents actual differences in clock/pin/interrupt/DMA/timer structures. Confirm vendor-specific semantics remain accessible and are not silently discarded.
- Review complete coverage, provenance and explicit source-gap decisions. Record the common description gate as passed only when REQ-0040 is met.

Exit: all three MCUs demonstrably fit the description architecture. This permits driver implementation; it does not claim runtime driver portability, physical hardware behavior or RTOS support.

## Phase C: completed bare-metal foundation

Every item in this phase is gated by PLN-0017. Build backends against existing descriptions rather than adding MCU addresses to generic drivers. If a driver reveals a metadata omission or model error, correct and revalidate the affected contracts on all three before continuing.

## PLN-0005 Deliver bare-metal GPIO and UNO startup
@status(done)
@link(REQ-0003, REQ-0009, REQ-0014, REQ-0021, REQ-0031, REQ-0036, REQ-0037, REQ-0038, REQ-0041)

Prerequisite: PLN-0017.

- Add minimal UNO startup/linker/vector composition using the reviewed package, pins, memory and clock data. Document bootloader/flash offsets and startup ownership.
- Implement clock/protection/pin setup behavior in RA backends and the GPIO functional driver. Verify output initialization levels, reads, atomic set/clear and adjacent-pin preservation.
- Run native register-sequence tests and physical LED/loopback checks. Compare simple access code with a direct-register baseline.

Exit: the first end-to-end bare-metal firmware works with no MCUOS dependency, and the functional/backend/description boundaries remain intact.

Current evidence: all 22 native CTest entries pass, including clock/GPIO
register traces and negative compile gates; all architecture probes compile;
the bootloader-preserving UNO ELF links; and resource/direct-register
comparisons are recorded. On 16 September 2026 the J-Link/RTT runner flashed
the loopback-enabled image to an UNO R4 Minima and reported five passing
startup/clock/LED/D2-to-D3 GPIO tests with no failures. The runner resolves the
RTT control block from the ELF and uses a PTY-backed channel-0 logger on macOS.
On 17 September 2026 the split startup/GPIO image passed four tests and the
standalone D2-to-D3 GPIO image passed its low/high/low loopback test. Each run
now has an independent imported hardware artifact.

## PLN-0006 Implement bare-metal ownership, execution policy and transfers
@status(done)
@link(REQ-0012, REQ-0014, REQ-0015, REQ-0016, REQ-0017, REQ-0018, REQ-0019, REQ-0020, REQ-0035, REQ-0041)

Prerequisite: PLN-0017; PLN-0005 supplies physical startup/interrupt composition.

- Resolve exact operation storage, active-handle destruction and result-consumption rules before coding. Implement fixed peripheral ownership, move-only handles, typed results and single completion publication.
- Implement native fake and bare-metal policies for time, short critical sections, readiness and waiting. Preserve a narrow future RTOS adaptation interface without implementing or testing MCUOS yet.
- Implement deadlines, cancellation, quiescence and resource reuse. Add race/lifetime tests for completion around sleep, late IRQs, moved handles, exhaustion, cancellation and forbidden ISR waits.

Exit: blocking and polling consumers use the same state machine without lost wakeups or writes to released buffers. The execution boundary is documented and independent of any RTOS.

Current evidence: 79 native mcutest cases pass with no failures, including
fixed ownership, value and void transfers, repeated inspection and single
consumption, all completion-before-sleep interleavings, move/lifetime rules,
capacity exhaustion, cancellation/completion races, late generation rejection,
clock wrap, quiescence failure, reuse, active-destruction rejection and ISR-wait
rejection. The execution/transfer probe and self-contained public headers
compile under the native, Cortex-M4, Cortex-M7 and ESP32-P4 RISC-V presets; the
bootloader-preserving UNO R4 Minima firmware still links. No MCUOS header,
scheduler service, callback adapter or coroutine adapter is included.

## PLN-0007 Deliver timers and basic PWM
@status(done)
@link(REQ-0004, REQ-0011, REQ-0022, REQ-0036, REQ-0038, REQ-0041)

Prerequisite: PLN-0005; PLN-0006 for timer notifications.

Implement GPT behavior against the existing instance/output/clock metadata, constant-evaluated timing solving, frequency-tolerance reporting, duty endpoints and buffered updates. Compile real board bindings and verify native boundary cases plus physical start/update/stop waveforms.

Exit: timer notifications and PWM behavior match the configuration and declared tolerances; no nonexistent third GPT output is used.

Current evidence: 88 native mcutest cases and all 23 CTest entries pass. The
PWM cases cover exact 20 kHz solving, real 16-/32-bit limits and prescalers,
invalid and unattainable requests, integer rounding, 0%/100% endpoints,
exclusive GPT ownership, protected RA4M1 register sequences, buffered duty
updates, stopped-low behavior and fixed-storage overflow notification. Invalid
third-output binding is rejected by a compile-fail test. Public headers and
description probes compile under native, Cortex-M4, Cortex-M7 and ESP32-P4
RISC-V presets; the PWM-enabled UNO firmware links with the restricted runtime.
On 16 September 2026, J-Link/RTT ran the D3 GPT1B to D2 GPIO fixture on an
Arduino UNO R4 Minima. The zero-error 20 kHz solution used 2400 timer counts;
independent pin observation measured 2418 CPU cycles per period, 600 high
cycles at 25% duty and 1816 high cycles after the buffered 75% update, within
the declared 5% tolerance. Stop drove the observed pin low, all five board
tests passed, and the runner generated the imported hardware-test artifact.
On 17 September 2026 the new standalone PWM image independently passed with
2426 CPU cycles per period, 584 high cycles at 25% duty and 1842 high cycles
at 75% duty, followed by stopped-low verification. Its artifact is separate
from the startup, GPIO-loopback and UART results.

## Phase D: bare-metal implementation and target acceptance

## PLN-0008 Implement UART and I2C
@status(done)
@link(REQ-0024, REQ-0025, REQ-0026, REQ-0027, REQ-0036, REQ-0038, REQ-0041)

Prerequisite: PLN-0005 and PLN-0006. Use on-board VCP where available, documented loopback bridges/jumpers, or a directly wired I2C target. Record the observation limits of each physical procedure.

Implement SCI UART/baud solving and IIC timing/transactions against existing metadata. Add finite IRQ-backed operations, write/read/repeated-start semantics, error recovery and a fixed-storage external-device demonstrator. Test blocking/polling use on native models and real UART/I2C devices, including errors, timeout and reuse.

Exit: implementation, native tests, cross-builds and the selected direct-wired/board-native target procedures pass; the same external-device source runs against fake and real bus bindings.

Initial implementation evidence: SCI UART and IIC controller backends, timing solvers, finite
IRQ-backed transfers, typed errors, cancellation/recovery and the fixed-storage
`i2c_device` register helper were implemented. At that point the native suite had 102 passing
mcutest cases and all 23 CTest entries pass. Public headers and the compiled
UART/I2C usage example build under the native, Cortex-M4, Cortex-M7 and
ESP32-P4 RISC-V presets. The UNO startup/GPIO, GPIO-loopback, PWM, and UART
fixtures link as separate restricted-runtime executables without feature
defines or shared fixture state.

On 16 September 2026, J-Link/RTT ran the D1 SCI2 TX to D0 SCI2 RX fixture on
an Arduino UNO R4 Minima. All eight bytes passed at 115385 baud for the 115200
target, finite RX completed through RXI and final-wire TX completed through
TEI. The physical test exposed and verified the fix for a false startup frame:
while TE is disabled, SPTR now drives TXD at the asynchronous mark level rather
than leaving it high-impedance. The imported hardware artifact passes
REQ-0027 and REQ-0036. At that run, a real A4/A5 I2C device, pull-ups and
protocol capture were still absent.

On 17 September 2026, after board tests were split into independent images,
the standalone startup/GPIO image passed four tests and the standalone UART
image passed its SCI2 IRQ loopback test with the D1-D0 fixture. Their distinct
hardware artifacts are imported independently; no conditional fixture build
defines remain.

Also on 17 September 2026, a standalone instrumented I2C image was added and
cross-built for a dedicated 24LC256-compatible EEPROM on A4/A5. It preserves
and restores its scratch bytes while exercising IRQ-driven page writes and
combined repeated-START reads at 100 kHz and 400 kHz. The runner and aggregate
coverage flow include the new suite. It was subsequently replaced by the
available Grove-device procedure described below, whose physical artifact is
registered as trace evidence.

On 21 September 2026, the I2C image was adapted to the directly wired Grove
Gesture PAJ7620U2 at address 0x73 on A4/A5. The physical run selected register
bank 0 with an ordinary write and read the two-byte part ID `0x7620` through a
combined register-select/repeated-START transaction at both 99,334 Hz and
397,931 Hz. The test passed with IRQ-driven completion, explicit STOP,
controller shutdown and MCUCOV evidence at both rates. The target run exposed
and verified two RA4M1 receive fixes: ACKBT is now programmed through the
required prior ACKWP write, with WAIT released after the final byte, and TXI is
enabled before repeated START so the read-address event cannot be lost. Native
regressions cover one-, two- and three-byte receive endings and the complete
23-entry host CTest suite passes. This completes the direct-wired target exit
for PLN-0008. Independent timing/protocol capture, translated-voltage operation
and physical fault injection were not measured and are not claimed by this release.

## PLN-0009 Implement SPI and ADC
@status(done)
@link(REQ-0028, REQ-0029, REQ-0036, REQ-0038, REQ-0041)

Prerequisite: PLN-0005 and PLN-0006. Independent of PLN-0008 once shared contracts stabilize. Use a directly connected SPI device and a documented ADC source, and record their observation limits.

Implement SPI master behavior, segmented chip-select ownership, dummy reads and final-wire completion. Implement ADC single/finite-sequence conversion with validated channels/reference/sample timing and raw-result ordering. Exercise native failure/configuration cases and physical SPI/ADC fixtures with controlled ADC inputs.

Exit: both drivers pass native configuration/transfer/cleanup cases, cross-build, and the selected direct-wired or board-native target procedures.

On 17 September 2026, the portable SPI/ADC contracts and RA4M1 backends were
implemented. Native evidence covers SPI rate selection, mode/order/width
programming, segmented dummy/full-duplex transfers, GPIO-held chip select,
SPII/IDLNF final-wire completion, fault and timeout cleanup; ADC evidence
covers channel/sample validation, 12/14-bit setup, finite scans, raw result
ordering, rejection and timeout cleanup. The complete example cross-compiles
for Cortex-M4. At that point, physical SPI and controlled A1/A2 ADC board
artifacts remained outstanding.

The standalone `spi` and `adc` UNO images were then added and cross-built on
17 September 2026. SPI exercises mode 0/MSB at 1 MHz and mode 3/LSB at 6 MHz,
including a dummy-read segment and SPII completion. ADC requests AN001 then
AN000 at both 14- and 12-bit resolution, with five-percent rail thresholds for
A2 tied to 3.3 V and A1 tied to GND. The board runner and aggregate coverage
merge include both suites. At that stage neither unrun artifact was registered
in mcutrace.

On 21 September 2026, the SPI image was adapted to an Arduino Ethernet Shield
R3 with its WIZnet W5100 selected on D10 and the shared microSD device held
inactive on D4. At both 1 MHz and 6 MHz, the physical run read the W5100 retry
timer value 2,000, wrote and read back 42,330, then restored and verified 2,000.
Each rate completed ten four-byte IRQ-driven transactions, including segmented
reads with chip select retained through the command and dummy-response phases,
and reported ten idle completions with no error interrupt. The first run
exposed that the board binding incorrectly used the RA4M1's alternative P108
SSLB0 route even though UNO D10 is connected to P112. The corrected board and
route metadata, plus a native pin assertion, passed the physical rerun. The
connected potentiometer then supplied the direct ADC procedure on UNO A0. The
source-backed board and RA4M1 models were extended with A0/P014/AN009, and the
physical image sampled the stationary wiper at both resolutions: 2,680 at
14 bits and 670 at 12 bits, both corresponding to about 539 mV against the
documented nominal 3.3 V reference. The exact 4:1 result scaling passed the
five-percent stability tolerance, and the standalone hardware artifact is now
registered in mcutrace. Together with the passing native suite and Cortex-M4
build, this completes the direct-wired implementation exit. Independent SPI
timing/CS capture, calibrated multi-level ADC accuracy and physical fault
injection were not measured and are not claimed by this release.

## PLN-0010 Implement DMAC and peripheral DMA paths
@status(done)
@link(REQ-0013, REQ-0018, REQ-0019, REQ-0027, REQ-0028, REQ-0029, REQ-0030, REQ-0036, REQ-0041)

Prerequisite: PLN-0006 plus the relevant implementation portions of PLN-0008 and PLN-0009. Use memory-to-memory tests, internal known-answer cases, native-connector traffic and directly wired peripheral paths, retaining each procedure's observation limits.

Implement fixed DMAC ownership and operations using already-reviewed request/memory/width constraints. Add source-confirmed UART/SPI/ADC paths while preserving non-DMA operation. Test quiescence, stale completion, bounds and peripheral-versus-DMA completion ordering. DTC remains a separate driver extension even though its hardware is described.

Exit: documented directions/modes pass native tests, cross-builds and the selected internal/direct target checks without buffer-lifetime violations.

Started 22 September 2026 with the scheduler-independent RA4M1 DMAC channel
foundation. The first slice owns one of the four hardware channels, releases
the shared module-stop gate, validates finite aligned source and destination
ranges against the described flash/SRAM reachability, and performs normal-mode
software-triggered memory copies. Completion is published only after DTIF with
ACT clear. Cancellation disables DTE and software requests, waits a fixed bound
for ACT to clear, and either permits reuse or retires the operation slot; escape
and MMIO failures remain typed. Native tests cover exclusive ownership,
programming, count/width/alignment/range rejection, flash-to-SRAM acceptance,
cancellation, stale IRQs,
timeout with a stuck active flag, typed escape failure and reuse. A dedicated
Cortex-M4 compile probe instantiates the same backend. The second slice adds
event-triggered memory-to-peripheral and peripheral-to-memory transfers with a
fixed peripheral endpoint, incrementing memory endpoint, metadata-checked event
source, and DELSR disconnection during completion, cancellation and failure.
SPI exposes this only as an explicit 8-bit equal-length full-duplex DMA mode:
two DMAC channels consume SPI receive events and feed transmit events while the
existing segmented interrupt-driven API remains unchanged. The outer SPI
operation completes only after both DMAC operations succeed and SPI reports
IDLNF clear; either DMA failure cancels its peer and releases chip select.
Native tests cover the register programming, invalid event/peripheral/memory
rejection, two-channel completion ordering, final wire-idle ordering and peer
cancellation. The final slice adds explicit SCI UART transmit and receive DMA
operations and ADC single-channel continuous-scan capture. UART TX completion
consumes the DMAC result, disables TXI requests, and remains pending until SCI
TEI confirms TEND; UART RX completes only after DMAC has consumed the requested
number of RXI events. Receive errors and cancellation stop and retire the DMA
request before releasing the buffer. ADC continuous mode selects exactly one
bound channel, uses each ADI event to copy its result register into the next
16-bit buffer element, then stops ADST and clears channel selection before
publishing completion. DMA failure and timeout paths quiesce both the DMAC
request and owning peripheral. The ordinary interrupt-driven UART, segmented
SPI and single-scan ADC APIs remain independent of DMA. Native tests cover both
UART directions, DMA failure, cancellation/timeout, ADC mode programming,
buffer completion and peripheral-stop ordering.

The first physical DMAC image ran on the UNO R4 Minima on 22 September 2026.
Using DMAC0 normal mode and software activation, it copied sixteen 32-bit words
from code flash to SRAM and thirty-one 8-bit units between SRAM buffers. Both
known-answer comparisons passed, DMAC0_INT completed each transfer exactly once,
the channel shut down cleanly, and the J-Link RTT artifact includes mcutest and
mcucov output. This establishes the actual channel, memory reachability and
transfer-end interrupt path. The image was then extended against the stacked
Ethernet Shield R3 W5100. At 1 MHz, DMAC0 consumed SPI1 SPRI and DMAC1 serviced
SPI1 SPTI for ten four-byte full-duplex transactions. The test read the W5100
retry timer as 2,000, wrote and read back `0xA55A`, restored 2,000 and read it
back again. It observed ten receive-DMA interrupts, ten transmit-DMA interrupts,
ten final SPI-idle interrupts and zero SPI errors. The attached network cable
is recorded as connected-hardware context only: this result proves physical SPI peripheral
DMA and restoration, not Ethernet link or packet traffic. The capability review
also confirmed the remaining intended UNO request sources in the RA4M1 event
table: SCI2 RXI/TXI and ADC140 ADI.

The final physical procedures passed on the UNO R4 Minima. ADC140 continuously
sampled the potentiometer on AN009 and DMAC0 filled 32 16-bit samples from ADI
events; one transfer-end interrupt completed the buffer, values ranged from
2,667 to 2,686 with an average of 2,677, and the converter and channel were
stopped before ownership returned. The existing 14-bit/12-bit non-DMA ADC
regression also passed. With no D1-to-D0 jumper or receiver attached in the
current shield configuration, the available UART procedure exercised transmit:
DMAC1 fed 16 bytes to SCI2 from TXI events, produced one DMAC1 interrupt, and
the transfer completed only after one SCI2 TEI with no SCI error. The ordinary
interrupt-driven transmit regression also passed. UART RX DMA behavior,
failure and cancellation are covered by native tests; physical receive/error
stimulus and independent TX waveform/baud observation were not measured and
are not claimed by this release. Host tests, the Cortex-M4 cross-build and the available
internal/direct target checks now cover every documented mode, so PLN-0010 is
complete.

## PLN-0011 Accept the first RA4M1 bare-metal driver release
@status(done)
@link(REQ-0001, REQ-0002, REQ-0015, REQ-0016, REQ-0031, REQ-0035, REQ-0036, REQ-0037, REQ-0038, REQ-0039, REQ-0041)

Prerequisite: PLN-0005 through PLN-0010. MCUOS is explicitly not a prerequisite. D14 defines representative directly connected UNO hardware as the physical acceptance scope.

Run native contracts, compile examples, cross-build/link checks, physical driver scenarios and measurements. Establish numeric resource/latency budgets from measured results with the user before claiming acceptance. Publish source/coverage identities, API/storage/context rules, flash/debug procedures and reproducible evidence.

Exit: all D04 drivers are accepted on RA4M1 bare metal. Missing hardware tests or resource budgets remain blockers for this release; missing MCUOS code does not. REQ-0041's later three-target portability evidence continues in PLN-0018/PLN-0019.

Acceptance work started on 22 September 2026. The current working tree passes
174 native cases, all 23 CTest entries, the Cortex-M4 probe build and the full
UNO link build. A dedicated non-instrumented `-Os` image now records physical
DWT and linker evidence without confusing MCUCOV overhead with release costs:
109 cycles per checked GPIO write versus a four-cycle direct-register baseline,
a nine-cycle critical enter/leave pair, and 165 cycles for a real DMAC
completion-handler body. Its linked footprint is 21,248 bytes of flash and
12,984 bytes of static RAM including the 8 KiB evidence buffer; the conservative
stack watermark is 1,272 bytes within the fixed 4 KiB reserve. Backend instances
range from 48 to 152 bytes. The largest full instrumented driver image uses
115,472 bytes of flash and 14,928 bytes of static RAM. Exact ELF and source-tree
identities, reproduction commands, API/storage/context rules, flash/debug steps,
coverage identity and adopted numeric budgets are published in
`doc/acceptance/ra4m1-release.md`.

Completed 22 September 2026 after the user retired the cross-target fixture
requirement and selected the available directly connected hardware as the
release-acceptance scope. The measured limits published in the acceptance
record are adopted as the initial release budgets. PAJ7620U2 I2C, W5100
SPI/DMAC, potentiometer ADC, GPIO/PWM jumpers, UART, startup and internal DMAC
procedures provide 17 passing physical cases across nine images. Unmeasured
electrical limits, independent waveform/protocol capture, calibrated absolute
ADC accuracy and physical fault injection remain explicit non-claims rather
than hidden evidence or blockers. All D04 drivers are accepted on RA4M1 bare
metal; later STM32H753 and ESP32-P4 releases retain their own scoped evidence.

## PLN-0024 Enable W5100 Ethernet on Arduino UNO R4 Minima
@status(done)
@link(REQ-0002, REQ-0003, REQ-0009, REQ-0015, REQ-0017, REQ-0018, REQ-0019, REQ-0028, REQ-0030, REQ-0031, REQ-0035, REQ-0036, REQ-0037, REQ-0038, REQ-0041, REQ-0044)

Prerequisite: PLN-0011 and its accepted UNO SPI/DMAC foundation. Use the
connected Arduino Ethernet Shield R3 with W5100 and the available local-network
peer. This follow-on milestone does not reopen the first RA4M1 driver release.

1. Pin the official W5100 datasheet and Arduino Ethernet Shield R3 schematic,
   including chip revision, register/command behavior, buffer-memory layout,
   reset timing, D10/P112 chip select and D4 microSD isolation. Record the
   physical shield revision and network configuration used for acceptance.
2. Implement a scheduler-independent, fixed-storage W5100 device driver over
   the existing SPI controller. Cover common-register setup, MACRAW socket
   ownership, interrupt/status processing, RX/TX circular-buffer wrap,
   frame validation, backpressure, timeouts, cancellation and reinitialization.
   Keep interrupt-driven SPI available and use paired RA4M1 DMAC only where its
   bounded completion and cleanup rules are preserved.
3. Add deterministic native tests with a recording/fake SPI W5100 model. Test
   reset and commands, ordinary and wrapped RX/TX frames, malformed/oversize
   lengths, exhausted space, delayed/stale status, command timeout, peer DMA
   failure, cancellation and reuse. Add compile probes for public headers and
   the complete Cortex-M4 composition.
4. Reinspect the actual sibling `../mcuip` contracts and implement a thin
   W5100 MACRAW `NetworkDevice` adapter. Keep MCUIP responsible for frame and
   protocol storage; do not expose the W5100 hardware TCP/UDP socket API, copy
   MCUIP into mcuhw, add lwIP or depend on the Arduino Ethernet library.
5. Add a dedicated restricted-runtime UNO firmware and runner. Preserve the
   Arduino bootloader offset, emit mcutest/MCUCOV evidence over J-Link RTT and
   record source/ELF identities, SPI/DMAC mode, fixed buffer capacities, image
   sizes and timing/throughput measurements.
6. On the connected local network, reset and identify the W5100, enter MACRAW
   mode, exchange small and MTU-sized raw frames, then complete ARP plus either
   ICMP echo or a UDP datagram through MCUIP with an independent peer. Exercise
   peer loss or cable unplug/reconnect, ring pressure and recovery, and publish
   reproducible peer commands without claiming unmeasured electrical behavior.

Implementation checkpoint 22 September 2026: the fixed-storage driver, actual
sibling-MCUIP contract adapter, native fake-device suite, public-header checks,
Cortex-M4 linked composition, dedicated RTT/MCUCOV runner, and physical
ARP/ICMP path are implemented. The connected shield passed reset identity,
MACRAW, 60-byte and 1,514-byte raw frames, one ARP resolution, small and
full-MTU ICMP echo, then clean socket recovery followed by another successful
echo. The accepted instrumented image uses 142,248 bytes of flash and 22,760
bytes of static RAM. The completed performance follow-up compares 64 full-MTU
ICMP packets with a four-packet window that fits within the W5100's 8 KiB RX
allocation. All variants completed 64/64 replies without timeout or SPI error.
The 6 MHz interrupt-driven path reached 100,285 bit/s with 448.294 ms average
RTT; a W5100-specific bounded-polling path, retaining one `/SS` frame per
32-bit W5100 command, reached 159,763 bit/s and 281.000 ms at 6 MHz. At the
datasheet-compliant 12 MHz RA4M1 divider it reached 175,569 bit/s and 256.995 ms.
The three `-Os` images use 37,220/35,840/35,844 bytes of flash respectively and
21,112 bytes of RAM. A controlled manual cable unplug/replug produced 20 missed
peer replies, triggered a bounded W5100 socket reopen, and then returned to
successful ICMP traffic with zero SPI transport errors. The complete procedure,
source identities and benchmark comparison are published in
`doc/board/arduino-uno-r4-minima/ethernet-w5100.md`. PLN-0024 is complete.

Exit: the W5100 driver and MCUIP adapter pass native behavior tests, Cortex-M4
build/link checks and the physical local-network procedure on the UNO R4
Minima. Evidence proves bounded ownership and restored traffic after recovery;
the plan remains open if only W5100 register access or SPI traffic has passed.

## PLN-0018 Implement STM32H753 bare-metal backends
@status(done)
@link(REQ-0003, REQ-0004, REQ-0009, REQ-0013, REQ-0014, REQ-0017, REQ-0019, REQ-0021, REQ-0022, REQ-0024, REQ-0025, REQ-0026, REQ-0027, REQ-0028, REQ-0029, REQ-0031, REQ-0033, REQ-0036, REQ-0037, REQ-0041)

Prerequisite: the accepted STM32 descriptions and PLN-0017. The RA4M1 release is not a prerequisite. Use NUCLEO-H753ZI MB1364 Rev E and keep board configuration separate from MCU metadata.

1. Inventory the physical board assembly, jumper state and relevant solder bridges. Before changing a bridge, record its schematic designator, original and target state, purpose and restoration procedure.
2. Complete startup, clocks, memory layout, vectors, GPIO and debug procedures using ST-LINK and built-in board features.
3. Implement the common driver set against the existing descriptions. Use built-in LEDs/buttons, ST-LINK virtual COM, documented point-to-point jumper/bridge loopbacks, native USB where selected, and the on-board Ethernet PHY/connector with an ordinary network peer. Ethernet scope here is clock/pin setup, MDIO/PHY control, link state and raw MAC-frame exchange, not an implicit TCP/IP stack.
4. Validate cache/MPU behavior and DMA constraints with internal memory/peripheral known-answer tests and native-connector traffic where that gives independent observation.
5. Reuse functional tests and external-device sources with hardware bindings changed. Revalidate all three description contracts if the port exposes a model error. Record any unmeasured timing, analog, electrical or fault behavior as an explicit non-claim.

Completed 19 September 2026. The NUCLEO-H753ZI MB1364 Rev E startup boots
from `0x08000000`, verifies reset-state HSI at 64 MHz, owns the vector table and
AXI-SRAM runtime, and drives on-board LD2 through the shared GPIO interface.
The STM32H753 backends now implement TIM/PWM, USART, I2C, SPI and ADC against
the same functional interfaces used by RA4M1. They retain STM-native timing
encodings, compile-time package/AF bindings, finite operation pools, typed
errors, cancellation and final-event completion. GPIO, startup/clock and the
Cortex-M7 barrier path remain covered by their earlier shared implementations.

The native host suite instantiates all backends with instrumented MMIO and fake
events: 123 mcutest cases pass, and all 23 CTest entries pass including negative
compile constraints. Public headers build independently; the Cortex-M7 probe
and barrier-instruction check pass. The all-backend Nucleo image evaluates its
timing solvers at compile time, links without a hosted runtime or heap, and its
build reports flash/static-RAM use.

The Cortex-M7 board image is also an instrumented mcutest/mcucov suite. Eight
requirement-tagged tests run on the Nucleo and pass with no registry overflow.
The SWD runner extracts the binary coverage frame from cache-cleaned AXI SRAM
and produces JSON, HTML and VS Code-compatible LCOV reports. The imported target-specific report covers
startup/clock plus every STM32 backend module in this plan: 96/171 functions,
512/1062 statements, 564/1061 lines, 185/602 branches and 19/502 MC/DC
conditions. mcucov records 46 unsupported or constant-evaluated constructs as
not instrumented rather than presenting them as coverage. mcutrace imports both
the on-board mcutest and mcucov artifacts with no validation error.

The reproducible `run-nucleo-h753zi-native.sh` procedure passed on the connected
board with STLINK V3J5M2 at 3.28 V. Its result mask was `0x000000ff`: the
built-in LED passed set/read/clear through the common GPIO driver; TIM1
configured ARR=3199/CCR1=800 for the 20 kHz 25% case; USART3 BRR=556 and the
physical PD8/PD9 ST-LINK VCP delivered `MCUHW H753 USART3 OK`; I2C1 and SPI1
initialized on the source-backed Arduino pins; ADC1 calibrated and converted
floating A0/PA3 within the 14-bit range. The test also enabled the Cortex-M7
D-cache, cleaned a 64-byte aligned AXI-SRAM DMA1 source, and independently read
back `0xCACE0000..3` through SWD. No board jumper or solder bridge was changed.

`doc/board/nucleo-h753zi/README.md` records the board/source identity, received
configuration, commands, resource model, observation limits and non-claims.
PWM waveform measurement, controlled/calibrated ADC levels, real I2C/SPI
targets, peripheral-DMA traffic and supervised fault/electrical cases were not
executed and are not claimed. A D11-D12 SPI bridge and controlled A0 ties are safe
direct procedures but were not installed and are recorded as unexecuted,
not simulated evidence. Ethernet and native USB are independent board-native
extensions tracked by PLN-0023 rather than silently folded into this agreed
GPIO/PWM/UART/I2C/SPI/ADC backend exit.

Exit: the agreed STM32 backends pass native tests, Cortex-M7 builds and the selected board-native/direct-wired procedures; board modifications and observation limits are documented. No MCUOS Cortex-M7 port is required.

## PLN-0023 Implement Nucleo native USB and Ethernet connectivity
@status(in_progress)
@link(REQ-0003, REQ-0004, REQ-0009, REQ-0013, REQ-0033, REQ-0036, REQ-0037, REQ-0041, REQ-0043)

Prerequisite: PLN-0018 startup, memory/cache rules and board binding. Use the received
MB1364 Rev E assembly and ordinary USB/Ethernet peers; record any required
solder-bridge or jumper change before making it.

- Extend the source-backed STM32 description only where the USB or Ethernet
  backend needs currently absent MAC/PHY, MDIO, DMA descriptor, USB OTG or pin
  metadata. Re-run the three-description gate after shared-model changes.
- Bring up the on-board Ethernet PHY and connector: source/validate clocks and
  pins, read/write PHY registers over MDIO, observe link state, and exchange
  raw Ethernet frames with an ordinary peer. Keep this backend protocol-neutral,
  then add the explicit target adapter required by REQ-0043 against the actual
  sibling `../mcuip` public contracts. Do not add lwIP.
- Cross-build and link MCUIP with the Nucleo backend, then exchange ARP plus an
  ICMP echo or UDP datagram with an independent peer. Exercise link cycling,
  descriptor exhaustion, completion ordering and cache maintenance using fixed
  storage with the data cache enabled.
- Select the native USB role and connector path, implement the bounded endpoint
  and transfer subset required by the chosen smoke protocol, and exchange data
  with an ordinary host. Keep USB clock, FIFO and cache/DMA constraints explicit.
- Record link/USB captures, image sizes, cache maintenance, board state and
  repeatable host-side procedures. Keep unmeasured fault/electrical behavior as
  explicit non-claims.

Exit: Ethernet MDIO/PHY/link/raw-frame, the MCUIP protocol smoke path and the
selected native USB smoke path pass repeatable board-native procedures with
source-backed metadata and bounded storage. No lwIP, adapter hardware or MCUOS
dependency is implied.

## PLN-0019 Implement ESP32-P4 bare-metal backends
@status(in_progress)
@link(REQ-0003, REQ-0004, REQ-0009, REQ-0010, REQ-0011, REQ-0013, REQ-0014, REQ-0016, REQ-0017, REQ-0019, REQ-0024, REQ-0025, REQ-0026, REQ-0027, REQ-0028, REQ-0031, REQ-0034, REQ-0036, REQ-0037, REQ-0041, REQ-0043)

Prerequisite: PLN-0018 and the already accepted ESP32-P4 descriptions. Use the board's native connectors, on-board devices, direct header connections and ordinary peers for target-specific evidence.

Current ESP32-P4-NANO development evidence uses a custom second-stage loader and a separately linked factory application. The loader owns only initialization, partition validation and the fixed factory-selection policy; `mcutest`, MCUCOV and the functional GPIO/UART/I2C/I2S/audio/SDMMC/SDIO/SPI/Ethernet code reside in the application. ESP-IDF completes image/cache/libc/constructor and heap initialization, then the application takes control before main-task creation, so no FreeRTOS scheduler is started. The 158-case native host suite includes ESP32-P4 I2C timing, matrix routing, write/read/repeated-START command lists, explicit STOP, bounded FIFO validation, address/data NACK attribution, arbitration-loss, timeout, cancellation and recovery; I2S fractional timing, Philips-slot and directional GPIO-matrix routing, AHB-PDMA clock/reset/descriptor/completion, cache-policy, cancellation and ES8311 sequence tests; SD-card protocol initialization, capacity decoding and sector read/write plus native SDMMC clock, dedicated and matrix pin routing, slot/card selection, LDO, command and IDMAC behavior; scheduler-independent SDIO CMD5/RCA/CCCR/CIS/function-enable/block-size/four-bit enumeration; GPSPI2 timing, mode/order/width/CS configuration, one-command multi-segment ownership, completion, error, cancellation and CPU-buffer bounds; and native EMAC/RMII clock, route, MDIO, IP101 identity/link, fixed descriptor-ring, cache and MCUIP-adapter contracts. On the connected revision-v1.0 device, eight registered mcutest cases pass: GPIO2 register readback; I2C0 100 kHz register-select/repeated-START reads of the on-board ES8311 codec identity `0x8311` at address `0x18` on GPIO8/GPIO7; I2S0/ES8311 48 kHz capture of 4,032 bytes with 2,016 nonzero analog-microphone samples followed by a completed 193,536-byte, 1,008 ms, 1 kHz DAC tone under GPIO53 amplifier control; native SDMMC identification of the 61,454,336-sector SDHC card plus sector-0 read and last-sector write/read/restore verification on GPIO39-GPIO45; native SDIO slot-1 enumeration of the GPIO14-GPIO19 ESP32-C5 wireless coprocessor after GPIO54 reset, including two advertised functions, RCA `0x0001`, CCCR `0x32`, CIS `0x01000`, Espressif identity `0x0092/0x6666`, function-1 ready, 512-byte block sizes and four-bit mode; one GPSPI2 1 MHz four-frame, two-segment command on expansion GPIO20-GPIO23; a UART0 finite transfer whose marker is physically received through the on-board CH343 USB bridge after TX-done completion; and native EMAC/RMII/SMI initialization plus physical identification of the on-board IP101GRI at PHY address 1 with ID1 `0x0243`, ID2 `0x0c54`, OUI `0x0090c3` and model `0x05`. The PHY reported BMSR `0x7849` and link down because no Ethernet peer was connected, so raw-frame exchange and the physical MCUIP smoke path remain open. Revision v1.0 uses different I2C0 APB-gate/reset bits, retained as an explicit development-board binding without altering the production-v3.x description. The MCUCOV FULL-profile capture reports 180/279 functions, 1129/1886 statements, 1275/2010 lines, 486/1102 branches and 72/880 MC/DC conditions. The on-board codec provides physical I2C transaction and live microphone/DMA evidence; the test deliberately produces an audible tone, while independent level/frequency/distortion measurement, timing/protocol capture and injected I2C faults are not claimed. The SDMMC result proves 4-bit physical block I/O and restoration but does not claim filesystem or multi-block support. The C5 result proves the physical SDIO link, identity and function readiness but not ESP-Hosted packet transport, remote Wi-Fi API operation, scanning, association or IP connectivity. UART RX/error stimulus still needs a direct loopback if that behavior is claimed. The SPI result proves controller execution and idle completion but, with no external slave, loopback or logic analyzer attached, does not claim returned data or electrical timing/CS-waveform behavior. The Ethernet result proves native MAC/pin/clock/reset setup and SMI communication with the physical PHY; live link, electrical RMII traffic, raw frames and protocol traffic still require an ordinary cabled peer. The v1.0 result remains development-only evidence outside the accepted v3.x silicon range.

Choose the board, toolchain/boot path and initial core ownership. Implement the common driver set against existing Espressif descriptions, validating actual clock, routing, memory/cache and DMA behavior. Keep selected boot support separate from functional drivers and ensure the delivered execution mode does not require an RTOS scheduler. Run common and physical target tests; concurrent multicore/LP support needs separate evidence.

Exit: all three declared bare-metal targets have real backends supporting the shared interfaces, with host/cross-build evidence and their selected board-native/directly connected target procedures. Description compatibility and runtime portability are separately demonstrated; observation limits remain explicit.

## PLN-0025 Deliver the modern C++ application interface foundation
@status(done)
@link(REQ-0002, REQ-0014, REQ-0015, REQ-0018, REQ-0035, REQ-0037, REQ-0038, REQ-0045)

Implemented the optional `mcuhw::io` target and board-oriented API on the
existing checked drivers. Compile-time configuration selects peripheral and
DMA storage. A shared runtime tracks pin aliases and retained handles;
move-only leases and asynchronous operations expose explicit lifetime and error
contracts. Execution/MMIO injection provides the extension boundary while this
increment remains single-core foreground polling.

The UNO, NUCLEO and ESP32-P4-NANO profiles retain their existing fixed clock and
silicon assumptions. Native tests cover ownership, rollback, completion,
timeouts, cancellation and ISR rejection, with JSON evidence in
`build/host/artifact/mcutest-io.json`. Positive/negative compile cases and all
public headers build for the three architectures. UNO blink, serial echo and
SPI DMA examples link with the existing standalone startup. The
[application guide](doc/getting-started/io.md) documents API use and limitations.
`tool/report-io-cost.py` produces paired linked probes, source hashes and
flash/static-RAM/individual-stack-frame reports in `build/io-cost/`.

Exit scope: implemented and checked at the host/build level. No physical
acceptance of the new facade, peak-stack/latency measurement, camera API,
MCUOS adapter or MCUIP service is claimed by this milestone.

## PLN-0026 Add shared bus endpoints and attached-device drivers
@status(done)
@link(REQ-0014, REQ-0018, REQ-0019, REQ-0024, REQ-0028, REQ-0035, REQ-0037, REQ-0038, REQ-0044, REQ-0046)

Implemented I2C address leases and SPI device leases over one stable controller
per enabled bus. SPI controller backends support external CS handling and idle
reconfiguration on RA4M1, STM32H753 and ESP32-P4; existing raw backend behavior
remains available. Retained result handles prevent device release and bus
shutdown. Failed SPI shutdown blocks reuse until a successful retry.

Attached drivers now live under `device/`: TMP102 owns an I2C endpoint and
returns signed fixed-point samples; W5100 owns a SPI endpoint and retains a
separate register protocol plus direct controller transports. ES8311 codec
configuration lives under `device/audio/`. Generic MCUIP integration replaces
board-specific naming; the former driver headers and compatibility aliases
have been removed, and their callers use device headers directly.
W5100 validates SPI settings, shares absolute operation deadlines, copies at
most a configured payload budget per poll, resumes partial work and requires
recovery after ambiguous commits or cancellation of an issued SEND.

The host suite, all three architecture/header builds, generic MCUIP contract
probes and five linked UNO examples provide build/simulation evidence. New
examples cover a temperature threshold and raw Ethernet reception. Fresh
paired cost probes quantify the additional shared-SPI state and code; the
[components guide](doc/getting-started/components.md) documents the API and
its single-foreground-context contract. Physical verification and renewed
throughput measurements of the refactored W5100 remain separate work.

## Phase D2: repository adoption and scale

## PLN-0027 Separate consumer builds and export the library package
@status(done)
@link(REQ-0002, REQ-0003, REQ-0038, REQ-0047)

Prerequisite: PLN-0026. This begins the D18 adoption work immediately after the
attached-device foundation.

Delivered: interface-only default builds, explicit maintainer options and a
versioned relocatable install/export. Real source, installed/relocated and
direct-header consumers pass without sibling projects; parent tests and compiler
settings remain intact. See [adoption qualification](doc/acceptance/repository-adoption.md).

- Make embedded CMake use define only the public library interfaces by default.
  Gate tests, probes, header checks, coverage and hardware images with explicit
  mcuhw options; preserve parent testing and compiler settings.
- Add versioned install/export support for `mcuhw::mcuhw`, `mcuhw::io` and
  `mcuhw::device`, preserving source-checkout and direct-header consumption.
- Resolve optional dependencies only when selected. Retain explicit local source
  overrides without requiring the sibling development workspace for applications.
- Add clean external-consumer checks for source inclusion, installed and relocated
  packages, a parent with its own tests enabled, and board-independent headers.
  Keep maintainer verification available through explicit configurations.

Exit: source and installed consumers build without sibling projects, source-tree
path leakage or unrelated validation targets; parent test behavior is unchanged.

## PLN-0028 Establish local board ownership
@status(done)
@link(REQ-0003, REQ-0005, REQ-0010, REQ-0038, REQ-0046, REQ-0048)

Board build settings and runner configuration live in each board's `board.cmake`.
Shared MCU, backend and device implementations retain their public C++ paths.
The earlier component catalog has been removed from the current implementation.

## PLN-0029 Unify selected-board firmware builds across the existing targets
@status(done)
@link(REQ-0031, REQ-0038, REQ-0045, REQ-0046, REQ-0047, REQ-0048, REQ-0049)

Prerequisite: PLN-0028. Reuse the existing UNO, NUCLEO and ESP32-P4 integrations;
completion of the remaining PLN-0019/PLN-0023 peripheral work is not required.

Delivered: pre-project board/toolchain selection, local build entries, explicit
startup ownership and separate reusable runners. All six external Blink/TMP102
applications link on the measured macOS host, including the ESP SDK boot path;
external startup and relocated installed firmware also pass. No new physical
acceptance is claimed. Detailed scope is in the adoption qualification record.

- Replace central board-specific build branches with selected local board rules
  and shared firmware/linker/artifact helpers. Select toolchains before compiler
  configuration and isolate each board/toolchain build directory.
- Offer explicit supplied-startup and external-BSP/RTOS-startup modes with one
  owner for reset, clocks, vectors and memory layout. Adapt the ESP SDK/boot path
  to the common workflow while preserving its revision and acceptance limits.
- Factor reusable flash/debug runners from board-specific probe/port/flash-region
  settings. Keep flashing separate from compilation and document recovery.
- Create complete external Blink and TMP102 projects, with board profiles and
  wiring configuration, and cross-build/link them for all three current boards.
  Check unknown identities, unsupported revisions, incompatible toolchains and an
  externally supplied startup composition.

Exit: one board-selection workflow builds the three target integrations without
unrelated targets or duplicate startup ownership. Build results carry no new
physical acceptance claim.

## PLN-0030 Deliver reproducible setup and the first-application workflow
@status(in_progress)
@link(REQ-0002, REQ-0036, REQ-0038, REQ-0047, REQ-0049, REQ-0050)

Prerequisite: PLN-0029.

Implemented: optional new/setup/doctor/build/flash frontend, pinned
tool and SDK identities, validated archive caches, explicit overrides and
operation-specific diagnostics. Real macOS setup/build/offline rebuilds and
runner-command tests pass. Linux and Windows clean first-application setup and
cross-build execution remain unqualified; the hosted native CI matrix has been
added but has not run from this checkout. This item stays in progress until
the required host-specific results exist.

- Publish the tested Linux/macOS/Windows host matrix and compiler, C++23 library,
  CMake/Ninja, required SDK and debugger versions. Pin dependency/toolchain inputs
  and integrity checks while allowing explicit installed-tool/source overrides.
- Provide explicit setup with reusable caches and offline rebuilds. Diagnose
  missing/incompatible tools and cache entries for the requested operation with
  concrete remediation; missing flash/debug tools must not block build-only use. Avoid
  accidental dependence on sibling checkout presence or unpinned downloads.
- Add a thin frontend for project creation, prerequisite checks,
  build and explicit flash, reusing CMake/runner definitions. Document equivalent
  direct commands and keep the frontend optional for library consumption.
- Exercise clean setup and external application builds on all three host OSes,
  cached offline builds, paths requiring proper quoting and diagnostic failures.
  Test runner argument selection separately from actual hardware execution.

Exit: documented commands take a clean supported host to a reproducible linked
application. Tool/source identities and overrides are recorded; only physically
exercised runner combinations receive hardware evidence.

## PLN-0031 Publish component contribution workflows
@status(done)
@link(REQ-0036, REQ-0038, REQ-0039, REQ-0048, REQ-0049, REQ-0050, REQ-0051)

Board, device and MCU contribution guides describe local files, focused checks,
complete applications and evidence scope. Searchable generated support listings
are deferred.

## PLN-0032 Scale verification and qualify the repository adoption workflow
@status(in_progress)
@link(REQ-0035, REQ-0036, REQ-0037, REQ-0038, REQ-0039, REQ-0047, REQ-0048, REQ-0049, REQ-0050, REQ-0051, REQ-0052)

Prerequisite: PLN-0027 through PLN-0031; suite decomposition may begin after
PLN-0028 establishes component identities and dependencies.

Implemented: selectable native suites, full native and declared-board CI
selection, selected-input firmware and flash-bundle provenance. The earlier
catalog-specific scale and impact checks were removed. Integrated Linux/Windows
first-build qualification and hosted CI execution remain open.

Implemented target usability refinement: one `check-headers` target retains
independent header compilation; the shared preset base disables optional checks.
Selected application/component configurations have explicit default build targets,
while exhaustive maintainer presets, focused executables and test artifacts remain
available. Internal support targets have IDE folders. The local CMake File API
reports one project target per board application, two for the TMP102 component
configuration and 21 for full host verification. See
[targets and presets](doc/development/verification.md#targets-and-presets).

The D19 workspace refinement selects the architecture through a preset and
persists `MCUHW_ACTIVE_BOARDS` as a local CMake setting. Local board entries supply
vendor-qualified board example/test aggregates, with separate example images and
board/revision artifact directories. Reconfiguration removes inactive targets;
board-test dependencies are configured only when that group is requested.
An explicit `all` selection enables exhaustive compatible-board builds. The
focused regression adds 100 unselected board entries without changing targets
or compilation commands and builds two active board identities independently.

- Keep core, MCU/backend, board and device checks selectable with separate
  artifacts. Run the complete set in CI while dependency metadata is absent.
- Hash actual selected transitive source, configuration and toolchain inputs;
  record repository revision separately.
- Run external-consumer, board application and host setup checks, retaining
  physical acceptance as a separate observation.
- Keep mcutrace source/build/artifact links aligned with delivered outputs.

Exit: clean applications, local contributions and full-maintainer validation have
separate reproducible workflows with the selected board settings. Existing evidence
remains attributable; the milestone is marked done only after these checks run.

## Phase E: deferred MCUOS integration

## PLN-0020 Integrate MCUOS after bare-metal driver acceptance
@status(planned)
@link(REQ-0002, REQ-0015, REQ-0016, REQ-0018, REQ-0019, REQ-0031, REQ-0032, REQ-0038, REQ-0041)

Prerequisite: PLN-0011, PLN-0018 and PLN-0019. Hardware MCUOS acceptance additionally needs an executable MCUOS target; its absence does not reopen prior bare-metal acceptance.

- Reinspect `../mcuos` when this stage begins and pin the actual source/configuration identity. Its facade and target APIs may have changed since the planning review.
- Add optional MCUOS build integration with an explicit source-path setting. Implement the adapter using task-level facade calls and supplied ISR context, bounded completion transport, absolute deadlines and task-owned bus synchronization.
- Test actual MCUOS native tasks with fake peripherals, covering race boundaries, unrelated notifications, saturation/exhaustion, owner death and cancellation versus hardware quiescence. Preserve the same driver/backend sources as bare metal.
- Complete the first physical MCUOS composition on RA4M1 when context switching, interrupt entry/return, time/alarm and startup support exist. Reserve RTOS-owned timers/vectors and enforce its kernel-callable interrupt ceiling. Track later MCUOS hardware targets independently.

Exit: native MCUOS adapter behavior and the first claimed physical MCUOS target pass their own acceptance checks. No driver rewrite or mandatory RTOS dependency has been introduced.

## Phase F: later extensions

## PLN-0012 Add synchronized motor PWM
@status(planned)
@link(REQ-0004, REQ-0010, REQ-0023, REQ-0036)

Prerequisite: PLN-0007, PLN-0011 and a target board exposing a valid timer/output group plus an appropriate measurement path. Scheduled after the initial bare-metal release; independent of MCUOS.

Implement grouping, synchronization, complementary output pairs, dead time and fault/disable behavior using already described capabilities. Validate real timer/pin combinations and capture alignment, updates, dead time and fault outputs.

Exit: valid groups pass compile-time and physical waveform checks; invalid groups are rejected.

## PLN-0016 Optional adapters and broader driver modes
@status(planned)
@link(REQ-0002, REQ-0004, REQ-0018, REQ-0020, REQ-0038)

Prerequisite: the relevant accepted bare-metal driver and explicit selection of an extension. This is a roadmap item, not a commitment to every example.

Evaluate fixed-storage coroutine/callback adapters, generation tooling, DTC drivers, additional bus modes, streaming ADC/UART and drivers for otherwise description-only blocks. Add requirements, source review and acceptance cases before implementing a selected extension.

Exit: each selected extension preserves storage, ownership and portability contracts and has explicit evidence.

## Critical path and evidence boundaries

The path is all-three sources → shared metadata foundation → three incremental complete descriptions → common description gate → completed RA4M1 foundation → scoped target releases and the application/device foundation → repository adoption and scale (PLN-0027 through PLN-0032, immediately after PLN-0026) → MCUOS. Remaining target-specific acceptance proceeds independently of the repository work. Each target report defines what its physical procedures observed and preserves explicit non-claims; it does not require a shared cross-target fixture. MCUOS ports matter only in the later RTOS phase.

Compile-time tests establish representability and reject invalid configurations; they do not verify hardware register behavior, electrical timing or driver portability by themselves. Those claims require subsequent native behavioral tests and independent physical observations. Keep the coverage denominator, verification state and explicit exclusions visible throughout.
