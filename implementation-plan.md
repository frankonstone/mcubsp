# mcuhw implementation plan

Revised baseline, 18 September 2026. Implements [requirements.md](requirements.md). The current sequence is: **preserve the completed three-MCU description and RA4M1 driver foundation; design and electrically qualify reusable instrumentation shields; finish software features and physical acceptance using those fixtures; integrate MCUOS last**. Status annotations below track execution; a completed plan is not itself requirement evidence. Existing plan IDs and evidence are retained. Read in the phase order below, not numeric ID order.

## Delivery phases

| Phase | Work | Exit |
| --- | --- | --- |
| A: description foundation | Sources and coverage inventories for RA4M1, STM32H753 and ESP32-P4; core metadata/MMIO contracts; native and cross-compile probes | Shared model can express real differences without vendor register-layout assumptions |
| B: three complete descriptions | Reviewed block-sized increments across all three, followed by remaining documented blocks | All three declared baselines pass the common description and coverage gate, before any backend/driver implementation |
| C: completed bare-metal foundation | Preserve completed RA4M1 startup, GPIO, execution, transfer, timer and basic-PWM work | Stable fixture-facing baseline; existing evidence remains valid |
| D: instrumentation hardware | Common Arduino shield, ESP32-P4-NANO carrier, STM32 expansion adapter, supervisor firmware and complete feature/peripheral coverage matrix | Rev-A fixtures are electrically safe, calibrated and usable as independent test peers |
| E: remaining bare-metal features | Finish pending RA4M1 physical acceptance, DMA, STM32H753 and ESP32-P4 backends using the fixtures | Common driver contracts and physical bare-metal acceptance for each claimed target |
| F: MCUOS | Optional adapter using `../mcuos`, tested natively and then on an executable MCUOS hardware target | RTOS portability demonstrated independently of earlier bare-metal releases |

Motor PWM and other extensions remain later milestones. MCUOS source, API readiness and hardware ports are not prerequisites for phases A–E. During fixture design and fabrication, work is limited to the fixture, documentation, maintenance and already-scoped host verification; new software feature slices resume after Rev-A electrical bring-up.

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

## Phase D: cross-target instrumentation hardware

This phase takes precedence over the remaining software-feature work. Existing UART/I2C/SPI/ADC code and evidence are preserved, but their unfinished physical acceptance and new DMA/target-port work wait for Rev-A fixture electrical bring-up. Fixture design, procurement and fabrication may overlap only with maintenance, documentation and already-scoped host verification.

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

## Phase E: remaining bare-metal features and acceptance

## PLN-0008 Deliver UART and I2C
@status(planned)
@link(REQ-0024, REQ-0025, REQ-0026, REQ-0027, REQ-0036, REQ-0038, REQ-0041, REQ-0042)

Prerequisite: PLN-0005 and PLN-0006. The implemented host/cross-build work is preserved; remaining physical I2C acceptance resumes after PLN-0021 Rev-A bring-up.

Implement SCI UART/baud solving and IIC timing/transactions against existing metadata. Add finite IRQ-backed operations, write/read/repeated-start semantics, error recovery and a fixed-storage external-device demonstrator. Test blocking/polling use on native models and real UART/I2C fixtures, including errors, timeout and reuse.

Exit: protocol captures, transfer lifetimes and error paths pass on bare metal; the same external-device source runs against fake and real bus bindings.

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
are still absent, so this plan remains planned rather than done.

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
until the physical fixture passes and produces its artifact. Independent logic-
analyzer captures remain required.

## PLN-0009 Deliver SPI and ADC
@status(planned)
@link(REQ-0028, REQ-0029, REQ-0036, REQ-0038, REQ-0041, REQ-0042)

Prerequisite: PLN-0005 and PLN-0006. Independent of PLN-0008 once shared contracts stabilize. The implemented host/cross-build work is preserved; remaining physical SPI/ADC acceptance resumes after PLN-0021 Rev-A bring-up.

Implement SPI master behavior, segmented chip-select ownership, dummy reads and final-wire completion. Implement ADC single/finite-sequence conversion with validated channels/reference/sample timing and raw-result ordering. Exercise native failure/configuration cases and physical SPI/ADC fixtures with controlled ADC inputs.

Exit: both drivers pass their bare-metal configuration, transfer and cleanup acceptance cases.

On 17 September 2026, the portable SPI/ADC contracts and RA4M1 backends were
implemented. Native evidence covers SPI rate selection, mode/order/width
programming, segmented dummy/full-duplex transfers, GPIO-held chip select,
SPII/IDLNF final-wire completion, fault and timeout cleanup; ADC evidence
covers channel/sample validation, 12/14-bit setup, finite scans, raw result
ordering, rejection and timeout cleanup. The complete example cross-compiles
for Cortex-M4. Physical D11-D12 SPI loopback and controlled A1/A2 ADC board
artifacts remain outstanding, so the plan remains planned rather than done.

