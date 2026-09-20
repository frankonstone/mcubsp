# mcuhw implementation plan

Revised baseline, 19 September 2026. Implements [requirements.md](requirements.md). The current sequence is: **preserve the completed three-MCU description and RA4M1 driver foundation; continue adapter-independent implementation and board-native verification while reusable instrumentation hardware is built; join the two tracks for deferred electrical acceptance; integrate MCUOS last**. Status annotations below track execution; a completed plan is not itself requirement evidence. Existing plan IDs and evidence are retained. Read in the phase order below, not numeric ID order.

## Delivery phases

| Phase | Work | Exit |
| --- | --- | --- |
| A: description foundation | Sources and coverage inventories for RA4M1, STM32H753 and ESP32-P4; core metadata/MMIO contracts; native and cross-compile probes | Shared model can express real differences without vendor register-layout assumptions |
| B: three complete descriptions | Reviewed block-sized increments across all three, followed by remaining documented blocks | All three declared baselines pass the common description and coverage gate, before any backend/driver implementation |
| C: completed bare-metal foundation | Preserve completed RA4M1 startup, GPIO, execution, transfer, timer and basic-PWM work | Stable fixture-facing baseline; existing evidence remains valid |
| D: parallel hardware and board work | Build the common Arduino shield, ESP32-P4-NANO carrier and STM32 expansion adapter while continuing adapter-independent target work | Rev-A fixtures are safe/calibrated, and board-native implementation has not been needlessly stalled |
| E: staged bare-metal completion | Finish pending driver implementations and target backends first; execute adapter-dependent cases after fixture qualification | Implementation evidence stays distinct from complete physical acceptance for each claimed target |
| F: MCUOS | Optional adapter using `../mcuos`, tested natively and then on an executable MCUOS hardware target | RTOS portability demonstrated independently of earlier bare-metal releases |

Motor PWM and other extensions remain later milestones. MCUOS source, API readiness and hardware ports are not prerequisites for phases A–E. During fixture design and fabrication, new software slices may proceed when they can be verified with host models, cross-builds, debug access, built-in board features, documented bridges/direct wiring, native connectors and ordinary peers, or internal known-answer tests. Only adapter-dependent electrical acceptance waits for Rev-A bring-up.

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
  backends/             added after description gate: peripheral register sequences
  drivers/              added after description gate: functional interfaces and logic
  devices/              later MCU-independent external-device drivers
  execution/            later portable execution contract and bare-metal policy
