# mcubsp common implementation plan

## Common and board documents

The root [requirements](requirements.md) and [implementation plan](implementation-plan.md)
own shared contracts, cross-board milestones and the historical decision/source baseline.
Each board owns the documents below beside its `board.cmake`. Shared MCU/backend code
remains reusable; document ownership does not move that code into a board.

| Scope | Requirements | Planning |
| --- | --- | --- |
| Common | [Requirements](requirements.md) | [Implementation plan](implementation-plan.md) |
| Arduino UNO R4 Minima | [Requirements](src/board/arduino/uno_r4_minima/requirements.md) | [Implementation plan](src/board/arduino/uno_r4_minima/implementation-plan.md) |
| ST NUCLEO-H753ZI | [Requirements](src/board/st/nucleo_h753zi/requirements.md) | [Implementation plan](src/board/st/nucleo_h753zi/implementation-plan.md) |
| Waveshare ESP32-P4-NANO | [Requirements](src/board/waveshare/esp32p4_nano/requirements.md) | [Implementation plan](src/board/waveshare/esp32p4_nano/implementation-plan.md) |
| Pro Micro nRF52840 | [Requirements](src/board/others/promicro_nrf52840/requirements.md) | [Implementation plan](src/board/others/promicro_nrf52840/implementation-plan.md) |

Requirements and plan IDs remain globally unique across all documents. Define each
ID once, reference common contracts from board documents, and retain explicit
`@depends` / `@link` relationships across files. Apply common contracts to the
peripherals selected by a board's scope; a shared API requirement does not promise
that every board implements it. Plan completion never substitutes for evidence.

Reorganized 27 October 2026: existing IDs, requirement bodies, relationships, plan
statuses and historical observations are retained. This split adds no peripheral
scope or hardware acceptance.

## Baseline

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
include/mcubsp/
  core/                 register/field types, units, concepts and metadata utilities
  arch/arm/cortex_m4/    architecture metadata; execution primitives added with drivers
  arch/arm/cortex_m7/
  arch/riscv/
  mcu/ra4m1/            declarative device, memory, clock, pin, IRQ, DMA and register data
  mcu/stm32h753/
  mcu/esp32p4/
  mcu/nrf52840/         initial GPIO/core/TWI slice
  backend/              peripheral register sequences shared by compatible MCU variants
  driver/               functional interfaces and logic
  device/               MCU-independent attached-chip protocols
  board/                package selection, board identity and connector/pin aliases
  execution/            later portable execution contract and bare-metal policy
  io/                   application facade, board profiles, storage and ownership
