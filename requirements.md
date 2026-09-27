# mcuhw requirements

Requirements baseline, 26 September 2026. Derived from [proposal.md](proposal.md), the supplied manuals, the local MCUOS and MCUIP projects, and the user's decisions below. All eighteen scope decisions are recorded. The first RA4M1 release is accepted using the connected UNO R4 Minima with direct-wired GPIO/PWM/UART, Grove I2C, Ethernet-shield SPI/DMAC and potentiometer ADC evidence. Subsequent decisions add UNO Ethernet through that shield, a modern C++ application interface, reusable attached devices, and repository/build workflows for broad adoption at hundreds of boards and thousands of devices. NUCLEO-H753ZI remains the STM32 carrier and MCUIP is the selected TCP/IP stack. This document specifies intended behavior; it does not claim implementation or verification. Detailed integration choices and measured resource budgets remain implementation milestones. The proposal's code snippets are illustrative API sketches, not verified register definitions or compilable specifications.

## Decisions and scope

| ID | Decision | State |
| --- | --- | --- |
| D01 | Describe RA4M1, STM32H753 and ESP32-P4 and validate the shared model before implementing drivers. Preserve the completed RA4M1 foundation. After the description gate, continue driver/backend implementation and board-native verification using direct wiring, attached devices, native connectors, ordinary peers and internal known-answer tests. | Revised by user |
| D02 | Complete documented hardware descriptions, delivered in reviewed increments across all three MCUs during the description phase. Describe the initial peripheral set and dependencies first, then remaining documented blocks; validate all three before driver work. | Revised by user |
| D03 | Bare metal first. Integrate MCUOS from `../mcuos` in a later stage after the bare-metal driver milestones; MCUOS is not a dependency of the description or bare-metal phases. | Revised by user |
| D04 | First RA4M1 bare-metal driver release includes GPIO, timer/PWM, I2C, UART, SPI, ADC, and DMA. | Accepted by user; release clarified |
| D05 | Core and drivers prohibit heap allocation, exceptions, and RTTI; use fixed storage and explicit errors. | Accepted by user |
| D06 | Author reviewed C++23 hardware descriptions directly; generation is optional future work. | Accepted by user |
| D07 | Basic PWM in the first release; synchronized motor PWM with complementary outputs and dead time is a later milestone. | Accepted by user |
| D08 | Keep the library independent; provide a minimal standalone UNO example and allow existing RTOS/BSP startup. | Accepted by user |
| D09 | Accept the three-MCU description gate with ESP32-P4 I3C exactly excluded because TRM v0.7 publishes no authoritative chapter content. Keep the unit in the denominator and re-open the gate when a source revision supplies it. | Accepted by user's request to implement PLN-0013/PLN-0017 after the disclosed PLN-0015 source gap |
| D10 | Develop a reusable KiCad cross-target test-fixture family. | Superseded by D14; no longer required |
| D11 | Use the proposed Rev-A component families as a fixture schematic baseline. | Superseded by D14; no longer required |
| D12 | Classify target work by its observation hardware and defer fixture-dependent electrical cases. Direct wiring, attached devices, native connectors, ordinary peers and internal known-answer tests remain valid only for the behavior they observe. | Revised by D14 |
| D13 | Use the sibling `../mcuip` project as the TCP/IP stack for mcuhw Ethernet targets and keep the MAC/MDIO/PHY/DMA boundary compatible with its public target contracts. MCUIP replaces lwIP; no lwIP dependency or compatibility layer is required. | Accepted by user |
| D14 | Retire the cross-target fixture requirement. For the first RA4M1 release, accept representative physical evidence from the connected UNO R4 Minima: direct GPIO/PWM/UART wiring, the Grove I2C device, W5100 Ethernet-shield SPI/DMAC traffic and a potentiometer ADC input. Do not claim unmeasured electrical properties, fault injection or complete-board coverage. Other targets use their own board-native/directly connected acceptance evidence when their releases are claimed. | Accepted by user, 22 September 2026 |
| D15 | Add Ethernet connectivity on Arduino UNO R4 Minima through the connected Arduino Ethernet Shield R3 and its WIZnet W5100. Use the existing RA4M1 SPI/DMAC foundation, expose a fixed-storage raw-frame device to MCUIP, and prove real traffic on the local network. Treat this as a follow-on milestone; it does not reopen PLN-0011. | Accepted by user, 22 September 2026 |
| D16 | Add `mcuhw::io` in this repository as a modern C++ application interface with familiar board/pin/peripheral operations, explicit DMA access, replaceable execution, shared ownership and compile-time storage selection. Implement the bare-metal foundation now. Camera, RTOS and network integration are future extensions, and Arduino sketch compatibility is outside this increment. Measure actual RAM/flash cost against the existing driver layer. | Accepted by user, 26 September 2026 |
| D17 | Introduce reusable attached-device drivers with shared I2C address and SPI chip-select handles supplied by `io`. Keep chip protocols independent of board composition, preserve checked fast transfers, and supply TMP102 and W5100 components plus generic MCUIP integration. Use explicit storage, ownership, deadlines and bounded polling; RTOS concurrency remains a later stage. | Accepted by user, 26 September 2026 |
| D18 | Make ease of use, building and contribution a first-class adoption requirement for hundreds of boards and thousands of devices. Keep one modular repository, independently consumable CMake/header interfaces, locally defined hardware components, selected-board builds, reproducible setup, searchable support information and dependency-scoped verification. Schedule the repository adoption work immediately after PLN-0026, before deferred MCUOS integration; preserve existing hardware and evidence boundaries. | Accepted for requirements and planning by user, 26 September 2026 |
| D19 | Configure the architecture once and expose board task groups only for a configurable active-board set. Persist the local selection, keep examples as separate firmware images beneath an aggregate, and isolate board/revision artifacts and optional test dependencies. | Accepted by user, 26 September 2026 |

Existing proposal decisions are retained: C++23; compile-time configuration and validation; metadata separate from drivers; functional abstraction with capabilities; RA4M1 as the first physical driver target after all three descriptions; `transfer<T>` as the async primitive; optional coroutine adapters.

Engineering defaults for the driver phase: fixed ownership of peripherals; no hidden transfer queue; runtime duty updates; I2C controller/master mode with 7-bit addressing; SPI controller/master mode; UART asynchronous serial; ADC software-triggered conversion and sequences; DMA for explicitly supported peripheral paths. Slave/target bus modes, ten-bit I2C, advanced motor PWM, and other advanced modes are separate extensions unless a decision below adds them. These defaults define a reviewable initial scope rather than claiming every hardware mode is implemented.

## Sources and limits