The standalone `spi` and `adc` UNO images were then added and cross-built on
17 September 2026. SPI exercises mode 0/MSB at 1 MHz and mode 3/LSB at 6 MHz,
including a dummy-read segment and SPII completion. ADC requests AN001 then
AN000 at both 14- and 12-bit resolution, with five-percent rail thresholds for
A2 tied to 3.3 V and A1 tied to GND. The board runner and aggregate coverage
merge include both suites. Neither unrun artifact is registered in mcutrace.

## PLN-0010 Implement DMAC and peripheral DMA paths
@status(planned)
@link(REQ-0013, REQ-0018, REQ-0019, REQ-0027, REQ-0028, REQ-0029, REQ-0030, REQ-0036, REQ-0041, REQ-0042)

Prerequisite: PLN-0006, PLN-0008, PLN-0009 and PLN-0021 Rev-A bring-up. Standalone DMAC design review and host cases may be prepared earlier, but new target feature implementation resumes in this phase.

Implement fixed DMAC ownership and operations using already-reviewed request/memory/width constraints. Add source-confirmed UART/SPI/ADC paths while preserving non-DMA operation. Test quiescence, stale completion, bounds and peripheral-versus-DMA completion ordering. DTC remains a separate driver extension even though its hardware is described.

Exit: documented directions/modes pass physical DMA and peripheral checks without buffer-lifetime violations.

## PLN-0011 Accept the first RA4M1 bare-metal driver release
@status(planned)
@link(REQ-0001, REQ-0002, REQ-0015, REQ-0016, REQ-0031, REQ-0035, REQ-0036, REQ-0037, REQ-0038, REQ-0039, REQ-0041, REQ-0042)

Prerequisite: PLN-0005 through PLN-0010 and PLN-0021. MCUOS is explicitly not a prerequisite.

Run native contracts, compile examples, cross-build/link checks, physical driver scenarios and measurements. Establish numeric resource/latency budgets from measured results with the user before claiming acceptance. Publish source/coverage identities, API/storage/context rules, flash/debug procedures and reproducible evidence.

Exit: all D04 drivers are accepted on RA4M1 bare metal. Missing hardware tests or resource budgets remain blockers for this release; missing MCUOS code does not. REQ-0041's later three-target portability evidence continues in PLN-0018/PLN-0019.

## PLN-0018 Implement STM32H753 bare-metal backends
@status(planned)
@link(REQ-0003, REQ-0004, REQ-0009, REQ-0013, REQ-0031, REQ-0033, REQ-0036, REQ-0037, REQ-0041, REQ-0042)

Prerequisite: PLN-0011, PLN-0021 and the already accepted STM32 descriptions. The exact STM32 board/revision and shield-plus-morpho mapping are frozen by PLN-0021 before backend pin bindings are added.

Choose the physical board and boot configuration, then implement the common driver set against its existing descriptions. Validate clocks, memory/cache/MPU and DMA constraints in execution. Reuse the functional tests and external-device source with hardware bindings changed. Revalidate all three description contracts if the port exposes a model error.

Exit: shared functional interfaces pass the STM32 bare-metal support matrix and physical tests. No MCUOS Cortex-M7 port is required.

## PLN-0019 Implement ESP32-P4 bare-metal backends
@status(planned)
@link(REQ-0003, REQ-0004, REQ-0009, REQ-0013, REQ-0016, REQ-0031, REQ-0034, REQ-0036, REQ-0037, REQ-0041, REQ-0042)

Prerequisite: PLN-0018, PLN-0021 and the already accepted ESP32-P4 descriptions.

Choose the board, toolchain/boot path and initial core ownership. Implement the common driver set against existing Espressif descriptions, validating actual clock, routing, memory/cache and DMA behavior. Keep selected boot support separate from functional drivers and ensure the delivered execution mode does not require an RTOS scheduler. Run common and physical target tests; concurrent multicore/LP support needs separate evidence.

Exit: all three declared bare-metal targets have real backends supporting the shared interfaces and independent hardware evidence. Description compatibility and runtime portability are now separately demonstrated.

## Phase F: deferred MCUOS integration

## PLN-0020 Integrate MCUOS after bare-metal driver acceptance
@status(planned)
@link(REQ-0002, REQ-0015, REQ-0016, REQ-0018, REQ-0019, REQ-0031, REQ-0032, REQ-0038, REQ-0041, REQ-0042)

Prerequisite: PLN-0011, PLN-0018 and PLN-0019. Hardware MCUOS acceptance additionally needs an executable MCUOS target; its absence does not reopen prior bare-metal acceptance.

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

The path is all-three sources → shared metadata foundation → three incremental complete descriptions → common description gate → completed RA4M1 foundation → cross-target fixture design and Rev-A electrical qualification → remaining RA4M1 features and acceptance → STM32H753 and ESP32-P4 backends → MCUOS. The test-fixture coverage matrix, not the Uno header alone, defines how every peripheral instance and board feature is exercised. MCUOS ports matter only in the later RTOS phase.

Compile-time tests establish representability and reject invalid configurations; they do not verify hardware register behavior, electrical timing or driver portability by themselves. Those claims require subsequent native behavioral tests and independent physical observations. Keep the coverage denominator, verification state and explicit exclusions visible throughout.
