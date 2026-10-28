# Pro Micro nRF52840 implementation plan

Implements the [board requirements](requirements.md) and their applicable
[common contracts](../../../../requirements.md). Shared foundation, description-gate,
application/device, repository and later MCUOS work stays in the
[common implementation plan](../../../../implementation-plan.md).

The items below retain their existing IDs, explicit requirement links, status,
prerequisites and verification notes. Read them in delivery order, not numeric
ID order. MCU description/backend work is owned here for this initial carrier
but remains reusable by other boards on the same MCU.

The user extended this board's target to every nRF52840 peripheral on
27 October 2026 (D20). PLN-0040/PLN-0041 remain completed initial increments;
PLN-0042/PLN-0043 are in progress; later full-peripheral stages remain planned.
GPIO/Blink, finite I2C and the native system-service checkpoint have independent
software/physical records. The historical
three-MCU gate does not establish complete Nordic descriptions or errata support.

## Full-peripheral delivery sequence

| Stage | Plan items | Delivery |
| --- | --- | --- |
| Source and model gate | PLN-0042 | Complete Nordic inventory, typed metadata, source/errata identity and core contracts |
| Shared runtime foundation | PLN-0043, PLN-0044 | Power/clocks, interrupts/events, time/watchdog and DMA/resource coordination |
| General I/O | PLN-0045, PLN-0046 | UART/UARTE, SPI/SPIM/SPIS, TWI/TWIM/TWIS, PWM, analog and QDEC |
| Memory and audio | PLN-0047, PLN-0048 | QSPI/internal flash/protection and I2S/PDM |
| USB and security | PLN-0049, PLN-0050 | USB device/classes/boot entry and RNG/crypto |
| Radio and NFC | PLN-0051, PLN-0052 | RADIO packet engine and NFCT/tag composition |
| Qualification | PLN-0053 | Complete implementation inventory and separately recorded physical acceptance |

New backend implementation follows PLN-0042's reviewed description gate for its
blocks and shared dependencies. Shared-model changes also revalidate the existing
MCUs. Preserve the initial GPIO/TWI evidence; the complete Nordic gate is a new
milestone. After their prerequisites, individual peripheral stages can progress
independently, with physical observations accumulated as each block becomes usable.

All new stages retain C++23, fixed storage, explicit ownership/errors and bare-metal
operation. Scope includes hardware-supported slave and streaming modes.
Higher-level radio stacks are independently selected integrations; direct RADIO
support must work without enabling the preserved SoftDevice. Missing hardware
access, source or a required dependency remains visible work, not a reduced
full-peripheral denominator.

A `done` status records the stated increment only. Physical observations and
unexecuted acceptance cases remain distinct in each item and in the
[support matrix](../../../../doc/reference/support-matrix.md).

## PLN-0040 Bring up Pro Micro nRF52840 GPIO and UF2 applications
@status(done)
@link(REQ-0053)

Pin the Zephyr board/UF2 definitions and Nordic GPIO baseline. Add the nRF52840
GPIO description/backend, a board profile with native pin names and P0.15 LED,
and supplied Arm startup/linker integration for the UF2/S140-v6 partition.
Reuse the existing CMake firmware workflow and provide deterministic UF2
generation without flashing. Publish the initial scope and exact handoff and
upload assumptions. Exercise native failure paths, UF2 format/bounds, and
source/installed-package linked applications.

Exit: the initial software path and documented procedures pass their stated
checks. Physical Blink and double-reset recovery remain explicitly unobserved
until exercised on the user's board; later peripheral backends are separate work.

Verified locally on 27 October 2026: 49 CTest entries and 304 cases pass on
AppleClang; 115 isolated headers pass on host and Cortex-M4 with architecture
barrier checks. Repository and source/relocated-package Blink link and emit UF2;
installed runner dry-run and the UNO firmware regression pass. That software
stage made no physical or complete Nordic description/errata claim. See the
[initial software record](../../../../doc/acceptance/promicro-nrf52840.md).

Physical follow-up on 27 October 2026: the connected board reported UF2 0.6.0,
Board-ID `nRF52840-nicenano` and S140 6.1.1. The explicitly authorized UF2 upload
completed; the user confirmed LED Blink. The acceptance record binds that
observation to the ELF/UF2 and selected-input hashes. Independent timing,
post-application double-reset recovery and broader hardware qualification remain
outstanding.

## PLN-0041 Add nRF52840 I2C and the Pro Micro sensor composition
@status(done)
@link(REQ-0054)