| Source | Inspected scope | Consequence |
| --- | --- | --- |
| [RA4M1 hardware manual](doc/_vendor/manuals/r01uh0887ej0110-ra4m1.pdf), R01UH0887EJ0110 Rev.1.10, 29 September 2023, 1,454 pages | Overview; contents; I/O ports; GPT overview, functions and output semantics | Primary starting source for RA4M1. Review relevant register tables and usage notes during each implementation slice. |
| [PM0253](doc/_vendor/manuals/pm0253-stm32f7-series-and-stm32h7-series-cortexm7-processor-programming-manual-stmicroelectronics.pdf), Rev.6, May 2026, 254 pages | Scope and core architecture contents | Covers the Cortex-M7 processor and core peripherals. It is insufficient to describe STM32H753 peripheral registers, clocks, pins, or package variants. It does not add all STM32F7/H7 devices to project scope. |
| [ESP32-P4 technical reference manual](doc/_vendor/manuals/esp32-p4_technical_reference_manual_en.pdf), preliminary pre-release v0.7, 3,701 pages | Release-status table, system/memory overview and contents | Treat as a versioned preliminary source. Its I3C chapter is explicitly incomplete; never invent missing definitions. Model HP/LP domains and core ownership explicitly. |
| [RM0433](doc/_vendor/manuals/rm0433-stm32h742-stm32h743753-and-stm32h750-value-line-advanced-armbased-32bit-mcus-stmicroelectronics.pdf), Rev.8, January 2023, 3,353 pages | Document identity, scope, page count and first/last-page readability checked | Present locally. Primary reference for STM32H753 memory and peripheral descriptions; detailed register review remains part of the description phase. |
| [DS12117: STM32H753xI](doc/_vendor/manuals/stm32h753xi.pdf), Rev.10, January 2026, 357 pages | Document identity, device family, page count and first/last-page readability checked | Present locally. Use for device/package variants, pins, alternate functions and electrical limits. |
| [ES0392](doc/_vendor/manuals/es0392-stm32h742xig-stm32h743xig-stm32h750xb-stm32h753xi-device-errata-stmicroelectronics.pdf), Rev.15, September 2025, 73 pages | Document identity, applicability table, page count and first/last-page readability checked | Present locally. Review applicable silicon limitations and documentation corrections alongside RM0433 and DS12117. |
| [Arduino UNO R4 Minima datasheet](https://docs-content.arduino.cc/resources/datasheets/ABX00080-datasheet.pdf) and [board documentation](https://docs.arduino.cc/hardware/uno-r4-minima) | Official board identification checked during planning | Target part is R7FA4M1AB3CFM#AA0. Pin the schematic, pinout, oscillator configuration, and boot assumptions before board implementation. |
| [NUCLEO-H753ZI product page](https://www.st.com/en/evaluation-tools/nucleo-h753zi.html) and MB1364-H753ZI-E01 board schematic | Official board family and expansion connectors checked during board planning | Selected physical STM32 board is MB1364 revision E. Use the E01 schematic as the mapping baseline and cross-check assembly options and jumper state against the board. Documented solder-bridge/jumper changes and native debug, USB and Ethernet connections may be used for target-specific evidence. |
| [ESP32-P4-NANO documentation](https://docs.waveshare.com/ESP32-P4-NANO) and vendor schematic/pinout | Official board features and connector families checked during board planning | Baseline ESP32-P4 board. Use its native USB, Ethernet, TF/SDIO, MIPI, audio and RTC interfaces plus direct connections on the two 2x13 GPIO headers for target-specific evidence. |

The [source manifest](doc/reference/source-manifest.json) records all six user-supplied PDFs, their revisions, page counts, byte sizes and SHA-256 checksums. The three previously missing STM32 documents are now available. At implementation time, check applicable vendor corrections and pin the reviewed revisions; do not silently replace the supplied source baseline. Readability checks do not constitute a register-by-register audit or establish that every vendor document is the latest revision.

[Searchable Markdown references](doc/_generated/manuals/README.md) provide the extracted text of all six manuals in page ranges, with PDF bookmark navigation and links to each original page. Use these copies to locate relevant sections efficiently. They preserve the embedded text rather than reconstructing table columns or diagrams; verify hardware values, bit positions, pin mappings and ambiguous layouts in the PDFs. [Conversion metadata](doc/_generated/manuals/conversion-manifest.json) ties generated files to the pinned source hashes. Regenerate with `python3 tools/convert_manuals.py` when a reviewed source baseline changes.

The RA4M1 manual, section 22.1, tables 22.1–22.2 (pages 392–393), describes two GPT32 and six GPT16 timer instances, each with GTIOCA/GTIOCB output pins. The proposal's three outputs on `gpt0` cannot be used as a valid three-phase mapping. Timer instances and output identities must be separate types; multi-phase output requires a validated combination of instances and pins. Section 19 also makes pin availability package-dependent.

### Deferred MCUOS integration baseline

For the later RTOS stage, the selected dependency is the sibling [MCUOS project](../mcuos/README.md). No MCUOS adapter, native RTOS test, source checkout or executable hardware port is required to complete descriptions or bare-metal drivers. The inspected [application facade](../mcuos/include/mcuos/rtos/rtos.hpp) provides `System::now()`, `wait_notification_until()`, task notification, bounded queues, mutexes, and an `IsrContext` with nonblocking notification/queue operations. Its time types use 64-bit nanoseconds. Task code should consume this facade; scheduler transitions and split-phase kernel waits belong inside MCUOS and its target integration.

The [port contracts](../mcuos/docs/port-contracts.md) and [wait/interrupt rules](../mcuos/docs/waits-and-interrupts.md) define saved interrupt-mask restoration, a kernel-callable interrupt ceiling, and scheduling deferred until interrupt return. These rules constrain the mcuhw adapter and target composition.

The current source tree contains native execution, while the README identifies Cortex-M execution as next target work. A Cortex-M compile preset is not an executable RA4M1 RTOS port. Within the later MCUOS stage, native adapter tests precede hardware RTOS acceptance, which additionally requires context switching, physical interrupt delivery, monotonic clock/alarm, and boot integration for RA4M1. This remains an explicit dependency on MCUOS target work.

The inspected facade currently accepts an interrupt-priority argument, while [MCUOS API hardening](../mcuos/req/07_API_Hardening.md) plans target-derived priority and other facade changes. Bind to the actual dependency snapshot, isolate compatibility in the adapter/target composition, and do not assume planned APIs exist. The sibling directory currently has no Git revision available; integration evidence shall record a source-content identity and configuration until a versioned revision is available.

### MCUIP Ethernet integration baseline

The selected TCP/IP dependency is the sibling [MCUIP project](../mcuip/README.md),
which replaces lwIP. Its current public target boundary defines
`mcuip::NetworkDevice`, `mcuip::DescriptorProtection`,
`mcuip::HardwareDescription`, fixed receive/transmit rings and explicit
completion ownership. The NUCLEO-H753ZI is also MCUIP's first specified physical
Ethernet target. Keep the protocol-neutral mcuhw MAC/MDIO/PHY/DMA implementation
separate from the MCUIP adapter, bind against the actual sibling headers, and
record a source-content identity and configuration because `../mcuip` currently
has no Git revision available.

## Product and architecture

## REQ-0001 Description-first, bare-metal delivery sequence

mcuhw shall first implement hardware descriptions for RA4M1, STM32H753 and ESP32-P4 and satisfy the common description-validation gate in REQ-0040. No functional driver or peripheral backend shall be implemented before that gate. Description work shall proceed in reviewed increments across all three MCUs, exposing differences before the common model is stabilized.

After that gate, implement and validate drivers on bare metal, with RA4M1 / Arduino UNO R4 Minima as the first physical target and STM32H753/ESP32-P4 following with the same functional interfaces. MCUOS integration shall follow the bare-metal driver milestones and shall not block them. A native test platform shall support each phase without physical hardware.

Acceptance: delivery records distinguish description validation, bare-metal driver support and later MCUOS support. Every target/peripheral is tracked as described, host-tested, cross-compiled and hardware-tested independently; an empty directory or matching type names do not establish support.

## REQ-0002 C++23 and restricted runtime

The public library shall use C++23 and support builds with exceptions and RTTI disabled. Core, backends, and drivers shall not allocate dynamically, require dynamic initialization for hardware descriptions, or depend on a hosted operating system. Operation and buffer storage shall have documented fixed bounds and ownership. Optional adapters shall preserve this contract when included in a supported embedded configuration.

Acceptance: representative linked firmware has no heap-allocation, exception-runtime, or RTTI dependency; configuration metadata requires no startup constructors. Build reports include the exact compiler, target flags, and library versions.

## REQ-0003 Layer boundaries
@depends(REQ-0002)

The library shall separate core types, CPU architecture support, MCU descriptions, peripheral backends, functional drivers, board composition, and execution-environment adapters. `mcu/*` shall contain descriptions and declarative traits only. Backends shall contain register sequencing for a peripheral IP family. Functional drivers shall contain no MCU addresses or dependencies on a vendor register layout. External-device drivers shall depend on functional bus/device contracts only.

Acceptance: before driver work, all three real MCU descriptions satisfy the shared metadata contracts without starting hardware or including an RTOS. During the subsequent driver phase, all three backends demonstrate reuse of functional operations and external-device drivers through binding changes. Description validation proves representability; runtime portability is verified separately when drivers exist.

## REQ-0004 Capabilities and unsupported operations
@depends(REQ-0003)

Capabilities shall describe the selected instance and mode, including FIFO, DMA directions, pin signals, timer width, alignment, and advanced output support. Compile-time requests for absent capabilities shall fail with relevant diagnostics; runtime requests shall return a typed unsupported/configuration error without silently degrading behavior.

Acceptance: positive and negative examples cover capability differences between instances and between targets, including an invalid third output on an RA4M1 GPT instance.

## Hardware descriptions

## REQ-0005 Description identity and provenance

Each MCU description shall identify vendor, family, supported device/package variants and revision assumptions. Every register block, route table, clock relationship, interrupt/event mapping, and capability set shall reference a source revision and section/table. Unresolved source conflicts shall be recorded explicitly and shall prevent support claims for affected functionality.

Acceptance: a maintainer can locate the authoritative source for any delivered definition. A source manifest records document identifiers, revisions, local filenames or URLs, and checksums for locally pinned inputs.

## REQ-0006 Incremental complete description coverage
@depends(REQ-0005)

Each of the three targets shall have an inventory covering every documented block, register, field and relevant route for its declared device/package/source baseline, including peripherals without planned drivers. Track description coverage separately from driver coverage; distinguish unreviewed, described, verified, deferred, unavailable-in-source and not-applicable.

Deliver descriptions in block-sized increments. Begin with identity/memory, clocks/reset, pins, interrupts/events, DMA and the GPIO/timer/PWM/I2C/UART/SPI/ADC blocks across all three MCUs; then complete the remaining documented blocks during the description phase. Do not complete an RA4M1 driver first and use later targets merely to confirm its design. Full description coverage does not require functional drivers for every peripheral.

Acceptance: before the driver phase, every available documented item in each declared baseline is described and reviewed or explicitly not applicable. Missing source material is recorded separately, never counted as verified, and handled under the gate in REQ-0040. Each coverage report names its denominator, revision, variant/package scope and outstanding source gaps.

## REQ-0007 Register and field metadata
@depends(REQ-0005)

Descriptions shall represent addresses/offsets, permitted access widths and alignment, register/field widths, reset values and known masks, reserved-bit rules, field encodings, access semantics, aliases, and protection requirements. Instance traits shall expose multiple clock inputs, reset controls, IRQ/event sources, DMA requests, and capabilities where applicable; the proposal's single IRQ/clock sketch is not a universal constraint.

Acceptance: representative 8-, 16-, and 32-bit registers, unknown reset values, aliases, and mixed-access fields are representable without a fabricated common register layout.

## REQ-0008 Safe MMIO operations
@depends(REQ-0007)

The register API shall enforce legal reads/writes and bit encodings. It shall distinguish ordinary read/write fields from write-one-to-clear, write-zero-to-clear, read-clear, write-only commands, and other documented side effects. Generic read-modify-write shall be unavailable unless safe for that register. Reserved-bit preservation or fixed-write requirements shall follow the manual. Field encoding shall reject overflow, invalid position/width, and invalid enumerators; full-width masks shall avoid undefined shifts.

Acceptance: compile-fail cases reject illegal operations; an instrumented MMIO backend verifies exact access widths, values, order, and absence of unwanted reads for side-effecting registers.

## REQ-0009 Ordering and protected access
@depends(REQ-0008)

Architecture support shall provide the barriers and ordering operations needed by backends. The implementation shall not treat C++ `volatile` as inter-thread synchronization, a CPU barrier, or cache coherency. Protected-register sequences shall use documented unlock/write/relock ordering and shall preserve shared register state under the selected execution policy.

Acceptance: operation traces cover RA4M1 protection sequences; target disassembly checks required barrier instructions. Concurrent access tests cover the software synchronization contract separately from MMIO volatility.

## REQ-0010 Pin, package, and board model
@depends(REQ-0007)

Pins and signals shall be typed. Compile-time validation shall reject nonexistent package pins, unroutable signals, conflicting functions, and unsupported electrical modes. Board aliases, reserved pins, oscillators, and external wiring shall live outside MCU metadata. A board-wide composition shall detect duplicate exclusive pin assignments within that composition; the API shall not imply it can discover unrelated declarations automatically.

Acceptance: valid UNO bindings compile; unavailable pins, invalid PFS selections, and duplicate assignments fail. Board examples use verified schematic/pinout mappings instead of proposal placeholders.

## REQ-0011 Clock and reset model
@depends(REQ-0007, REQ-0009)

Clock descriptions shall model sources, domains, dividers, constraints, peripheral gates/module-stop controls, reset dependencies, and protection. Compile-time configuration shall calculate effective frequencies and validate source/domain limits. Backends shall apply valid startup ordering and bounded stabilization waits. Shared clock controls shall not be disabled while another owner needs them. Drivers shall not silently change global clocks or rely on stale frequencies.

Acceptance: invalid clock plans fail compilation; host traces verify unlock/enable/stabilize/configure sequences and timeout cleanup. The initial release may keep the system clock fixed after initialization; any clock change requires explicit quiescence and reconfiguration.

## REQ-0012 Interrupts and event routing
@depends(REQ-0007, REQ-0009)

Descriptions shall distinguish peripheral event sources from CPU interrupt lines and model configurable routing where present. Integration shall explicitly bind handlers and preserve RTOS-owned core exceptions, priority rules, and vector ownership. Backends shall acknowledge events using correct side-effect semantics and perform bounded ISR work.

Acceptance: routing/conflict tests and target tests verify completion delivery, spurious events, acknowledgement, and absence of implicit takeover of SysTick, PendSV, or other environment-owned facilities.

## REQ-0013 DMA and memory metadata
@depends(REQ-0007)

Descriptions shall specify DMA engines, channels/requests, supported transfer widths, alignment, count limits, reachable memory, and peripheral handshakes. Architectures/targets shall expose cache maintenance and memory-order requirements. RA4M1 DMAC and DTC shall remain distinct mechanisms; a capability for one shall not imply support for the other.

Acceptance: invalid static channel/request/width combinations fail compilation; runtime buffer checks reject inaccessible or misaligned regions before starting a transfer. Later cached/multicore targets have explicit policies before DMA support is claimed.

## Execution and resource contracts

## REQ-0014 Explicit peripheral ownership
@depends(REQ-0003, REQ-0010, REQ-0012)

Each peripheral instance shall have one configured owner, with state stored explicitly in fixed storage. Shared bus access shall be serialized through that owner. Busy hardware or exhausted operation storage shall produce an explicit busy/resource error. There shall be no hidden unbounded queue or implicit reconfiguration by a second client. Static convenience APIs may wrap a unique instance but shall not obscure its ownership.

Acceptance: simultaneous clients, double initialization, duplicate resources, and exhaustion have deterministic outcomes without corrupting an active operation.

## REQ-0015 Portable execution policy
@depends(REQ-0014)

Generic drivers and transfers shall use a documented environment interface for monotonic time/deadlines, critical sections or synchronization, completion notification, and waiting. Bare-metal and RTOS integrations shall use the same functional driver/backend implementation. OS headers and scheduler primitives shall remain within adapters. No scheduler, task, or global event loop shall be required by the core.

Acceptance: the bare-metal driver phase exercises native fake and bare-metal policies without MCUOS. In the later RTOS phase, the MCUOS adapter exercises the same contracts without driver-source changes. The early execution-policy design shall preserve the adaptation boundary without implementing MCUOS prematurely.

## REQ-0016 Concurrency, waiting, and ISR context
@depends(REQ-0012, REQ-0015)

The execution contract shall define task/main-loop versus ISR-callable operations. Blocking waits shall be forbidden in ISR context and shall not hold a critical section needed for completion. Wait/notify implementations shall prevent lost wakeups, including completion between checking readiness and sleeping. Bare-metal polling shall make progress with documented interrupt requirements; optional WFI shall be used only with a race-safe wake protocol. RTOS waits shall block the caller according to the adapter contract and obey ISR notification/priority restrictions.

Acceptance: deterministic interleaving tests cover completion before registration, during registration, before sleep, and during wake. SMP targets shall initially declare single-core ownership or supply validated inter-core synchronization; local interrupt masking alone is insufficient.

## REQ-0017 Transfer object and completion
@depends(REQ-0002, REQ-0014, REQ-0015)

`transfer<T>` shall be the common representation of an asynchronous operation, supporting readiness inspection, nonblocking result inspection/consumption, and waiting with an explicit deadline. Terminal states shall distinguish success, failure, timeout, and cancellation. Results shall use explicit typed errors, not exceptions. State publication shall make completed data visible before readiness is observed. A result shall not be read as successful before completion or consumed ambiguously more than once.

Acceptance: one operation state machine is exercised through blocking and polling use. Tests cover `transfer<void>`, value results, pending inspection, error completion, repeated inspection, and single consumption.

## REQ-0018 Transfer storage and lifetime
@depends(REQ-0017)

The operation state and referenced buffers shall remain valid until hardware, DMA, and completion handlers are quiescent. Handles shall be move-only or otherwise enforce unique result ownership; moving a handle shall not relocate storage used by an ISR. Asynchronous external-device helpers shall not return an operation referencing a destroyed local array. Destruction/abandonment shall have an explicit bounded policy that cannot silently leave hardware writing through dangling references.

Acceptance: lifetime/move/abandonment tests and documented examples demonstrate safe caller-owned or operation-owned fixed storage. Select and document the precise destruction policy before implementing transfers; it may reject active destruction or synchronously perform bounded quiescence, but must not silently detach borrowed storage.

## REQ-0019 Deadlines, cancellation, and recovery
@depends(REQ-0017, REQ-0018)

Operations and hardware stabilization waits shall have explicit bounded deadlines using an environment-supplied monotonic clock. Timeout and cancellation shall quiesce relevant requests/interrupts, preserve a single terminal result, and release ownership only after hardware can no longer access buffers. Late IRQs shall not complete a subsequent operation. Recoverable failure shall permit a defined subsequent operation; failed recovery shall leave the peripheral in a documented faulted state.

Acceptance: fake-time tests cover timeout boundaries, clock wrap handling, completion/cancel races, late IRQs, stuck status flags, and successful reuse or explicit fault after recovery. No path waits forever for a hardware bit.

## REQ-0020 Optional completion adapters
@depends(REQ-0017, REQ-0018)

The transfer contract shall allow future callback/event and coroutine adapters without requiring ordinary callers to use coroutines. User callback or coroutine resumption context shall be explicit; arbitrary application code shall not run in an ISR by default. Any callback registrations and coroutine frames used by supported adapters shall have fixed storage and safe cancellation/lifetime behavior.

Acceptance: the first release works with the adapter headers excluded. Actual coroutine/callback adapters are a later deliverable, not a first-release dependency.

## First bare-metal release functional drivers

## REQ-0021 GPIO
@depends(REQ-0008, REQ-0010, REQ-0011, REQ-0014, REQ-0040)

GPIO shall provide validated input/output configuration, read, write, and set/clear operations, exposing supported pull and drive options through capabilities. Output initialization shall establish the intended level before enabling output where the hardware permits. Shared-port updates shall use atomic set/clear facilities or a documented synchronization strategy; toggle shall not imply atomicity unless provided.

Acceptance: native register traces and UNO loopback/LED tests verify pin direction, levels, supported electrical options, and preservation of adjacent pins.

## REQ-0022 Timer and basic PWM
@depends(REQ-0010, REQ-0011, REQ-0014, REQ-0040)

Timer/PWM shall expose typed instances and real output identities, validated configuration, start/stop, timer event notification, and runtime duty updates. Compile-time solving shall use actual instance widths and legal prescalers, calculate period/compare and achieved frequency, and enforce an explicit acceptable error. Arithmetic shall be range-checked. PWM shall define zero/full-duty, update-latching, initial, and stopped-output behavior.

Acceptance: boundary tests cover zero/invalid frequency, 16-/32-bit counters, prescaler limits, duty endpoints, rounding, and unattainable tolerances. Hardware captures verify representative frequencies, duty changes, and stop behavior. The example's `20_kHz` target is subject to solver validation, not assumed exact.

## REQ-0023 Advanced PWM extension
@depends(REQ-0022, REQ-0004)

In a milestone after the first RA4M1 release (D07), grouped PWM shall validate synchronization across actual timer instances, complementary output relationships, shared timebase constraints, dead-time encoding, and documented fault/disable behavior. Unsupported combinations shall fail at configuration time. A phase shall not be modeled as a nonexistent third GPT output.

Acceptance: compile-time positive/negative group configurations and hardware captures verify alignment, complementary edges, dead time, synchronized updates, and fault/stop outputs. Advanced PWM is not a first-release acceptance dependency.

## REQ-0024 I2C transactions and timing
@depends(REQ-0010, REQ-0011, REQ-0017, REQ-0019, REQ-0040)

The first I2C driver shall provide controller/master 7-bit addressed write, read, and combined write/repeated-start/read transactions with explicit STOP semantics. Addresses shall be unshifted and validated, with reserved addresses rejected by default unless an explicit supported operation permits them. Timing solving shall use actual source clocks, hardware encodings, and required rise/fall/filter assumptions; it shall report achieved timing and reject configurations outside required limits.

Acceptance: trace tests and logic-analyzer captures verify addressing, ACK/NACK, repeated START without an intervening STOP, last-byte handling, and representative 100/400 kHz configurations where validated. Empty and excessive lengths shall have explicit behavior. Ten-bit and target/slave operation are deferred extensions.

## REQ-0025 I2C failures and shared devices
@depends(REQ-0024, REQ-0014)

I2C shall distinguish address/data NACK, arbitration loss, busy/stuck bus, timeout, and transport faults where observable. A combined transaction shall hold bus ownership across repeated START. Recovery shall be bounded and shall only manipulate lines or reset hardware under an explicitly supported ownership/topology policy; arbitration loss shall not trigger disruptive automatic bus recovery.

Acceptance: injected errors verify correct STOP/abort behavior, single completion, ownership retention/release, and recovery before the next transaction.

## REQ-0026 I2C devices and external-device independence
@depends(REQ-0024, REQ-0018)

`i2c_device<Bus, Address>` shall bind an address without duplicating peripheral ownership. Register helpers shall explicitly define register-address width, byte order, and transfer semantics. A demonstrator external-device driver shall depend only on the bus/device contract and have a safe fixed-storage async path.

Acceptance: the same demonstrator runs against a fake bus and the RA4M1 binding; later ports reuse it without MCU-specific conditionals. Tests verify byte ordering and buffer lifetimes.

## REQ-0027 UART
@depends(REQ-0010, REQ-0011, REQ-0017, REQ-0019, REQ-0040)

UART shall support validated baud, data bits, parity, and stop bits within instance capabilities; finite-buffer transmit and receive operations; polling/blocking consumption and interrupt completion; and explicit overrun, framing, parity, and timeout errors. Baud solving shall report achieved baud and enforce a requested tolerance. Buffer saturation shall never silently overwrite unread data. Completion shall distinguish buffer consumption from final wire transmission where relevant.

Acceptance: host state-machine tests and target loopback cover normal TX/RX, framing settings, finite receive lengths, errors, and baud accuracy. Continuous circular reception and hardware flow control are optional later capabilities unless explicitly added.

## REQ-0028 SPI
@depends(REQ-0010, REQ-0011, REQ-0017, REQ-0019, REQ-0040)

SPI shall provide controller/master full-duplex, write, and read operations with explicit mode, bit order, frame width, rate, dummy transmit value for reads, and chip-select behavior. A transaction shall retain exclusive bus ownership and chip select across its declared segments. Completion shall account for the peripheral becoming idle before deasserting chip select, including after DMA completion.

Acceptance: fake-device and physical loopback/device tests verify supported modes, requested rate tolerance, segment ordering, chip-select timing, final-frame completion, and timeout cleanup. Slave operation is deferred.

## REQ-0029 ADC
@depends(REQ-0010, REQ-0011, REQ-0017, REQ-0019, REQ-0040)

ADC shall provide validated analog-channel selection, resolution/reference/sample-time configuration within hardware capabilities, software-triggered single conversions and finite channel sequences, and explicit result format/channel ordering. Initialization shall follow documented calibration/stabilization requirements where applicable. Conversion and overrun failures shall be explicit. Raw samples shall not be presented as calibrated voltages without a supplied reference/calibration model.

Acceptance: deterministic host tests verify sequence/result ordering and configuration rejection; board measurements use known input levels and documented reference/tolerance assumptions. Continuous streaming and advanced trigger modes are separate extensions.

## REQ-0030 DMA driver and peripheral integration
@depends(REQ-0013, REQ-0017, REQ-0019, REQ-0040)

The first release shall provide an explicit DMA operation API and at least one documented supported DMA path for UART, SPI, and ADC where RA4M1 routing permits. The initial engine is RA4M1 DMAC; DTC is a separately scoped extension. Drivers shall advertise each supported direction and mode, reject unsupported requests, and report terminal completion only after both DMA and peripheral protocol completion conditions are satisfied. DMA shall not be silently required for ordinary non-DMA use.

Acceptance: memory-transfer tests where supported and peripheral transfer tests cover count/width/alignment boundaries, buffer ownership, abort, stale completion, and DMA/peripheral completion ordering. Capability/routing review must confirm the selected paths before coding; any unavailable path is a documented scope decision, not an invented mapping.

## Integration and target expansion

## REQ-0031 Board and startup integration
@depends(REQ-0003, REQ-0011, REQ-0012)

The core library shall not own reset entry, bootloader policy, linker layout, or vectors implicitly. Integration shall document required initialization, memory layout, vector bindings, and clocks. Existing RTOS/BSP startup shall be usable through explicit bindings without pulling vendor HAL dependencies into generic drivers.

A standalone UNO example shall supply minimal startup/linker/vector support (D08). A separate integration path shall permit existing RTOS/BSP startup without replacing its ownership. No undocumented bootloader preservation or flash-offset assumptions are permitted.

Acceptance: documented builds produce a correctly linked target image with one owner per vector/core service, plus an explicit flash/debug procedure for the chosen boot path.

## REQ-0032 Later MCUOS integration and acceptance
@depends(REQ-0015, REQ-0016, REQ-0031, REQ-0041)

After the bare-metal driver milestones, a separate stage shall integrate MCUOS from `../mcuos` (D03), through a dedicated adapter and representative executable examples proving shared-bus serialization, task waiting, ISR completion, and bounded timeout/cancellation. Missing MCUOS implementation or evidence shall not prevent description or bare-metal release acceptance. MCUOS shall remain an optional build dependency selected through an explicit source-location option, defaulting to the sibling path for local integration; it shall not be copied into generic drivers.

The MCUOS adapter shall use the public `mcuos::rtos` facade for task-level waits and synchronization, and the supplied ISR context for kernel-callable interrupt completion. Time conversion shall preserve absolute deadlines, precision and saturation semantics. Target composition shall own critical-section/interrupt entry bindings, priority-ceiling checks, clock/alarm resources, vectors and scheduler exceptions. Peripheral initialization shall not take over MCUOS-owned timers or core services.

Completion transport shall use explicit bounded storage and ownership: a dedicated queue or an explicitly reserved task-notification facility with operation identity and readiness rechecking. It shall not consume unrelated application notifications. Notification saturation, queue exhaustion, mutex owner death, task-wait cancellation and target failures shall have explicit outcomes. Cancelling an MCUOS wait alone shall not count as cancelling/quiescing peripheral hardware. A mutex shall be released by its owning task, not by the completion ISR; interrupt-critical sections shall remain short and shall not span a transfer wait.

Acceptance: first run actual MCUOS native tasks against simulated mcuhw peripherals, including completion-before-wait, simultaneous timeout/completion, stale wakeups, shared-bus contention and capacity exhaustion. Then execute the representative scenarios on UNO R4 Minima with an executable MCUOS RA4M1 target, using the same functional driver/backend sources as bare metal. The hardware RTOS claim remains outstanding until that target exists and passes; host tests and Cortex-M compilation do not satisfy it. Record exact source identity, kernel capacities, port/configuration identity and reserved resources.

## REQ-0033 STM32H753 description and subsequent bare-metal port
@depends(REQ-0003, REQ-0005)

The initial description phase shall select and record the STM32H753 device/package scope, review the locally available RM0433 reference manual, DS12117 datasheet and ES0392 errata, and implement the documented hardware in the shared model. Describe memory/cache/MPU properties, register semantics, clock/reset domains, pin routes, interrupt sources and DMA reachability without inheriting RA4M1 layout assumptions. PM0253 alone is insufficient for peripheral definitions.

After the common gate, the driver phase shall select a physical board and implement STM32 backends for the agreed functional driver set. Board bring-up, live cache/MPU operations and runtime driver verification belong to that later phase, not description validation. MCUOS is not a prerequisite for either.

STM32 backend implementation and board-native bring-up shall use ST-LINK debug/trace and virtual COM, built-in LEDs/buttons, documented solder bridges or point-to-point jumpers, the on-board Ethernet PHY/connector with an ordinary network peer, native connectors, and internal memory/peripheral known-answer tests. Ethernet work in this phase covers source-backed clock/pin setup, MDIO/PHY control, link-state verification and raw MAC-frame exchange, followed by the explicit MCUIP integration required by REQ-0043. Every physical board modification shall be recorded with its schematic designator, original state, resulting state, purpose and restoration procedure. Claims remain limited to the behavior independently observed by those target-specific procedures.

Acceptance: the same description contract tests used for RA4M1 and ESP32-P4 instantiate real STM32H753 definitions before any driver work. Later, common driver contract tests, cross-builds and bare-metal target examples pass after hardware/board binding changes. Target evidence is reported under REQ-0036 with its actual observation path and without claiming unmeasured electrical behavior.

## REQ-0034 ESP32-P4 description and subsequent bare-metal port
@depends(REQ-0003, REQ-0005)

The initial description phase shall pin reviewed documentation and silicon/package scope and implement ESP32-P4 in the shared model. Describe HP/LP domains and CPU ownership constraints, GPIO matrix/IO MUX routing, interrupt matrix, clock/reset relations, memory/cache properties, DMA engines and actual timer/PWM capabilities without mimicking RA4M1 GPT or Arm core registers. Preliminary or missing definitions shall remain explicit source gaps.

After the common gate, select a physical board, boot/toolchain integration and initial core-ownership model, then implement the agreed functional driver set through Espressif-specific backends on bare metal. Hardware multicore/LP concurrency is a separate validated capability. MCUOS availability is not a prerequisite.

Acceptance: real ESP32-P4 definitions satisfy the common description tests before any driver work. Later, common driver tests and physical bare-metal examples pass using target bindings. Description-only success shall not imply verified boot, cache, DMA or multicore execution.

## Verification and delivery

## REQ-0035 Deterministic host verification
@depends(REQ-0008, REQ-0017)

The project shall provide native tests using instrumented MMIO, fake peripheral events, fake monotonic time, and execution-policy fakes. Tests shall assert externally meaningful protocol, lifecycle, and register-access behavior, including failures and concurrency interleavings. Compile-pass/fail tests shall verify static constraints. No host test shall dereference target MMIO addresses.

Acceptance: the suite detects unsafe RMW, invalid pins, solver overflow, invalid resource sharing, lost wakeups, premature completion, and buffer lifetime violations. Host success is not represented as physical-hardware verification.

## REQ-0036 Target verification and measurements
@depends(REQ-0001, REQ-0035)

Each supported release shall cross-compile and run representative target acceptance scenarios for every claimed driver. Verification is scoped per release and records its observation path. Debug access, built-in board features, documented bridges/jumpers, directly connected devices, native connectors with ordinary peers, and internal known-answer/self-tests may produce target evidence when they independently observe the claimed behavior. The first RA4M1 release uses the connected UNO R4 Minima with direct GPIO/PWM/UART wiring, a Grove I2C target, W5100 Ethernet-shield SPI/DMAC traffic and a potentiometer ADC input.

Captures/logs shall identify board and revision, firmware, toolchain, clocks, observation path, connected hardware and expected tolerance. PWM/bus timing requires observed waveforms or equivalent independent measurement when timing is part of the claim; ADC tests require a controlled or user-adjustable input. A debug-register readback or internal loopback is valid only for the behavior it independently observes and shall not be presented as external electrical or timing evidence. Unmeasured electrical limits, complete-board coverage and fault-injection behavior remain explicit non-claims rather than release blockers unless a later requirement adds them to a release scope.

Acceptance: each driver has a reproducible target procedure and recorded result, including interrupt and selected DMA paths. Every externally accessible instance receives at least a smoke test; representative instances additionally receive protocol, timing, error-injection and boundary tests. Board-only functions and internal accelerators have explicit native-connector or known-answer procedures. No driver or board feature is marked hardware-verified based only on compilation or mock results.

## REQ-0037 Overhead and bounded resources
@depends(REQ-0002, REQ-0035)

Compile-time descriptions and solvers shall introduce no runtime traversal or solving when static configuration is used. Builds shall include only used functionality, with operation storage sizes and bounded work documented. Benchmarks shall report flash, static RAM, stack, critical-section/ISR duration, and representative operation cost against a direct-register baseline under identical build conditions.

Acceptance: inspect optimized output for GPIO and statically configured PWM/I2C to verify absence of runtime solvers and unwanted allocation/dispatch. Numeric size/latency budgets shall be chosen from the first working RA4M1 measurements before release acceptance; no unsupported zero-overhead claim is permitted.

## REQ-0038 Build, examples, and maintainability
@depends(REQ-0002, REQ-0003)

CMake shall provide explicit native-test and cross-build configurations, individually usable library components, and documented toolchain selection. Public APIs shall be backed by compilable examples for all first-release drivers, including blocking/polling transfer use, failures, and board binding. Description changes shall be reviewable C++ diffs with provenance updates. Code generation shall not be required to consume the first release.

Acceptance: documented commands build native tests and a clean target example; every published example is compiled. A new backend/MCU author can identify required concepts, traits, environment bindings, and acceptance tests without copying a vendor HAL.

## REQ-0039 Requirements and plan traceability

Requirements shall retain stable IDs and observable acceptance criteria. Plan steps shall explicitly link the requirements they deliver and keep planned work distinct from completed implementation and verification. Changes to accepted scope shall update the decision register and affected requirements/plan links.

Acceptance: mcutrace loads these requirements and the implementation plan without unresolved IDs or malformed relationships. At this planning stage no build, test, coverage, or implementation evidence is claimed, and no new evidence-severity policy is imposed.

## REQ-0040 Three-MCU description validation before drivers
@depends(REQ-0003, REQ-0005, REQ-0006, REQ-0007, REQ-0010, REQ-0011, REQ-0012, REQ-0013, REQ-0033, REQ-0034)

A mandatory gate shall establish that the description model represents all three MCUs before peripheral backend or functional driver implementation. Core register/field primitives, metadata validation and description-only tests are allowed during this phase; initialization sequences, transfer engines and device drivers belong to the following phase.

Acceptance requires:

- Real, source-backed descriptions for all three declared device baselines, with the coverage audit required by REQ-0006. Compile a translation unit including all three together and isolated headers for each; no global target macro, symbol collision or board/RTOS include may be required to read metadata.
- One parameterized compile-time test suite instantiated against each MCU, validating register widths/access semantics, legal field values, pin availability/routing, clock relations, multiple IRQ/event sources, DMA constraints and instance capabilities. Include representative valid and invalid configurations for all three, rather than three vendor-specific tests that merely check local aliases.
- An explicit comparison of source-backed differences in pin routing, clock graphs, IRQ/event routing, DMA/memory domains and timer output identities. Preserve native capabilities and opaque vendor encodings; do not force a fake common register layout or silently drop features to pass concepts.
- Native metadata compilation and target-toolchain compile probes for each architecture, without executing MMIO or requiring board startup. No runtime peripheral behavior or driver support is claimed by these checks.
- A source-gap register. Resolve missing authoritative material needed for the common peripheral/resource model before accepting the gate. If a vendor has not published a remaining block, identify its exact exclusion and seek a scope decision before proceeding; an undocumented placeholder shall not satisfy complete coverage.

The gate shall be explicitly recorded as passed before the first driver plan item starts. If model changes are required by another target, apply and validate them across all three before freezing the initial driver-facing contracts.

## REQ-0041 Bare-metal driver acceptance before MCUOS
@depends(REQ-0021, REQ-0022, REQ-0024, REQ-0027, REQ-0028, REQ-0029, REQ-0030, REQ-0031, REQ-0035, REQ-0036)

Following description acceptance, the initial driver milestones shall use native simulation and bare-metal execution only. Deliver the D04 set on RA4M1 first, then validate reusable functional interfaces through the STM32H753 and ESP32-P4 backends. Driver storage, blocking/polling use, interrupt completion, deadlines, cancellation and selected DMA paths shall work without scheduler services.

Backend completion requires native behavior tests, cross-build/link evidence and the target procedures selected for that release. For the first RA4M1 release, representative direct-wired and attached-device results cover the D04 GPIO, timer/PWM, UART, I2C, SPI, ADC and DMA set. Evidence shall retain its observation limits; protocol, timing, analog accuracy, electrical limits and fault injection are claimed only where they were actually measured.

Acceptance: RA4M1 bare-metal release acceptance is independent of MCUOS and uses the representative physical scope selected in D14. Subsequent bare-metal target reports demonstrate the STM32H753 and ESP32-P4 descriptions can underpin real backends with shared functional/device interfaces using their own target-specific evidence. Missing MCUOS code does not block any bare-metal release. Begin the separate MCUOS integration milestone after the bare-metal acceptance milestones, retaining implementation, native, build and physical evidence distinctions.

## REQ-0043 MCUIP-compatible Ethernet device boundary
@depends(REQ-0002, REQ-0003, REQ-0009, REQ-0013, REQ-0015, REQ-0017, REQ-0018, REQ-0033)

Every Ethernet backend delivered by mcuhw shall expose protocol-neutral,
fixed-storage MAC, MDIO, PHY and DMA facilities plus a thin target adapter that
can be composed with the sibling `../mcuip` project. The initial
NUCLEO-H753ZI composition shall satisfy the actual public MCUIP
`NetworkDevice`, `DescriptorProtection` and `HardwareDescription` contracts and
shall integrate with MCUIP's fixed receive/transmit rings and completion model.
The boundary shall preserve descriptor counts, RX buffer arming, TX submission,
polling, link changes, DMA-region and alignment constraints, cache clean and
invalidate operations, publish/acquire barriers, ownership transitions and
terminal completion without dynamic allocation.

MCUIP owns packet storage policy and the Ethernet/ARP/IP transport protocols;
mcuhw owns the target peripheral implementation and source-backed hardware
description. Vendor types shall not cross into MCUIP's portable core. MCUIP
shall be an explicit optional source dependency whose local default is the
sibling path, not copied into mcuhw. lwIP is not a supported dependency or
compatibility target. Integration evidence shall identify the exact MCUIP
source content and build configuration used.

Acceptance: compile-time probes reject incomplete target contracts; one Cortex-
M7 firmware links the mcuhw Ethernet backend with `mcuip::mcuip` without lwIP,
heap allocation, exceptions or RTTI; and the physical NUCLEO-H753ZI exchanges
raw minimum/maximum-MTU frames plus at least one MCUIP-driven protocol path
(ARP followed by ICMP echo or UDP) with an independent Ethernet peer. Exercise
link down/up, descriptor exhaustion, completion ordering and cache maintenance
with data cache enabled, and record both mcuhw and MCUIP source identities.

## REQ-0044 Arduino UNO R4 Minima W5100 Ethernet connectivity
@depends(REQ-0002, REQ-0003, REQ-0009, REQ-0015, REQ-0017, REQ-0018, REQ-0019, REQ-0028, REQ-0030, REQ-0031, REQ-0035, REQ-0036, REQ-0037, REQ-0038, REQ-0041)

The Arduino UNO R4 Minima shall provide Ethernet connectivity through the
connected Arduino Ethernet Shield R3 and its WIZnet W5100. The board binding
shall use the source-backed UNO SPI route, D10/P112 chip select, D4 held
inactive for the shield's microSD device, and the existing interrupt-driven
SPI and selected DMAC transfer paths. The implementation shall pin the reviewed
W5100 datasheet and shield schematic identities rather than treating the
earlier register smoke test as a complete network driver specification.

The W5100 device driver shall use fixed caller-owned or statically bounded
storage, explicit ownership and typed timeout/error recovery. It shall support
reset and common-register configuration, MACRAW socket initialization,
interrupt/status handling, circular RX/TX buffer accounting including wrap,
frame submission and receipt, malformed/oversize rejection, command timeout,
cancellation and clean reinitialization. SPI chip select and any paired DMAC
operations shall be quiescent before buffers or operation slots are released.

Expose the raw-frame path through a thin adapter satisfying the actual sibling
MCUIP `NetworkDevice` contract so MCUIP owns Ethernet/ARP/IP transport policy.
The SPI-attached W5100 is an external network device, not an RA4M1 native MAC;
the adapter shall not invent MDIO, PHY-register or native Ethernet-DMA
facilities. The initial increment does not expose the W5100 hardware TCP/UDP
socket API and shall not depend on the Arduino Ethernet library or lwIP.

Acceptance: native tests cover register and buffer commands, ring wrap,
backpressure, malformed/oversize frames, timeout/cancellation, stale status and
SPI/DMAC cleanup; public headers and a complete MCUIP composition cross-build
and link for Cortex-M4 without heap allocation, exceptions or RTTI. On the
connected UNO and shield, firmware resets and identifies the W5100, configures
MACRAW mode, exchanges representative small and MTU-sized raw frames with an
independent local-network peer, and completes at least ARP plus either ICMP echo
or a UDP datagram through MCUIP. Unplug/reconnect or peer-loss recovery returns
to successful traffic without leaking ownership. Record board/shield revision,
MAC/IP configuration, firmware/toolchain, W5100 and MCUIP source identities,
SPI/DMAC mode, image sizes, bounded storage, throughput/latency and reproducible
peer commands.

## REQ-0045 Modern C++ application interface
@depends(REQ-0002, REQ-0014, REQ-0015, REQ-0018, REQ-0037, REQ-0038)

Provide an optional `mcuhw::io` target and namespace in this repository (D16).
Expose board objects, named pins, serial, I2C, SPI, analog input, PWM and explicit
DMA selection using the existing drivers and backends. Construction shall be
passive. Initialization and failures shall be explicit; convenience failure
handling shall use a statically selected policy. Disabled features shall omit
their backend storage, and unsupported selections shall fail at compile time.

One active runtime per physical target shall coordinate pin ownership across
features and aliases. Move-only resource leases and operation handles shall
retain stable runtime storage. Operations borrow caller-owned buffers, retain
the existing bounded completion/cancellation contracts and expose typed errors.
No heap, background queue, scheduler or implicit DMA fallback is permitted.
Expert backend access shall be identified as bypassing facade bookkeeping.

Execution and MMIO shall be replaceable independently of the application API.
This increment uses one foreground context on one core, progressing work through
polling and blocking waits. It does not implement an RTOS adapter, camera,
network service, multicore arbitration or Arduino sketch compatibility.
Profiles shall document fixed clock, pin and silicon assumptions. Existing
startup ownership remains explicit.

Acceptance: deterministic host tests verify storage selection, shared ownership,
alias conflicts, rollback, moved handles, deadline validation, polling,
cancellation, ISR rejection and retained-buffer/runtime lifetimes. Public
headers and representative applications compile for Cortex-M4, Cortex-M7 and
ESP32-P4 RISC-V; standalone UNO examples link with existing startup. Publish
reproducible paired flash/static-RAM and compiler stack-frame measurements
against equivalent checked-driver compositions, with source/compiler identities.
These build/host checks establish this foundation only; physical facade
acceptance, peak stack usage and timing measurements require separate evidence.

## REQ-0046 Attached devices and shared bus endpoints
@depends(REQ-0014, REQ-0018, REQ-0019, REQ-0024, REQ-0028, REQ-0044, REQ-0045)

Expose move-only I2C address and SPI chip-select device handles from `io`.
A bus shall have one stable controller backend shared by its endpoints. I2C
address claims shall reject duplicates and raw accesses that bypass a claim.
SPI endpoints shall reserve CS pins through shared GPIO ownership and carry
validated clock/mode/order/width/polarity settings applied before a transaction.
Combined I2C repeated-START transfers and multi-segment SPI transactions shall
retain their bus ownership until terminal completion. Pending and retained
result handles shall prevent endpoint release and controller shutdown. No
hidden heap or unbounded transaction queue is permitted.

Place attached-chip protocols under `mcuhw/device/` with a narrow supplied
transport contract. Add a TMP102 sensor returning explicit signed temperature
units without a board dependency; preserve its latest-sample semantics and
existing hardware settings. Compose the W5100 with a checked SPI device handle,
retain a board-independent register protocol, and expose its MCUIP contract
through a generic integration header. Place ES8311 codec configuration under
`device/audio/`. Remove the former driver and board-specific W5100 compatibility
entry points; keep shield wiring in the application.

Component operations comprising multiple bus transactions shall share one
absolute deadline. W5100 payload copying shall have an explicit per-poll
budget and resume across calls. Ambiguous hardware commits and cancellation
of an issued SEND shall require recovery before reuse. Validate W5100 bus
format and stable-read configuration before hardware access. Retain the
optimized RA4M1 immediate path with deadline checks and caller-owned buffers.

Acceptance: host simulations verify shared ownership, configuration switches,
CS completion/cleanup, failure recovery, repeated START, buffer/handle lifetime,
signed TMP102 samples, W5100 framing, partial-copy deadlines and cancellation.
Device compositions and generic MCUIP contracts compile on all three board
profiles/architectures; standalone UNO examples link. Publish fresh cost probes.
These checks do not establish new on-device acceptance or RTOS/multicore safety;
previous hardware measurements remain scoped to the recorded implementations.

## Repository adoption and scale

## REQ-0047 Independent library consumption and packaging
@depends(REQ-0002, REQ-0003, REQ-0038)

Provide distinct application, component-contributor and full-maintainer build
workflows (D18). Applications shall consume the existing `mcuhw::mcuhw`,
`mcuhw::io` and `mcuhw::device` interfaces from a source checkout or a versioned,
relocatable CMake package. Direct header consumption shall remain supported;
ordinary consumption shall require neither catalog generation nor a board
selection for board-independent code.

Embedded CMake use shall default to library interfaces only. Tests, architecture
probes, all-header checks, coverage and hardware-test images shall be explicit
mcuhw options and shall not change the parent project's testing settings or
global compiler policy. Resolve test, coverage, network and RTOS dependencies
only for requested features. Ordinary applications shall not require sibling
checkouts, and unrelated checkout presence shall not change selected features.

Acceptance: clean external consumers build through `add_subdirectory` and
`find_package(mcuhw CONFIG REQUIRED)`, including after relocating the installed
package and removing access to the original source/build trees. A parent with
its own tests enabled retains them without creating mcuhw test/probe targets.
Plain header inclusion works independently. Installed interfaces contain no
developer-specific paths, and an explicit maintainer configuration still enables
the existing verification workflows.

## REQ-0048 Local hardware catalog and reusable component identities
@depends(REQ-0003, REQ-0005, REQ-0010, REQ-0038, REQ-0046)

Organize boards and devices as locally owned components in one repository,
retaining stable public C++ entry points. A versioned, schema-validated catalog
shall distinguish peripheral IP implementations, MCU family/part/package/silicon
variants, board revisions and external chips. Use stable vendor-qualified IDs
and explicit references to shared implementations rather than copying a backend
or chip driver per board or variant.

Local manifests shall identify component kind, revision scope, source/header and
build entry points, dependencies and required capabilities, examples, maintained
documentation, ownership and evidence references. C++ remains authoritative for
register definitions, typed pin mappings and compile-time constraints; catalog
summaries shall be checked against their source rather than becoming a second
independently authored hardware description. Board composition owns wiring;
external-device protocols retain narrow transport contracts. The catalog shall
not introduce runtime registration or mandatory code generation.

Acceptance: all three existing boards and TMP102, W5100 and ES8311 have entries
with their actual support scope. Validation rejects unsupported schema versions,
duplicate IDs, missing references, dependency cycles and invalid revision or
capability selections. Adding a component that reuses existing infrastructure
requires only its local files and public implementation, without handwritten
central board/device lists; any global index is reproducibly derived. Shared
implementation and board-specific configuration are demonstrably separate.

## REQ-0049 Selected-board application builds and explicit startup
@depends(REQ-0031, REQ-0045, REQ-0047, REQ-0048)

Provide one board-selection interface using an exact board/revision identity.
Resolve its required toolchain before compiler configuration and load only its
build rules and transitive dependencies. Board-local rules shall use common
firmware, linker, artifact and runner helpers; adding a board on an existing
platform shall not require a root CMake branch, a new global option or a
handwritten preset. Different board/toolchain configurations shall use isolated
build directories.

Repository workspaces shall select an architecture through a configure preset
and an explicit, persistent active-board list through CMake cache settings or a
local user preset. Only those boards shall contribute task groups such as
`<board>.examples` and `<board>.tests`; individual example images remain
independently buildable. Resolve examples from each selected board's manifest,
reject incompatible architecture/revision selections, and isolate generated
configuration and artifacts per board/revision/example. Changing the active set
shall remove inactive targets on reconfiguration. Unselected catalog growth
shall not enlarge the workspace target list or configure test dependencies.

Applications shall explicitly choose supplied board startup or their existing
BSP/RTOS startup. Supplied integration shall bind clocks, memory layout, vectors
and boot policy without transferring their ownership to generic drivers. Keep
the ESP32-P4 SDK/boot path explicit under the common selection interface and
preserve its silicon limitations. Flashing shall be a separate requested action,
with board/probe/port selection and documented flash regions and recovery.

Acceptance: complete external Blink and TMP102 applications cross-build and link
for UNO R4 Minima, NUCLEO-H753ZI and ESP32-P4-NANO using the common workflow and
documented wiring/profile choices. A composition using externally supplied
startup links without duplicate vectors or reset/clock services. Unknown board
IDs, unsupported revisions and incompatible toolchains fail with actionable
diagnostics. Build configuration creates no unrelated board/device targets;
build success does not establish physical device acceptance.

## REQ-0050 Reproducible setup and portable first-build workflow
@depends(REQ-0002, REQ-0047, REQ-0049)

Provide a documented first-build path on Linux, macOS and Windows: discover a
board, create a complete external application, diagnose prerequisites, build,
and explicitly flash through the selected runner. A thin convenience frontend
shall use the same CMake and runner definitions as direct tool invocation;
ordinary CMake and header consumers shall not depend on that frontend.

Publish tested compiler, C++23 standard-library, build-tool and required SDK
versions. Pin dependency/toolchain identities and download integrity, with
explicit setup, reusable caches and configurable local installations/source
overrides. Local sibling checkouts remain a development option. After setup,
selected builds shall work offline without silently resolving newer versions.
Diagnostics shall identify missing or incompatible prerequisites for the requested
operation and concrete remediation before starting it; absent flash/debug tools
shall not block a build-only workflow. Board-specific flashing differences belong in
runner configuration or reusable adapters, not copied host-specific scripts.

Acceptance: clean environments on the three host operating systems complete the
documented setup and external application build using pinned inputs. A cached
offline rebuild succeeds; missing cache entries, incompatible C++ libraries and
missing SDK/debug tools required by the requested operation produce focused errors.
Reports identify exact tools, dependencies, overrides and board configuration.
Flash-command selection is
tested independently, and physical runner claims name the host, hardware and
observation actually exercised under REQ-0036.

## REQ-0051 Searchable support catalog and component contribution workflow
@depends(REQ-0036, REQ-0039, REQ-0048, REQ-0049, REQ-0050)

Generate searchable board/device listings and support tables from component
manifests and referenced evidence. Expose identity/revision, category, buses and
required capabilities, toolchain/runner availability, examples, wiring and
limitations. Keep described capabilities, implemented functionality, host-test,
cross-build and physical verification states distinct; compatibility shall not
imply that every board/device combination has been tested. Missing, failed and
stale evidence shall retain their own states.

Provide separate application quick starts and contributor guides/templates for
an existing-MCU board, an external chip and a new MCU/backend. Each component
shall own its focused tests and narrative documentation; generated indexes
shall remove repeated manual registration across docs, builds and CI. Version
and compatibility guidance shall identify public API/catalog changes and any
required migration. Examples shall be complete, compilable applications with
explicit prerequisites and wiring rather than unbuildable fragments alone.

Acceptance: users can find I2C temperature devices, boards exposing Ethernet,
available flash methods and physically verified features for an exact revision.
Each result leads to matching build commands and scoped evidence. Following
the board/device templates adds a working local component without central
handwritten lists. Documentation generation is reproducible, validates its
references and examples, and excludes synthetic scalability fixtures from
published support claims.

## REQ-0052 Dependency-scoped verification, provenance and scale
@depends(REQ-0035, REQ-0036, REQ-0037, REQ-0039, REQ-0047, REQ-0048, REQ-0049)

Separate core, MCU/backend, board-integration and attached-device test suites
with independently selectable runs and evidence artifacts. Changes shall select
affected components and transitive dependents; changes to shared infrastructure,
deleted inputs or uncertain impact shall trigger a conservative broader check.
Retain explicit exhaustive, shardable catalog validation for scheduled/release
runs and representative mixed-MCU/header checks. Device protocol tests and bus
backend tests shall be reusable; a board-by-device Cartesian product shall not
be the default test strategy or a substitute for scoped physical acceptance.

Application configuration and compilation shall operate on the selected
dependency closure without scanning/hashing every device implementation or
creating unrelated validation targets. Firmware identity shall cover actual
transitive selected sources, generated configuration, dependencies, toolchain
and flags. Record whole-repository revision separately; unrelated component
changes shall not alter the selected firmware identity. Preserve source and
evidence traceability when moving or splitting existing build/test rules.

Acceptance: synthetic fixtures representing at least 300 boards and 3,000
devices exercise discovery, validation and selection without claiming real
hardware support. Record configuration, discovery and selected-build timings,
target/source counts and identity inputs on a named host/toolchain. Growing the
unselected catalog leaves the selected target/source/hash set unchanged. A
device change selects its tests and dependents; a shared or deleted dependency
selects broader checks. Scheduled/release validation covers the full declared
catalog. Selected-source/configuration changes alter firmware identity, while
unrelated additions do not; all evidence claims retain their observation scope.