src/board/<vendor>/<board>/ local board settings, startup, tests, requirements and planning
src/device/<category>/  device-owned tests; device documentation under doc/device/
src/                    non-template behavior where useful
adapters/mcuos/          phase E only
test/                 description contracts, then driver models and hardware procedures
doc/                   manuals, provenance and coverage manifests, design decisions
cmake/                 native and architecture/toolchain configurations
```

Templates and constant-evaluated definitions live in headers; other behavior may use compiled translation units. Description headers must be usable together on a host without board startup, OS includes or a global target-selection macro. Do not create empty target trees as evidence of support.

Phase D2 retains the public `include/mcubsp/` entry points and places local
build settings and procedures beside each board. Shared MCU, backend and device
support remains reusable. Board selection reads the chosen `src/board/<vendor>/<board>/board.cmake`.

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

The MCU-specific description items live with their initial carrier:

- [UNO / RA4M1: PLN-0004](src/board/arduino/uno_r4_minima/implementation-plan.md#pln-0004-describe-ra4m1-foundational-resources-and-initial-peripherals)
- [NUCLEO / STM32H753: PLN-0014](src/board/st/nucleo_h753zi/implementation-plan.md#pln-0014-describe-stm32h753-using-the-shared-model)
- [P4-NANO / ESP32-P4: PLN-0015](src/board/waveshare/esp32p4_nano/implementation-plan.md#pln-0015-describe-esp32-p4-using-the-shared-model)

The next description items may be interleaved by block category. For example, describe clocks for all three before proceeding to pins for all three. This is scheduling of work, not a requirement to delegate to multiple agents. Do not finish a RA4M1 driver while the other descriptions remain hypothetical.

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

UNO startup/GPIO and basic PWM (PLN-0005/PLN-0007) are in the
[UNO board plan](src/board/arduino/uno_r4_minima/implementation-plan.md).
Shared ownership, execution and transfers remain below.

Every item in this phase is gated by PLN-0017. Build backends against existing descriptions rather than adding MCU addresses to generic drivers. If a driver reveals a metadata omission or model error, correct and revalidate the affected contracts on all three before continuing.

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

## Phase D: bare-metal implementation and target acceptance

Driver implementation and physical acceptance are tracked in each board plan:

- [UNO: PLN-0008–PLN-0011 and W5100 PLN-0024](src/board/arduino/uno_r4_minima/implementation-plan.md)
- [NUCLEO: backends PLN-0018 and USB/Ethernet PLN-0023](src/board/st/nucleo_h753zi/implementation-plan.md)
- [P4-NANO: backends and board-native acceptance PLN-0019](src/board/waveshare/esp32p4_nano/implementation-plan.md)
- [Pro Micro: GPIO/I2C PLN-0040–PLN-0041 and complete-peripheral delivery PLN-0042–PLN-0053](src/board/others/promicro_nrf52840/implementation-plan.md)

The application facade and reusable attached-device milestones stay common.

## PLN-0025 Deliver the modern C++ application interface foundation
@status(done)
@link(REQ-0002, REQ-0014, REQ-0015, REQ-0018, REQ-0035, REQ-0037, REQ-0038, REQ-0045)

Implemented the optional `mcubsp::io` target and board-oriented API on the
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
examples cover a temperature threshold and raw Ethernet reception. The
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
  mcubsp options; preserve parent testing and compiler settings.
- Add versioned install/export support for `mcubsp::mcubsp`, `mcubsp::io` and
  `mcubsp::device`, preserving source-checkout and direct-header consumption.
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

Current migration note (24 October 2026): the frontend/workspace/verification
helpers described in the historical implementation notes below were removed.
PLN-0033 restores direct CMake workflows and current native evidence. It does not
reestablish the removed setup/flash frontend, multi-board workspace qualification,
or hosted/physical execution. These acceptance items remain open.

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

Current migration note (24 October 2026): the frontend/workspace/verification
helpers described in the historical implementation notes below were removed.
PLN-0033 restores direct CMake workflows and current native evidence. It does not
reestablish the removed setup/flash frontend, multi-board workspace qualification,
or hosted/physical execution. These acceptance items remain open.

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
persists `MCUBSP_ACTIVE_BOARDS` as a local CMake setting. Local board entries supply
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

## PLN-0033 Repair framework usability and verification after build migration
@status(done)
@link(REQ-0014, REQ-0015, REQ-0018, REQ-0035, REQ-0038, REQ-0039, REQ-0040, REQ-0045, REQ-0047, REQ-0048, REQ-0049, REQ-0051, REQ-0052)

Implement the approved repository review: direct CMake application and library
workflows, independent native/cross/integration checks, current CI and evidence
paths, diagnostic-aware compiler rejections, and isolated public-header checks.
Provide selectable facade peripherals, byte SPI buffers, explicit monotonic
clock adaptation, and actionable lifecycle diagnostics. Publish a support matrix
that separates source review, typed metadata, driver implementation and physical
evidence. Existing target acceptance keeps its original firmware/fixture scope.

Exit: fresh native, packaging, header and cross-compiler checks pass; Blink
firmware links for all three boards; documentation describes delivered commands.
Physical traffic/timing checks and hosted CI execution remain independently open
until their producers are actually run.

Verified locally on macOS arm64: 41/41 CTest entries with GCC 15.1 and
AppleClang 21; 266 behavior/workflow cases; 110 isolated headers and architecture
probes on Cortex-M4, Cortex-M7 and P4; optional native MCUIP integration;
linked Blink for all three boards and all non-instrumented UNO test images.
Installed UNO/NUCLEO consumers link with supplied or external startup and one
Reset_Handler each. See [the repair record](doc/acceptance/usability-repair.md).
Hosted execution, coverage regeneration and physical refresh are not completed
by this software milestone; PLN-0030/PLN-0032 and target milestones retain their
independent outstanding acceptance.

## PLN-0034 Restore capture tooling and current coverage production
@status(done)
@link(REQ-0014, REQ-0017, REQ-0035, REQ-0036, REQ-0038, REQ-0039, REQ-0049, REQ-0050, REQ-0052)

Continue the approved usability work by restoring shared board-runner helpers,
explicit probe/port and image-boundary validation, NUCLEO test-image builds,
and native coverage capture/report production. Test runner planning with fake
tools and image fixtures; do not substitute those tests for physical evidence.
Repair unsupported trace annotation placement without changing requirement scope.

Exit: shared runner and coverage regressions pass, native coverage reports are
produced from executed current suites, and board test images build. Hosted CI,
physical flashing/observations and production-silicon acceptance remain separate.

Verified locally: 42/42 CTest entries and 283 behavior/workflow cases with
ordinary GCC and instrumented AppleClang; FULL native coverage exports with
zero invalid MC/DC decisions and explicit skipped constructs. All ten UNO,
three NUCLEO and the P4 SDK test images build. Current-image dry runs validate
all three runner paths without hardware access. See the
[capture and coverage record](doc/acceptance/capture-and-coverage.md) for scope,
coverage denominators and remaining qualification.

## PLN-0035 Record selected firmware inputs
@status(done)
@link(REQ-0049, REQ-0052)

Extend completed firmware records with selected compiler dependencies, generated
configuration, linker inputs, build commands and tool fingerprints. Keep the
repository revision separate from the selected-input identity and reject missing
or modified provenance sidecars during runner validation. Cover the SDK's
separate application and bootloader graphs; explicitly identify unsupported
generator tracking instead of claiming complete provenance.

The selected SDK graphs also revealed and corrected an ignored P4 test
bootloader setting and implicit Git-derived application version flags. Current
builds select the factory-only boot component and explicit application version.

Exit: selected source/header/configuration/flag and transitive-library changes
alter identity; unrelated files and repository-only commits do not. Fresh Arm
and P4 images produce records, runner tampering checks pass, and documentation
states the supported producer scope. Hosted/physical qualification stays open.

Verified locally: 43/43 CTest entries, 287 cases on GCC/AppleClang, all 14 board
test bundles and P4 Blink input records. Relocated installed UNO/NUCLEO consumers
with supplied/external startup produce records in paths with spaces and contain
one Reset_Handler each. See the [input record](doc/acceptance/selected-firmware-inputs.md).

## PLN-0036 Define the library user's CMake entry points
@status(done)
@link(REQ-0047, REQ-0049, REQ-0050, REQ-0051)

Expose application helpers through source/package consumption without requiring
private includes. Select the board's toolchain before project(), preserve pure
library inclusion in an existing BSP, and use explicit named C++ board types
in applications. Ship a complete external Blink project and explain
the user's responsibilities separately from framework test/release tooling.

Exit: source and relocated-package consumers use the same public helper; the
starter links for the three supported boards; existing-BSP consumption retains
its own targets/testing/compiler policy; invalid selections fail clearly.
Host/physical qualification and the remaining workspace milestone stay separate.

Verified locally on 26 October 2026: public starter links from source and a
relocated installation for UNO, NUCLEO and P4; existing-BSP consumers retain
their own targets/testing/languages; 43/43 CTest entries and 289 cases pass on
GCC/AppleClang. See the [application CMake record](doc/acceptance/application-cmake.md).

Board-name follow-up: remove the generated generic C++ alias and its include
directory. Starter sources now name UNO, NUCLEO and P4 types and LED pins
explicitly; CMake selects each board's source file and startup/toolchain.

## PLN-0037 Consolidate public board ownership
@status(done)
@link(REQ-0031, REQ-0048, REQ-0049, REQ-0051)

Move concrete I/O compositions into the vendor-qualified public board tree.
Provide one named board entry header and separate lightweight pins/bindings
headers per board. Keep generic I/O in io/, chip facts in mcu/ and the root
board support resources in place. Update consumers, packaging, build metadata
and migration documentation together; retain historical evidence identities.

Exit: native, isolated-header and cross-architecture checks pass; source and
installed applications link for the supported boards; low-level board bindings
require no I/O facade/backend implementation. Physical acceptance is unchanged.

Verified locally on 26 October 2026: 43 CTest entries and 290 cases on GCC and
AppleClang, 113 isolated headers on host and all three target architectures,
source and relocated-package starters for all three boards, and all 14 board
test images. The lightweight binding dependency check passes. See the
[board layout record](doc/acceptance/board-layout.md) for scope and migration.

## PLN-0038 Complete the BSP support-tree move
@status(done)
@link(REQ-0048, REQ-0049, REQ-0050, REQ-0051, REQ-0052)

Finish the approved directory layout: move root board support to bsp/ and
board-local startup sources to startup/, with Arm linker scripts beside the
board entry. Update build selection, runners, packaging, trace inputs and
authored documentation together, preserving captured evidence bytes.

Exit: root board/ is absent; CMake and installed consumers resolve bsp/ resources;
native, architecture and firmware builds pass with current paths; hardware
planning resolves the moved board entries without device execution. Existing
cached repository application defaults migrate to the new support path.

Verified locally on 26 October 2026: all three repository firmware builds,
source/relocated-package starters, all 14 board-test images and dry-run image
validation pass. Both native configurations pass 43 CTest entries and 290 cases.
Root board/ is absent, installed resources use bsp/, and historical captures
are preserved byte for byte. See the [BSP completion record](doc/acceptance/bsp-layout.md).

## PLN-0039 Extract cross-library integration
@status(done)
@link(REQ-0043, REQ-0044, REQ-0046, REQ-0047, REQ-0051)

Move MCUIP adapters, contract probes and combined board-test applications into
the sibling ../integration project. Keep peripheral/device implementations and
protocol-neutral tests in mcubsp. Remove core integration switches and document
explicit upper-layer consumption. Preserve recorded physical evidence.

Exit: mcubsp builds and installs independently; the sibling project compiles its
contracts on host and all three architectures and links both Ethernet images.

Verified locally on 27 October 2026: the real sibling project builds with default
sibling paths, host/M4/M7/P4 contracts pass, both Ethernet images link with and
without integration coverage, and both runner dry runs validate selected inputs.
mcubsp retains 43 passing CTest entries and 290 cases on GCC/AppleClang, 110
standalone headers and independent source/package consumers. See the
[integration extraction record](doc/acceptance/integration-extraction.md).