Implement source-backed TWI0/TWI1 metadata, native bindings and CPU-driven finite
transfers. Reuse operation pools and I/O device leases, preserve shared serial
instance ownership, and fail closed on unsuccessful cleanup. Add Pro Micro I2C
pin aliases, selector support and a standalone TMP102 example. Verify byte/STOP
ordering, combined transactions, cancellation/errors and shared-bus behavior on
the host; cross-build and check relocated installed consumption. Physical bus
and sensor acceptance are separate from this software increment.

Exit: focused and full native verification, isolated headers, board examples,
and source/package sensor builds pass with explicit remaining hardware scope.

Verified locally on 27 October 2026: all 52 CTest entries and 319 cases pass on
AppleClang, including 15 I2C cases and three negative binding compilations.
All 118 isolated headers compile on host and Cortex-M4; architecture barrier
checks pass. Blink/TMP102 repository examples and source/relocated-package sensor
applications link and emit UF2; installed runner planning passes without upload.
See the [I2C software record](../../../../doc/acceptance/nrf52840-i2c.md). Physical sensor,
bus timing and broader Nordic qualification remain outstanding.

## PLN-0042 Establish the complete Nordic description and errata gate
@status(in_progress)
@link(REQ-0055, REQ-0073)

Prerequisite: the existing GPIO/I2C baselines and common metadata contracts.

Pin content identities for the product specification, relevant Arm references,
MDK cross-check, board wiring and revision-applicable errata. Read FICR/build
identity when hardware is available. Reconcile every peripheral instance,
shared alias, CryptoCell subblock, register/field and supported mode against the
specification; include core/debug services and memory/clock/DMA constraints.
Implement the missing typed descriptions and define startup/external-BSP
responsibilities. Inventory physical pin availability separately from chip scope.

Exit: the complete inventory has owners and no unexplained omission; reviewed
blocks and their dependencies pass metadata, isolated-header and Cortex-M4F
checks before new backend sequencing. Required source/revision gaps stay open.

Implementation checkpoint, 27 October 2026: pin the Nordic MDK/Arm/Zephyr source
identities and add reproducible typed source facts for 43 families / 74 SVD
instances (2,142 expanded register and 5,949 field entries). Add capability
constraints, metadata tests/probes and source/package licensing. The import
passes deterministic reproduction; it is not the complete product gate. The
newer CC_* definitions, full mode/pin/package audit and exact silicon readback
remain open. See the [source baseline](../../../../doc/reference/nrf52840/README.md)
and [checkpoint](../../../../doc/acceptance/nrf52840-system.md).

Follow-up, 28 October 2026: pin PS v1.11 and the Cortex-M4/ETM-M4 manuals.
Reconcile all 1,567 register-overview entries by absolute address or explicit
reserved/CTRL-AP classification. Add 11 CryptoCell subblocks with 199 typed
registers and 343 fields, including 192 absent from the MDK. Preserve the seven
older CC_HOST_RGF absolute addresses through their different base/offset pair.
Extend identity readout with the ROM fields used by Nordic's errata predicates.
Complete product field/mode/core/pin/package and stepping-specific errata review,
and actual board identity readback, remain open; this is not gate acceptance.

## PLN-0043 Implement power, event routing, core services and time
@status(in_progress)
@link(REQ-0056, REQ-0057, REQ-0058, REQ-0073)

Prerequisite: PLN-0042 for the affected system/core/event/timing descriptions.

Implement POWER/CLOCK and reset/retention, full GPIO sensing/GPIOTE, PPI,
EGU/SWI, NVIC/vector/fault and MPU/FPU/debug integration. Add all TIMER/RTC
instances, WDT and a long-duration monotonic clock/alarm. Define shared clock,
pin, route and execution-resource ownership before connecting clients. Supply
bounded ISR dispatch, low power wake coordination and explicit debugger controls.

Exit: native race/boundary tests and linked compositions pass; physical records
exercise GPIO events, hardware routes, timer/counter/capture, RTC wake, watchdog,
core services and power states, with current and trace-access limits stated.

Implementation checkpoint, 27 October 2026: deliver explicit native clock/power,
GPIOTE/port sensing, programmable PPI/group/fork, EGU/SWI, TIMER/RTC/watchdog and
core/IRQ services; add bounded cleanup, endpoint leases, core reservations,
selected errata handling and the system example. Native software and linked
source/installed consumers pass their stated checks. Upload the identified UF2;
the user confirms regular timer/PPI/GPIOTE LED blinking. This does not prove RTC
wakeup or current consumption. Predefined PPI channels, full FPB/ETM/TPIU tooling,
remaining clock/power interfaces and exhaustive errata review remain software
work, alongside the independent physical procedures. The plan stays in progress;
see the [checkpoint](../../../../doc/acceptance/nrf52840-system.md).