boards/               package/pin aliases separate from MCU data; startup added later
src/                    non-template behavior where useful
adapters/mcuos/          phase F only
examples/               metadata examples first; bare-metal firmware after the gate
tests/                 description contracts, then driver models and hardware procedures
hardware/fixtures/      KiCad projects, shared fixture interface, BOMs and fabrication outputs
doc/                   manuals, provenance and coverage manifests, design decisions
cmake/                 native and architecture/toolchain configurations
```

Templates and constant-evaluated definitions live in headers; other behavior may use compiled translation units. Description headers must be usable together on a host without board startup, OS includes or a global target-selection macro. Do not create empty target trees as evidence of support.

## Phase A: sources, contracts and description test infrastructure

## PLN-0001 Pin sources and coverage scope for all three MCUs
@status(done)
@link(REQ-0001, REQ-0005, REQ-0006, REQ-0033, REQ-0034, REQ-0039, REQ-0040)

Prerequisite: none. D01–D08 are recorded, including the revised ordering.

- Inventory RA4M1, STM32H753 and ESP32-P4 documented blocks, part/package variants and source revisions. Create separate description and driver-support columns and explicit source-gap states. Prioritize the same resource and initial-peripheral categories across all three.
- Review the now-local STM32H753 RM0433 Rev.8, DS12117 Rev.10 and ES0392 Rev.15. Their identity/readability checks and all six supplied PDF checksums are recorded in [the source manifest](doc/source-manifest.json). Pin applicable RA4M1 corrections and the ESP32-P4 source/silicon baseline; resolve published updates and preliminary gaps before claiming definitions.
- Use the generated [Markdown manual index](doc/markdown/README.md) for section lookup and text search; follow its exact PDF-page links to verify register tables, diagrams and ambiguous extraction. The conversion records source/output hashes and preserves all 9,192 page locations; it does not replace the detailed source audit.
- Record checksums, section/table provenance, supported package scope and known exclusions. Ask for important scope decisions if a source is unavailable or the selected variants materially change the work; never invent register data.
- Keep MCU family data separate from board choices. UNO is the initial physical driver target, but board fixtures and later STM32/ESP boards are not required to inspect or compile descriptions.

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

## Phase D: parallel fixture construction and board-native implementation

After the completed description gate, work proceeds on two parallel tracks. PLN-0021 designs, fabricates and qualifies the reusable fixture. The software track continues backend implementation, host verification, cross-builds and target tests that use debug access, built-in board features, documented bridges or point-to-point jumpers, native connectors with ordinary peers, or internal known-answer/self-tests. Only tests whose stimulus or independent observation comes from the shared adapter wait for Rev-A electrical bring-up. Each plan and evidence record must say which track it belongs to.

## PLN-0021 Design and qualify the cross-target test-fixture family
@status(planned)
@link(REQ-0005, REQ-0010, REQ-0033, REQ-0034, REQ-0036, REQ-0038, REQ-0041, REQ-0042)

Prerequisite: PLN-0017 and the completed fixture-facing work in PLN-0005 through PLN-0007. NUCLEO-H753ZI MB1364 revision E is selected; reconcile the MB1364-H753ZI-E01 schematic with the physical assembly options, solder bridges and jumper state before freezing the STM32 connector mapping.

The Rev-A planning baseline is recorded in [the PLN-0021 fixture workbook](outputs/pln-0021/mcuhw-fixture-plan.xlsx). Its BOM, shared-net matrix, complete ESP32-P4-NANO P1/P2 allocation, Nucleo wing connections, Pico 2 supervisor pinout, native-feature coverage and source register are design inputs for KiCad rather than a released purchase list. The workbook freezes the Arduino-shield common paths and the initial P4 carrier assignment while keeping board-native high-speed interfaces on dedicated cables and peers. Before schematic freeze, resolve its explicit gaps: enumerate inaccessible UNO R4 SCI routes, complete the Nucleo instance-to-morpho audit against the physical revision-E assembly, and prove a non-interfering ADC2 path on the P4 board or retain that limitation as a blocking gap. Verify all candidate footprints, alternates and live distributor availability before ordering.

1. Build a board/feature/peripheral coverage matrix from the UNO R4 Minima, selected STM32H753 and ESP32-P4-NANO schematics plus the existing description inventories. Give every instance and board feature an electrical test, native-connector test or internal known-answer method. Record signals unavailable at board connectors as blocking gaps rather than silently treating a peripheral type as fully covered.
2. Freeze a shared fixture interface and partition it into an Arduino Uno R3 shield, a small Nucleo morpho/Zio wing for non-Uno signals, and an ESP32-P4-NANO carrier using its two 2x13 headers. Keep native USB, Ethernet, TF/SDIO, MIPI CSI/DSI, audio, RTC, debug, boot and power connectors accessible for cabled fixture modules.
3. Use a socketed Raspberry Pi Pico 2 with headers as the initial USB fixture supervisor. Put all DUT-facing outputs into hardware-default high impedance until the supervisor has identified the adapter, IOREF and jumper state. Use direction-controlled `SN74LVC8T245` banks for grouped push-pull signals and `PCA9306` for I2C; do not use an auto-direction translator as the universal bus solution. Provide per-bank disconnect/direction jumpers, 1 kOhm-class series protection where timing allows, probe grounds and labeled logic-analyzer headers.
4. Populate a 24AA256/24LC256-compatible SOIC-8 I2C target and 25LC256 SOIC-8 SPI target in the target voltage domain. Add passive UART/SPI/GPIO loopback selections, supervisor-driven open-drain fault injection, buttons/DIP inputs, buffered LEDs, timer/PWM capture paths and an RC-filtered PWM measurement channel. Make bus pull-ups, chip selects, terminations and deliberate error conditions jumper-selectable.
5. Generate target-relative analog levels with a 0.1% resistor ladder buffered by an MCP6004-class rail-to-rail op amp. Measure the actual stimulus and DUT DAC/PWM outputs independently with an ADS1115-class VSSOP-10 ADC behind protected dividers; calibrate gain, offset and IOREF for each assembled fixture. Add an INA226-class current/power monitor on a switchable external DUT supply path, not in a way that forces the DUT bus under test to operate the instrument.
6. Provide a target-side MCP2562FD-class CAN-FD transceiver, switchable 120-ohm termination and a second independent node. The initial independent node is an MCP2518FD-class SSOP external controller plus CAN-FD transceiver controlled by the supervisor; keep a standard CAN header so a known USB-CAN adapter can cross-check it.
7. Treat high-speed and board-native functions as dedicated modules: known USB host/device peers and cables, an Ethernet peer/switch, qualified SD cards, the vendor-compatible MIPI camera/display, speaker/acoustic loopback, RTC supply simulator and radio/co-processor test procedure. These modules belong in the fixture BOM and coverage matrix but are not routed through the common low-speed shield.
8. Capture the design in KiCad with hierarchical sheets shared between the two carriers where practical. Prefer 0805 passives, SOIC/TSSOP/SSOP/SOT-23 actives, 2.54-mm jumpers and socketed modules. At schematic freeze, record lifecycle, two-source availability or an approved alternate, package/footprint verification and hand-assembly notes for every active part and connector.
9. Review power trees, back-feed paths, IOREF limits, translator direction, unpowered behavior, analog limits, connector clearance and worst-case contention before fabrication. Run ERC/DRC, inspect Gerbers and drill files, then assemble one common shield and one ESP carrier. Bring up rails/current limits first, supervisor and self-test second, each interface third, and only then attach each DUT.
10. Version the supervisor protocol/firmware, board ID, calibration record, KiCad sources, fabrication outputs, BOM/alternates, assembly drawings, jumper tables and reproducible smoke-test procedures. Record Rev-A faults and ECOs without hiding bodges or substituted parts.

Exit: REQ-0042 acceptance is met; the same Arduino shield passes safe startup and the common digital/analog/bus self-tests on UNO R4 Minima and the selected STM32H753 board, the ESP32-P4-NANO carrier passes the equivalent checks, and every described peripheral instance and board feature has an executable test path or an explicit blocking gap.

## Phase E: bare-metal implementation and staged acceptance

## PLN-0008 Implement UART and I2C
@status(in_progress)
@link(REQ-0024, REQ-0025, REQ-0026, REQ-0027, REQ-0036, REQ-0038, REQ-0041)

Prerequisite: PLN-0005 and PLN-0006. No fixture prerequisite. Use on-board VCP where available, documented loopback bridges/jumpers, or a directly wired I2C target for pre-fixture target tests. Translated-voltage operation, supervised fault injection and reusable-fixture coverage move to PLN-0022.

Implement SCI UART/baud solving and IIC timing/transactions against existing metadata. Add finite IRQ-backed operations, write/read/repeated-start semantics, error recovery and a fixed-storage external-device demonstrator. Test blocking/polling use on native models and real UART/I2C fixtures, including errors, timeout and reuse.

Exit: implementation, native tests, cross-builds and available direct-wired/board-native target procedures pass; the same external-device source runs against fake and real bus bindings. Deferred adapter-dependent rows are enumerated for PLN-0022. This implementation exit is not full physical release acceptance.

Current evidence: SCI UART and IIC controller backends, timing solvers, finite
IRQ-backed transfers, typed errors, cancellation/recovery and the fixed-storage
`i2c_device` register helper are implemented. The native suite has 102 passing
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
REQ-0027 and REQ-0036. A real A4/A5 I2C device, pull-ups and protocol capture
are still absent, so this plan remains in progress.

On 17 September 2026, after board tests were split into independent images,
the standalone startup/GPIO image passed four tests and the standalone UART
image passed its SCI2 IRQ loopback test with the D1-D0 fixture. Their distinct
hardware artifacts are imported independently; no conditional fixture build
defines remain.

Also on 17 September 2026, a standalone instrumented I2C image was added and
cross-built for a dedicated 24LC256-compatible EEPROM on A4/A5. It preserves
and restores its scratch bytes while exercising IRQ-driven page writes and
combined repeated-START reads at 100 kHz and 400 kHz. The runner and aggregate
coverage flow include the new suite, but it is not registered as trace evidence
until a physical execution produces its artifact. A directly wired target may
complete this implementation plan; independent logic-analyzer capture and
fixture-controlled error cases remain PLN-0022 work.

## PLN-0009 Implement SPI and ADC
@status(in_progress)
@link(REQ-0028, REQ-0029, REQ-0036, REQ-0038, REQ-0041)

Prerequisite: PLN-0005 and PLN-0006. Independent of PLN-0008 once shared contracts stabilize; no fixture prerequisite. Temporary SPI loopback/device wiring and documented ADC ties or bench sources may be used now. Calibrated multi-level stimulus, fixture-controlled error cases and reusable-fixture coverage move to PLN-0022.

Implement SPI master behavior, segmented chip-select ownership, dummy reads and final-wire completion. Implement ADC single/finite-sequence conversion with validated channels/reference/sample timing and raw-result ordering. Exercise native failure/configuration cases and physical SPI/ADC fixtures with controlled ADC inputs.

Exit: both drivers pass native configuration/transfer/cleanup cases, cross-build, and every applicable direct-wired or board-native target procedure. Deferred adapter-dependent rows are enumerated for PLN-0022. This implementation exit is not full calibrated analog or electrical acceptance.

On 17 September 2026, the portable SPI/ADC contracts and RA4M1 backends were
implemented. Native evidence covers SPI rate selection, mode/order/width
programming, segmented dummy/full-duplex transfers, GPIO-held chip select,
SPII/IDLNF final-wire completion, fault and timeout cleanup; ADC evidence
covers channel/sample validation, 12/14-bit setup, finite scans, raw result
ordering, rejection and timeout cleanup. The complete example cross-compiles
for Cortex-M4. Physical D11-D12 SPI loopback and controlled A1/A2 ADC board
artifacts remain outstanding, so the plan remains in progress.

The standalone `spi` and `adc` UNO images were then added and cross-built on
17 September 2026. SPI exercises mode 0/MSB at 1 MHz and mode 3/LSB at 6 MHz,
including a dummy-read segment and SPII completion. ADC requests AN001 then
AN000 at both 14- and 12-bit resolution, with five-percent rail thresholds for
A2 tied to 3.3 V and A1 tied to GND. The board runner and aggregate coverage
merge include both suites. Neither unrun artifact is registered in mcutrace.

## PLN-0010 Implement DMAC and peripheral DMA paths
@status(planned)
@link(REQ-0013, REQ-0018, REQ-0019, REQ-0027, REQ-0028, REQ-0029, REQ-0030, REQ-0036, REQ-0041)

Prerequisite: PLN-0006 plus the relevant implementation portions of PLN-0008 and PLN-0009. No fixture prerequisite. Memory-to-memory tests, internal known-answer cases, native-connector traffic and directly wired peripheral paths may be implemented and run immediately; only fixture-observed electrical and fault cases move to PLN-0022.

Implement fixed DMAC ownership and operations using already-reviewed request/memory/width constraints. Add source-confirmed UART/SPI/ADC paths while preserving non-DMA operation. Test quiescence, stale completion, bounds and peripheral-versus-DMA completion ordering. DTC remains a separate driver extension even though its hardware is described.

Exit: documented directions/modes pass native tests, cross-builds and available internal/direct target checks without buffer-lifetime violations. Adapter-dependent observation rows are enumerated for PLN-0022.

## PLN-0011 Accept the first RA4M1 bare-metal driver release
@status(planned)
@link(REQ-0001, REQ-0002, REQ-0015, REQ-0016, REQ-0031, REQ-0035, REQ-0036, REQ-0037, REQ-0038, REQ-0039, REQ-0041, REQ-0042)

Prerequisite: PLN-0005 through PLN-0010, PLN-0021, and the RA4M1 rows of PLN-0022. MCUOS is explicitly not a prerequisite. Implementation plans may finish before the fixture; this release-acceptance plan may not.

Run native contracts, compile examples, cross-build/link checks, physical driver scenarios and measurements. Establish numeric resource/latency budgets from measured results with the user before claiming acceptance. Publish source/coverage identities, API/storage/context rules, flash/debug procedures and reproducible evidence.

Exit: all D04 drivers are accepted on RA4M1 bare metal. Missing hardware tests or resource budgets remain blockers for this release; missing MCUOS code does not. REQ-0041's later three-target portability evidence continues in PLN-0018/PLN-0019.

## PLN-0018 Implement STM32H753 bare-metal backends
@status(done)
@link(REQ-0003, REQ-0004, REQ-0009, REQ-0013, REQ-0014, REQ-0017, REQ-0019, REQ-0021, REQ-0022, REQ-0024, REQ-0025, REQ-0026, REQ-0027, REQ-0028, REQ-0029, REQ-0031, REQ-0033, REQ-0036, REQ-0037, REQ-0041)

Prerequisite: the accepted STM32 descriptions and PLN-0017. Neither the RA4M1 release nor PLN-0021 is a prerequisite. Use NUCLEO-H753ZI MB1364 Rev E and keep board configuration separate from MCU metadata. Fixture connector bindings may be added only after PLN-0021 freezes them, but that does not block peripheral backends or board-native bindings.

1. Inventory the physical board assembly, jumper state and relevant solder bridges. Before changing a bridge, record its schematic designator, original and target state, purpose and restoration procedure.
2. Complete startup, clocks, memory layout, vectors, GPIO and debug procedures using ST-LINK and built-in board features.
3. Implement the common driver set against the existing descriptions. Prefer tests available without the adapter: built-in LEDs/buttons, ST-LINK virtual COM, documented point-to-point jumper/bridge loopbacks, native USB where selected, and the on-board Ethernet PHY/connector with an ordinary network peer. Ethernet scope here is clock/pin setup, MDIO/PHY control, link state and raw MAC-frame exchange, not an implicit TCP/IP stack.
4. Validate cache/MPU behavior and DMA constraints with internal memory/peripheral known-answer tests and native-connector traffic where that gives independent observation.
5. Reuse functional tests and external-device sources with hardware bindings changed. Revalidate all three description contracts if the port exposes a model error. Enumerate, but do not simulate away, the cases that require translated shared buses, calibrated analog stimulus, independent PWM capture, supervised fault injection or the fixture CAN peer.

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

`board/nucleo_h753zi/README.md` records the board/source identity, received
configuration, commands, resource model, observation limits and deferred rows.
PWM waveform measurement, controlled/calibrated ADC levels, real I2C/SPI
targets, peripheral-DMA traffic and supervised fault/electrical cases remain
PLN-0022 acceptance work. A D11-D12 SPI bridge and controlled A0 ties are safe
pre-adapter procedures but were not installed and are recorded as unexecuted,
not simulated evidence. Ethernet and native USB are independent board-native
extensions tracked by PLN-0023 rather than silently folded into this agreed
GPIO/PWM/UART/I2C/SPI/ADC backend exit.

Exit: the agreed STM32 backends pass native tests, Cortex-M7 builds and every applicable board-native/direct-wired procedure; board modifications and deferred adapter rows are documented. Full STM32 electrical acceptance is completed later under PLN-0022. No MCUOS Cortex-M7 port is required.

## PLN-0023 Implement Nucleo native USB and Ethernet connectivity
@status(in_progress)
@link(REQ-0003, REQ-0004, REQ-0009, REQ-0013, REQ-0033, REQ-0036, REQ-0037, REQ-0041, REQ-0042, REQ-0043)

Prerequisite: PLN-0018 startup, memory/cache rules and board binding. This work
is explicitly before and independent of fixture completion. Use the received
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
  repeatable host-side procedures. Route only fixture-supplied fault/electrical
  cases to PLN-0022.

Exit: Ethernet MDIO/PHY/link/raw-frame, the MCUIP protocol smoke path and the
selected native USB smoke path pass repeatable board-native procedures with
source-backed metadata and bounded storage. No lwIP, adapter hardware or MCUOS
dependency is implied.

## PLN-0019 Implement ESP32-P4 bare-metal backends
@status(in_progress)
@link(REQ-0003, REQ-0004, REQ-0009, REQ-0010, REQ-0011, REQ-0013, REQ-0014, REQ-0016, REQ-0017, REQ-0019, REQ-0024, REQ-0025, REQ-0026, REQ-0027, REQ-0028, REQ-0031, REQ-0034, REQ-0036, REQ-0037, REQ-0041)

Prerequisite: PLN-0018 and the already accepted ESP32-P4 descriptions. PLN-0021 is not a prerequisite for implementation or board-native tests; only carrier-specific fixture bindings and deferred electrical acceptance depend on it.

Current ESP32-P4-NANO development evidence uses a custom second-stage loader and a separately linked factory application. The loader owns only initialization, partition validation and the fixed factory-selection policy; `mcutest`, MCUCOV and the functional GPIO/UART/I2C/I2S/audio/SDMMC/SDIO/SPI code reside in the application. ESP-IDF completes image/cache/libc/constructor and heap initialization, then the application takes control before main-task creation, so no FreeRTOS scheduler is started. The 154-case native host suite includes ESP32-P4 I2C timing, matrix routing, write/read/repeated-START command lists, explicit STOP, bounded FIFO validation, address/data NACK attribution, arbitration-loss, timeout, cancellation and recovery; I2S fractional timing, Philips-slot and directional GPIO-matrix routing, AHB-PDMA clock/reset/descriptor/completion, cache-policy, cancellation and ES8311 sequence tests; SD-card protocol initialization, capacity decoding and sector read/write plus native SDMMC clock, dedicated and matrix pin routing, slot/card selection, LDO, command and IDMAC behavior; scheduler-independent SDIO CMD5/RCA/CCCR/CIS/function-enable/block-size/four-bit enumeration; and GPSPI2 timing, mode/order/width/CS configuration, one-command multi-segment ownership, completion, error, cancellation and CPU-buffer bounds. On the connected revision-v1.0 device, seven registered mcutest cases pass: GPIO2 register readback; I2C0 100 kHz register-select/repeated-START reads of the on-board ES8311 codec identity `0x8311` at address `0x18` on GPIO8/GPIO7; I2S0/ES8311 48 kHz capture of 4,032 bytes with 2,016 nonzero analog-microphone samples followed by a completed 193,536-byte, 1,008 ms, 1 kHz DAC tone under GPIO53 amplifier control; native SDMMC identification of the 61,454,336-sector SDHC card plus sector-0 read and last-sector write/read/restore verification on GPIO39-GPIO45; native SDIO slot-1 enumeration of the GPIO14-GPIO19 ESP32-C5 wireless coprocessor after GPIO54 reset, including two advertised functions, RCA `0x0001`, CCCR `0x32`, CIS `0x01000`, Espressif identity `0x0092/0x6666`, function-1 ready, 512-byte block sizes and four-bit mode; one GPSPI2 1 MHz four-frame, two-segment command on expansion GPIO20-GPIO23; and a UART0 finite transfer whose marker is physically received through the on-board CH343 USB bridge after TX-done completion. Revision v1.0 uses different I2C0 APB-gate/reset bits, retained as an explicit development-board binding without altering the production-v3.x description. The MCUCOV FULL-profile capture reports 161/237 functions, 1039/1679 statements, 1169/1782 lines, 453/994 branches and 69/777 MC/DC conditions. The on-board codec provides physical I2C transaction and live microphone/DMA evidence; the test deliberately produces an audible tone, while independent level/frequency/distortion measurement, timing/protocol capture and injected I2C faults remain fixture work. The SDMMC result proves 4-bit physical block I/O and restoration but does not claim filesystem or multi-block support. The C5 result proves the physical SDIO link, identity and function readiness but not ESP-Hosted packet transport, remote Wi-Fi API operation, scanning, association or IP connectivity. UART RX/error stimulus still needs a loopback or fixture. The SPI result proves controller execution and idle completion but, with no external slave, loopback or logic analyzer attached, does not claim returned data or electrical timing/CS-waveform behavior. The v1.0 result remains development-only evidence outside the accepted v3.x silicon range.

Choose the board, toolchain/boot path and initial core ownership. Implement the common driver set against existing Espressif descriptions, validating actual clock, routing, memory/cache and DMA behavior. Keep selected boot support separate from functional drivers and ensure the delivered execution mode does not require an RTOS scheduler. Run common and physical target tests; concurrent multicore/LP support needs separate evidence.

Exit: all three declared bare-metal targets have real backends supporting the shared interfaces, with host/cross-build evidence and every applicable board-native/direct-wired target procedure. Deferred adapter rows are explicit. Description compatibility and runtime portability are separately demonstrated; complete electrical acceptance remains PLN-0022 work.

## PLN-0022 Execute deferred adapter-dependent physical acceptance
@status(planned)
@link(REQ-0021, REQ-0022, REQ-0024, REQ-0025, REQ-0026, REQ-0027, REQ-0028, REQ-0029, REQ-0030, REQ-0036, REQ-0041, REQ-0042)

Prerequisite: PLN-0021 Rev-A electrical qualification and the relevant implemented backend/test image. This plan is intentionally after fixture completion. It owns only the cases that need fixture-provided stimulus or independent observation; board-native results from PLN-0008 through PLN-0010, PLN-0018 and PLN-0019 are inputs and are not repeated without cause.

Execute translated-voltage digital/bus tests, real shared I2C/SPI targets, calibrated ADC/DAC paths, independent timer/PWM capture, supervisor-driven error injection, power/current cases and the CAN/CAN-FD peer scenarios. Run the applicable matrix rows on UNO R4 Minima, NUCLEO-H753ZI and ESP32-P4-NANO. Record fixture revision, firmware, calibration, jumper state, captures, tolerances and failures/ECOs. Keep unavailable instances as explicit gaps.

Exit: every deferred row has passing evidence or an explicit unresolved gap. These results join the earlier implementation and board-native evidence for target/release acceptance; they do not retroactively change what the earlier observations proved.

## Phase F: deferred MCUOS integration

## PLN-0020 Integrate MCUOS after bare-metal driver acceptance
@status(planned)
@link(REQ-0002, REQ-0015, REQ-0016, REQ-0018, REQ-0019, REQ-0031, REQ-0032, REQ-0038, REQ-0041, REQ-0042)

Prerequisite: PLN-0011, PLN-0018, PLN-0019 and the required PLN-0022 target rows. Hardware MCUOS acceptance additionally needs an executable MCUOS target; its absence does not reopen prior bare-metal acceptance.

- Reinspect `../mcuos` when this stage begins and pin the actual source/configuration identity. Its facade and target APIs may have changed since the planning review.
- Add optional MCUOS build integration with an explicit source-path setting. Implement the adapter using task-level facade calls and supplied ISR context, bounded completion transport, absolute deadlines and task-owned bus synchronization.
- Test actual MCUOS native tasks with fake peripherals, covering race boundaries, unrelated notifications, saturation/exhaustion, owner death and cancellation versus hardware quiescence. Preserve the same driver/backend sources as bare metal.
- Complete the first physical MCUOS composition on RA4M1 when context switching, interrupt entry/return, time/alarm and startup support exist. Reserve RTOS-owned timers/vectors and enforce its kernel-callable interrupt ceiling. Track later MCUOS hardware targets independently.

Exit: native MCUOS adapter behavior and the first claimed physical MCUOS target pass their own acceptance checks. No driver rewrite or mandatory RTOS dependency has been introduced.

## Phase G: later extensions

## PLN-0012 Add synchronized motor PWM
@status(planned)
@link(REQ-0004, REQ-0010, REQ-0023, REQ-0036, REQ-0042)

Prerequisite: PLN-0007, PLN-0011 and a PLN-0021 adapter exposing a valid timer/output group. Scheduled after the initial bare-metal release; independent of MCUOS.

Implement grouping, synchronization, complementary output pairs, dead time and fault/disable behavior using already described capabilities. Validate real timer/pin combinations and capture alignment, updates, dead time and fault outputs.

Exit: valid groups pass compile-time and physical waveform checks; invalid groups are rejected.

## PLN-0016 Optional adapters and broader driver modes
@status(planned)
@link(REQ-0002, REQ-0004, REQ-0018, REQ-0020, REQ-0038, REQ-0042)

Prerequisite: the relevant accepted bare-metal driver and explicit selection of an extension. This is a roadmap item, not a commitment to every example.

Evaluate fixed-storage coroutine/callback adapters, generation tooling, DTC drivers, additional bus modes, streaming ADC/UART and drivers for otherwise description-only blocks. Add requirements, source review and acceptance cases before implementing a selected extension.

Exit: each selected extension preserves storage, ownership and portability contracts and has explicit evidence.

## Critical path and evidence boundaries

The path is all-three sources → shared metadata foundation → three incremental complete descriptions → common description gate → completed RA4M1 foundation, then two parallel branches. The implementation branch completes remaining RA4M1 features and the STM32H753/ESP32-P4 backends using host, cross-build, board-native, native-connector and direct-wired evidence. The hardware branch designs, fabricates and qualifies the Rev-A fixture. The branches join at PLN-0022 adapter-dependent physical acceptance and the target release-acceptance plans, followed by MCUOS. The test-fixture coverage matrix, not the Uno header alone, defines final physical coverage, but it does not gate unrelated implementation. MCUOS ports matter only in the later RTOS phase.

Compile-time tests establish representability and reject invalid configurations; they do not verify hardware register behavior, electrical timing or driver portability by themselves. Those claims require subsequent native behavioral tests and independent physical observations. Keep the coverage denominator, verification state and explicit exclusions visible throughout.