Follow-up, 28 October 2026: implement fixed PPI 20–31, forks and 32-channel group
membership, with exclusive external radio/AAR/CCM owner adapters. Add external
LF clock configuration, HFXO debounce, POWER/CLOCK/PORT event endpoints and
interrupt cleanup, calibration-expiry/recovery handling, and a watchdog TIMEOUT
endpoint. RTC/watchdog clients share explicit LF configurations. Supply optional
GDB FPB/DWT/fault/identity inspection and OpenOCD SWO/ETM-M4/TPIU configuration
with explicit halted-target resource handoff and bounded recovery. Host mocks
and OpenOCD parsing/Jim Tcl execution pass. Complete source/errata review and
independent physical timing/wake/watchdog/core/power/debug/trace acceptance remain
open; no current image upload or hardware result is inferred from software tests.

## PLN-0044 Implement peripheral DMA and resource coordination
@status(planned)
@link(REQ-0062)

Prerequisite: PLN-0042; integrate clocks/interrupts from PLN-0043 as needed.

Extend shared ownership beyond the current TWI0/TWI1 registry. Distinguish
exclusive personalities from independent registers sharing a peripheral address.
Add memory/count/alignment validation, fixed buffer handoff, per-peripheral DMA
completion and cancellation contracts, including the crypto-specific engines.
Implement the common coordination layer here and each concrete DMA sequence in
its owning driver stage. Do not introduce fictitious generic DMA channels.

Exit: alias/resource and invalid-buffer tests pass; each later DMA driver links
to this contract and supplies its own completion/quiescence evidence. This stage
alone does not establish all peripherals' DMA implementation or physical timing.

## PLN-0045 Complete UART, SPI and I2C master/slave families
@status(planned)
@link(REQ-0059, REQ-0060, REQ-0061, REQ-0062)

Prerequisite: PLN-0042 through PLN-0044 for the relevant resources; preserve
PLN-0041's CPU-driven I2C and device-lease behavior.

Deliver UART0/UARTE0–1, SPI0–2/SPIM0–3/SPIS0–2 and TWI0–1/TWIM0–1/TWIS0–1.
Extend facade/native APIs for hardware-supported target and streaming operations,
with explicit storage, counts, errors, pin routing, timing and cancellation.
Implement instance-specific limits/errata and shared personality exclusion.
Keep every instance usable through explicit board selections.

Exit: per-instance native/negative-binding tests, linked examples and package
consumers pass. Physical peers verify UART TX/RX/flow control, SPI master/slave
and I2C master/target paths; independent wire timing is recorded separately.

## PLN-0046 Implement PWM, analog and quadrature decoding
@status(planned)
@link(REQ-0063, REQ-0064, REQ-0065, REQ-0062)

Prerequisite: reviewed metadata plus PLN-0043/PLN-0044 resources.

Implement every PWM instance and native sequence mode; SAADC calibration,
finite/continuous acquisition and limit events; COMP/LPCOMP and TEMP; and QDEC.
Reuse the common basic PWM/analog interfaces and expose additional native
features explicitly. Validate actual pin availability, references, clock ratios
and DMA buffer transitions; implement applicable shutdown/calibration workarounds.

Exit: native and cross-build suites pass. Measure PWM sequences, known analog
levels, comparator/wake transitions and QDEC direction/counting with suitable
sources, documenting sample-rate/accuracy and board-circuit constraints.

## PLN-0047 Implement external memory, internal flash and protection
@status(planned)
@link(REQ-0066, REQ-0072, REQ-0062)

Prerequisite: reviewed memory/protection descriptions, PLN-0043/PLN-0044 resources.

Implement QSPI indirect/custom/XIP modes, NVMC, FICR/UICR interfaces, ACL/MWU,
APPROTECT and CTRL-AP tooling. Keep flash command geometry in an explicit device
binding and reserve test/storage ranges. Preserve bootloader/SoftDevice regions
and distinguish normal programming from explicit provisioning/lock/recovery.
Review execution/interrupt and power-fail constraints before physical writes.

Exit: bounds, XIP lifetime, alias, protection and failure tests pass. External
flash and reserved internal regions support independently observed operations;
provisioning/debug-lock or destructive recovery remains separately requested and
recorded, never run merely to finish this planning item.

## PLN-0048 Implement I2S and PDM audio
@status(planned)
@link(REQ-0067, REQ-0062)

Prerequisite: reviewed audio descriptions and PLN-0043/PLN-0044 resources.

Implement supported I2S clock roles/formats and PDM capture, including clock
solving, sample packing, finite transfers, continuous fixed-buffer handoff,
underrun/overrun policy and safe stop. Supply explicit peer/microphone wiring
and standalone test applications; keep codec protocols outside the backend.

Exit: native boundary/event/errata tests, linked/package examples, independent
I2S TX/RX and PDM microphone observations pass with sample-rate and continuity
evidence. Acoustic performance is claimed only if independently measured.

## PLN-0049 Implement USB device operation and usable class examples
@status(planned)
@link(REQ-0068, REQ-0056, REQ-0062)

Prerequisite: reviewed USBD metadata and PLN-0043/PLN-0044 resources.

Implement power/clock startup, EP0 and all documented endpoint types, bounded
endpoint/DMA arbitration and reset/suspend/resume/error behavior. Add explicit
CDC ACM and HID compositions and an intentional, quiescent bootloader-entry
request using the existing UF2 layout. Retain default non-USB Blink builds.

Exit: endpoint/control tests and package consumers pass. Real-host enumeration,
CDC/HID exchange, endpoint-type exercises, suspend/resume and reconnect succeed;
bootloader entry and subsequent recovery are independently observed.

## PLN-0050 Implement RNG and every cryptographic accelerator block
@status(planned)
@link(REQ-0071, REQ-0062)

Prerequisite: reviewed complete crypto sources from PLN-0042 and
PLN-0043/PLN-0044 resources. Resolve any source/licensing gap before claiming exit.

Implement RNG, ECB, CCM, AAR and all documented CRYPTOCELL/CC_* engines and
control/DMA interfaces. Define fixed key/buffer storage, sharing, error handling
and cleanup. If an adapter is selected, pin its exact source/configuration and
prove restricted-runtime compliance; an enable-register-only implementation is
insufficient. Use published deterministic vectors and documented RNG health rules.

Exit: native bounds/failure/ownership tests, linked examples and on-device
known-answer/health tests cover the implemented engines. Required unresolved
engines stay planned; no blanket crypto or certification claim is inferred.

## PLN-0051 Implement the complete radio packet engine
@status(planned)
@link(REQ-0069, REQ-0062)

Prerequisite: reviewed RADIO/errata metadata, PLN-0043/PLN-0044 and required
crypto facilities from PLN-0050. Protocol-stack selection is independent.

Implement all hardware packet-mode/PHY families with explicit RF, buffer,
event/timing and resource contracts. Add native packet/bounds/disable-race tests
and independently controlled peer applications. Define an optional stack-binding
boundary that excludes conflicting direct ownership; do not silently start S140.

Exit: driver/API, cross-build and representative independent TX/RX/mode/PHY
observations pass. State peer/RF conditions and retain higher-level Bluetooth,
Thread/Zigbee/ANT behavior as separately selected integration work.

## PLN-0052 Implement NFC tag operation and field wake
@status(planned)
@link(REQ-0070, REQ-0062)

Prerequisite: reviewed NFCT metadata, PLN-0043/PLN-0044 and the explicit pin-role
configuration/provisioning contract from PLN-0047.

Implement field/activation/frame/timing/error handling and bounded tag example
storage, including field-loss cleanup and low power wake. Determine the actual
clone's NFC pad/antenna access and document any added matching/antenna assembly
before using it. Keep missing physical access visible without dropping NFCT.

Exit: native frame/event/lifetime tests and linked examples pass; an independent
NFC reader verifies tag exchange, field loss and wake through a documented antenna.
Unexecuted pin/provisioning/hardware observations remain explicitly pending.

## PLN-0053 Qualify the complete Pro Micro peripheral delivery
@status(planned)
@link(REQ-0074)

Prerequisite: implementation exits from PLN-0042 through PLN-0052; accumulate
physical observations during those stages rather than delaying all hardware work.

Reconcile every instance/register/field/mode and required erratum against the
source inventory. Run common/native/negative/header/M4F checks, source and
relocated-package examples, and concurrent legal multi-peripheral compositions.
Publish actual code/storage costs and an accessible API/example for every block.
Refresh the support matrix with separate implementation and verification states.

Exit: all required blocks have working implementations and no unsupported stubs
or unexplained exclusions. Record the software implementation gate separately
from complete board qualification. Physical acceptance remains pending for any
required unexecuted peer, pin, analog/RF/power/timing or recovery procedure;
missing access or equipment is recorded without changing the requirement scope.
