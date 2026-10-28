# Pro Micro nRF52840 requirements

Board identity: `others/promicro_nrf52840`. Target scope: implement every
programmable peripheral of the nRF52840 for the nice!nano-compatible Pro Micro
family. Preserve the UF2/S140-v6 application layout. V1940 remains an unverified
user-reported marking.

Scope extended by the user on 27 October 2026 (common decision D20).
REQ-0053/REQ-0054 retain their initial GPIO/I2C increment scope; REQ-0055 through
REQ-0074 define the complete-peripheral target. These are intended requirements,
not claims that the new drivers or hardware verification already exist.

Read this with the [common requirements](../../../../requirements.md), whose decision
register and shared contracts apply within the selected scope. The
[board implementation plan](implementation-plan.md) owns target-specific work;
[the common plan](../../../../implementation-plan.md) owns shared milestones.

## Applicable common contracts

| Area | Common requirements |
| --- | --- |
| Restricted runtime, layering and capability reporting | REQ-0002–REQ-0004 |
| Hardware metadata, pins, clocks, interrupts and memory | REQ-0005–REQ-0013 |
| Explicit startup ownership | REQ-0031 |
| Ownership, fixed transfer storage and deadlines | REQ-0014–REQ-0019 |
| Functional drivers and shared devices | REQ-0021–REQ-0030, REQ-0046 |
| Host verification and build/provenance workflows | REQ-0035–REQ-0039, REQ-0045–REQ-0052 |

This table is a reading guide, not an additional set of trace relationships or
a claim that every capability is implemented. Explicit dependencies remain on
the canonical requirement definitions below and in the common document.

The historical three-MCU description gate does not establish complete Nordic
coverage. GPIO Blink has a recorded physical observation; finite I2C has
software/build evidence. All additional work below is planned. Common first-release
defaults are extended here for the Nordic target, including hardware-supported
slave/target modes, streaming and peripheral-integrated DMA.

## Complete-peripheral scope and sources

The planning inventory is based on Nordic's [nRF52840 Product Specification](https://docs.nordicsemi.com/r/bundle/ps_nrf52840/page/keyfeatures_html5.html),
whose [revision history](https://docs.nordicsemi.com/r/bundle/ps_nrf52840/page/rev_history.html)
identifies v1.11 (October 2024), and its [peripheral instantiation table](https://docs.nordicsemi.com/r/bundle/ps_nrf52840/page/memory.html).
The existing [nrfx v3.2.0 MDK baseline](https://github.com/NordicSemiconductor/nrfx/tree/v3.2.0/mdk)
is a cross-check, not the completeness denominator for newer published registers.
The [silicon/document compatibility matrix](https://docs.nordicsemi.com/r/bundle/comp_matrix_nrf52840/page/comp/nrf52840/nrf52840_doc_ref_design_files_overview.html)
and [Revision 3 errata](https://docs.nordicsemi.com/r/bundle/errata_nrf52840_rev3/page/err/nrf52840/rev3/latest/err_840.html)
identify the source families to review after determining the actual chip revision;
the user's board is not assumed to be Revision 3. The
[Zephyr board definition](https://docs.zephyrproject.org/latest/boards/others/promicro_nrf52840/doc/index.html)
remains the initial wiring/boot-layout reference. Inventory review on
27 October 2026 does not constitute a complete register or errata audit.
REQ-0055 requires pinned, content-identified sources before each implementation.

| Peripheral group / all silicon instances | Requirement owner |
| --- | --- |
| GPIO P0/P1, including sensing and pad configuration | REQ-0053, REQ-0055, REQ-0057 |
| POWER, CLOCK, reset/retention and regulator controls | REQ-0056 |
| GPIOTE, PPI, EGU0–EGU5 and SWI0–SWI5 | REQ-0057 |
| TIMER0–TIMER4, RTC0–RTC2, WDT | REQ-0058 |
| UART0 and UARTE0/UARTE1 | REQ-0059 |
| SPI0–SPI2, SPIM0–SPIM3, SPIS0–SPIS2 | REQ-0060 |
| TWI0/TWI1, TWIM0/TWIM1, TWIS0/TWIS1 | REQ-0054, REQ-0061 |
| Peripheral-integrated EasyDMA and shared resource identities | REQ-0062 |
| PWM0–PWM3 | REQ-0063 |
| SAADC, COMP, LPCOMP, TEMP | REQ-0064 |
| QDEC | REQ-0065 |
| QSPI | REQ-0066 |
| I2S, PDM | REQ-0067 |
| USBD | REQ-0068 |
| RADIO | REQ-0069 |
| NFCT | REQ-0070 |
| RNG, ECB, CCM, AAR, CRYPTOCELL and its CC_* subblocks | REQ-0071 |
| NVMC, FICR, UICR, ACL, MWU, APPROTECT and CTRL-AP | REQ-0072 |
| Cortex-M4F core services, FPU interrupt, debug/trace and memory interconnect | REQ-0073 |
| Complete implementation and independently reported verification | REQ-0074 |

Every documented programmable instance, register, field and hardware-supported
mode belongs in the coverage inventory, including legacy serial aliases and
CryptoCell subblocks. Reserved IDs and nonexistent hardware are not peripherals.
Read-only information blocks receive typed query interfaces; control blocks
receive working configuration/operation interfaces. Shared-address aliases are
not independent simultaneously usable hardware.

Package/PCB pin availability, missing attached devices or lack of measurement
access constrain board compositions and physical verification, not the MCU driver
implementation commitment. Record those constraints per instance and retain
unexecuted tests as open work. Never substitute an empty type, register constants
alone, or a permanent unsupported result for a required working peripheral.

RADIO support covers the hardware packet engine and stack integration boundary.
Complete Bluetooth LE, Thread, Zigbee or ANT stacks are separate software
integrations; the preserved S140 binary is not an enabled stack. USB class
compositions and an NFC tag example exercise their respective peripheral drivers.
External devices, filesystems, audio codecs and network protocols remain outside
the peripheral implementation itself.

## Board-specific requirements

## REQ-0053 Pro Micro nRF52840 initial bare-metal integration
@depends(REQ-0010, REQ-0021, REQ-0031, REQ-0049)

Add the nice!nano-compatible Pro Micro nRF52840 board family identified by the
Zephyr `promicro_nrf52840` board definition. Initial support shall provide
source-backed GPIO metadata and sequencing, a GPIO-only I/O profile, an LED
Blink application, and an explicit supplied-startup build using the documented
UF2 / S140-v6 application partition. Preserve the SoftDevice, storage and UF2
bootloader regions. Do not silently enable a radio stack, change UICR, or claim
support for unimplemented peripherals or unverified clone revisions.

Generate a bounded, nRF52840-family UF2 file from the linked application without
accessing hardware. Document the bootloader handoff, fixed clock, pin identities,
recovery and explicit upload procedure. V1940 is a user-reported marking, not a
verified silicon or PCB revision.

Acceptance: native tests exercise GPIO ownership, atomic writes, electrical
configuration, failure handling and board composition. Cross-build and link
Blink from the checkout and an installed package; inspect vector/load addresses
and test UF2 payloads and partition rejection. Record physical LED observation
and bootloader recovery separately when the exact board is connected; build
success alone establishes no physical acceptance.

## REQ-0054 Pro Micro nRF52840 finite I2C controller and shared devices
@depends(REQ-0053, REQ-0024, REQ-0025, REQ-0026, REQ-0046)

Extend the initial Pro Micro profile with source-backed CPU-driven TWI0/TWI1
controller bindings, finite seven-bit writes, reads and repeated-START combined
transactions. Preserve native pin selection and the shared TWI/SPI hardware
instance identity. Support the documented 100/250/400 kHz encodings with explicit
timing assumptions; reject unavailable or duplicate pins and invalid requests.

Integrate with the existing I/O bus, device leases and TMP102 driver, without a
heap, vendor HAL dependency or generic DMA claim. Report observable address/data
NACK and transport errors, detect idle stuck lines, bound cancellation cleanup,
and prohibit reuse after failed cleanup. Release hardware only after explicit
successful shutdown; recovery then requires a fresh backend/runtime, preserving
the existing retired-operation-slot contract. Do not pulse external bus lines automatically or claim
multi-controller arbitration support.

Acceptance: native traces and deterministic fake peripherals exercise initial
configuration, repeated START, final-byte STOP, errors, stale events, ownership,
timeout/cancel and MMIO failures. Test shared-address leases and a real TMP102
composition with a simulated peripheral. Link complete sensor applications from
source and a relocated installed package. Wire protocol, electrical timing and
physical sensor results remain unverified until an actual fixture is exercised.

## REQ-0055 Complete nRF52840 descriptions, capabilities and errata baseline
@depends(REQ-0003, REQ-0005, REQ-0006, REQ-0007, REQ-0053)

Implement reviewed typed metadata and usable native interfaces for every block
and instance in the inventory above. Record register access semantics, reset
values, tasks/events/shortcuts, interrupts, clocks, pin/package constraints,
memory access and shared identities. Extend the existing GPIO/TWI slice without
forcing Nordic differences into another MCU's layout. Represent all documented
hardware modes; generic functional APIs may be complemented by typed native APIs.

Pin product-specification, silicon/package, errata and board source identities
with content hashes and section references. Determine errata applicability from
actual part/build information; provide required initialization/workarounds in
supplied startup or explicit external-startup contracts. Unavailable source or
unidentified revision remains a visible gap and prevents a complete claim.

Acceptance: a source-to-register/field/instance/mode inventory has no unexplained
omissions; each implemented slice passes positive/negative metadata checks,
isolated headers and Cortex-M4F probes. Revalidate affected common contracts and
other MCU descriptions after shared model changes.

## REQ-0056 Power, clocks, reset and low power operation
@depends(REQ-0055, REQ-0011, REQ-0031)

Implement POWER/CLOCK control for documented oscillator sources, source-ready
coordination, LFRC calibration, clock requests and release, reset causes,
GPREGRET retention, supply/power-fail events, applicable regulator/DC-DC controls,
and per-section RAM power/retention. Respect the actual board circuitry and
UICR configuration. Provide System ON idle and System OFF entry with explicit
wake sources, retained-state and reset/resume semantics.

Shared clock clients shall prevent premature shutdown while USB, radio, timers
or transfers depend on a source. Sleep entry shall preserve wake events and
reject incompatible active work; no operation may silently invalidate borrowed
buffers or its deadline source.

Acceptance: native tests cover source transitions, failed starts, references,
wake races and retention bounds. Hardware records exercise applicable wake/reset
paths and measure current with debugger/USB/board-load conditions stated.

## REQ-0057 GPIO events, interrupts and programmable event routing
@depends(REQ-0055, REQ-0056, REQ-0012, REQ-0014, REQ-0016)

Complete GPIO configuration/sensing/latch behavior and implement GPIOTE task,
edge and port-event operation, PPI channels/groups/forks, EGU0–EGU5 and
SWI0–SWI5. Use fixed ownership of pins, channels, groups, event/task endpoints
and shared interrupt sources. Validate legal routes, edge polarity and endpoint
lifetime; disable routes before releasing their source, destination or buffer.
Provide bounded interrupt dispatch and explicit interrupt-priority integration.

Default board reservations for crystal, NFC and reset pads remain enforced.
Alternative documented roles require an explicit board configuration, with UICR
changes handled under REQ-0072 rather than performed by ordinary pin acquisition.

Acceptance: test edge/level handling, latch clearing, shared-vector dispatch,
stale events, route conflicts and teardown races. Physically demonstrate GPIO
interrupts and a timer/event-to-pin route without foreground polling.

## REQ-0058 Timers, real-time counters and watchdog
@depends(REQ-0056, REQ-0057, REQ-0073)

Implement TIMER0–TIMER4 timing/counter modes, supported widths, prescalers,
capture/compare and shortcuts; RTC0–RTC2 tick/compare/overflow operation; and WDT
reload channels, timeout, sleep/debug behavior and reset consequences. Expose
instance-specific channel limits and hardware restrictions without inventing
stop or reconfiguration facilities absent from the silicon.

Provide a monotonic execution-clock/alarm composition supporting long waits and
low power operation, extending beyond the initial short DWT delay contract.
Reserve execution/stack-owned timers explicitly and preserve overflow/deadline
semantics across supported sleep states.

Acceptance: native tests cover boundaries, wrap, compare races and ownership.
Physical records cover timer/counter/capture, RTC wake/long waits and watchdog
reset cause, including its irreversible-until-reset behavior where applicable.

## REQ-0059 UART and UARTE serial communication
@depends(REQ-0055, REQ-0056, REQ-0057, REQ-0062, REQ-0027)

Implement UART0 and UARTE0/UARTE1 with validated native pin routing, baud/parity,
flow control and supported transfer modes. Provide finite TX/RX and explicitly
bounded continuous receive using caller-owned buffers; expose actual byte
counts, receive-idle/stop behavior and observable serial errors. UART0/UARTE0
shall share one physical ownership identity.

Use the common serial/transfer facade for compatible operations, preserving
interrupt/polling completion, deadlines and cancellation. TX completion shall
include the final wire event; RX buffers remain owned until hardware stops.

Acceptance: test baud bounds, RX partial/error paths, flow-control stalls,
continuous-buffer handoff and late interrupts. Cross-build examples and verify
bidirectional physical serial traffic, supported flow control and error recovery.

## REQ-0060 SPI master and slave implementations
@depends(REQ-0055, REQ-0056, REQ-0057, REQ-0062, REQ-0028, REQ-0046)

Implement legacy SPI0–SPI2, EasyDMA SPIM0–SPIM3 and SPIS0–SPIS2. Support all
hardware SPI modes, supported bit orders and rates, full/half-direction transfer
compositions, chip-select lifetime and instance-specific features such as SPIM3
high-speed operation. Master multi-segment commands preserve ownership and CS
as required by the selected transaction.

SPIS shall expose bounded armed TX/RX buffers, hardware semaphore ownership,
actual counts, default/over-read behavior and completion/abort semantics.
Slave operation is an explicit extension of the common master-only baseline.
Apply alias exclusion and all revision-specific transfer/event workarounds.

Acceptance: test modes/rate selection, asymmetric lengths, CS and semaphore
ordering, overruns and abort races. Physically verify master traffic with a
slave/loopback and slave traffic with an independent master, including timing.

## REQ-0061 Complete I2C master and target support
@depends(REQ-0054, REQ-0055, REQ-0056, REQ-0057, REQ-0062)

Retain CPU-driven TWI0/TWI1 and implement TWIM0/TWIM1 plus TWIS0/TWIS1.
Master operation shall support validated timing, finite and combined
repeated-START transactions, DMA limits and observable errors. TWIS shall
support the documented address slots, bounded receive/transmit buffer arming,
address/direction events, actual counts and target clock-stretch behavior.
Expose native distinctions rather than promising unsupported ten-bit addressing
or arbitration features. Apply the relevant TWIM timing errata.

All personalities of a shared TWI/SPI instance shall use the same hardware
ownership contract. Preserve existing device leases and explicit recovery policy.

Acceptance: native tests cover alias conflicts, boundary lengths, NACK/STOP
ordering, stretching and target buffer exhaustion. Physical master/sensor and
independent-controller-to-TWIS tests verify transactions and cancellation.

## REQ-0062 Peripheral DMA and shared hardware resource ownership
@depends(REQ-0055, REQ-0013, REQ-0014, REQ-0017, REQ-0018, REQ-0019)

Implement each documented peripheral DMA path, including applicable EasyDMA
channels/list modes and the crypto engines' own DMA interfaces. Validate
RAM reachability, alignment, per-instance count widths and actual transferred
counts. Streaming/double-buffer operation uses explicit fixed capacity and
backpressure/overrun policy. Flash-backed TX data requires a documented bounded
copy or rejection. There is no invented generic memory-to-memory DMA engine.

Model true mutual exclusion separately from independent controls sharing an
address block. Cover serial personalities, COMP/LPCOMP, CCM/AAR, EGU/SWI and
NVMC/ACL/control-block overlap as their documented semantics require. Coordinate
shared IRQs, pins, clocks, PPI and DMA users across polling and ISR paths.
Success, cancellation or failure shall not release memory before hardware quiesces.

Acceptance: test all DMA-capable instances/directions, boundary and invalid
buffers, partial completion, restart, teardown failures and simultaneous legal
clients. Physical checks distinguish DMA completion from final peripheral activity.

## REQ-0063 Complete PWM implementation
@depends(REQ-0056, REQ-0057, REQ-0062, REQ-0022)

Implement PWM0–PWM3 with their four outputs, supported counter/alignment modes,
clock/top/polarity selection, decoder and sequence modes, repeats/loops,
shortcuts, DMA sequence ownership and bounded runtime updates. Provide basic
PWM through the common facade and typed access to native sequence features.
Hardware lacks a generic promise of complementary motor pairs/dead time;
advanced motor behavior shall only be exposed where actually supported.

Acceptance: test timing bounds, output mapping, sequence transitions, false/stale
completion handling and stop/update races. Measure frequency, duty, alignment and
representative sequence behavior on accessible board pins.

## REQ-0064 ADC, comparators and internal temperature
@depends(REQ-0056, REQ-0057, REQ-0062, REQ-0029)

Implement SAADC single-ended/differential channels, supported resolution,
reference/gain/acquisition choices, calibration, oversampling/burst, finite scan,
continuous buffered sampling and limit events. Preserve channel ordering,
signed representation and calibrated conversion assumptions. Implement COMP and
LPCOMP thresholds/references, hysteresis, events and wake features, with exclusive
ownership where their hardware resources overlap. Implement TEMP conversion and
factory-calibration use where required by the source/errata.

Acceptance: test configuration limits, calibration sequencing, DMA handoff,
thresholds, stop/restart and power cleanup. Hardware checks use known analog
levels, comparator transitions/wake and an independently observed temperature;
record reference uncertainty rather than implying unmeasured precision.

## REQ-0065 Quadrature decoder
@depends(REQ-0056, REQ-0057)

Implement QDEC input/LED routing, sample and report intervals, debounce options,
accumulation/read-clear behavior, direction, overflow/double-transition reporting
and supported event/task operation. Expose bounded reads and event delivery with
explicit reset and ownership semantics.

Acceptance: native sequences cover both directions, invalid/double transitions,
accumulator boundaries and concurrent read/event handling. A controlled two-phase
source or encoder verifies physical counting and direction on accessible pins.

## REQ-0066 QSPI external memory interface
@depends(REQ-0056, REQ-0057, REQ-0062)

Implement QSPI pin/clock/protocol configuration, activation/deactivation,
custom instructions, supported read/write/erase operations and memory-mapped
XIP configuration. Keep flash-device command geometry/timing in an explicit
attached-device configuration. Model XIP and indirect access ownership so erase,
program or deactivation cannot invalidate executing code or borrowed mappings.
Apply silicon-specific XIP and resource-shutdown workarounds.

Acceptance: test command encodings, address/length/alignment bounds, busy/error
and cancellation behavior. With a compatible external flash and verified wiring,
exercise identification, read/write/erase and XIP in a reserved test region,
restoring contents where the procedure requires it.

## REQ-0067 I2S and PDM audio interfaces
@depends(REQ-0056, REQ-0057, REQ-0062)

Implement I2S supported controller/target clock roles, formats, alignment,
channel/sample widths, clock ratios and TX/RX operation. Implement PDM microphone
clock, edge/channel selection, gains and sample capture. Provide finite and
continuous caller-buffered operation, explicit underrun/overrun policy and
safe stop at documented buffer boundaries. Metadata shall reflect actual
per-instance sample packing and limits.

Acceptance: test clock solving, packing, DMA-buffer handoff, stale pointer-update
events and shutdown workarounds. Verify I2S transmission/reception with an
independent peer and PDM capture with a compatible microphone; record sample
rate and buffer continuity separately from acoustic quality.

## REQ-0068 USB device peripheral and application compositions
@depends(REQ-0056, REQ-0057, REQ-0062, REQ-0031)

Implement USBD full-speed device operation: power/clock coordination, EP0 setup
and control transfers, documented endpoint types/directions including isochronous
operation, endpoint buffers/DMA arbitration, reset, stall, suspend/resume,
remote wake when enabled and disconnect/reconnect. Enforce bounded descriptors,
buffers and endpoint ownership with revision-specific errata handling.

Provide fixed-storage CDC ACM serial and HID example compositions through an
explicit class layer. USB application startup shall coexist with the preserved
UF2 bootloader layout. Expose an intentional bootloader-entry request with a
quiescent handoff; ordinary serial traffic shall not silently reset the board.

Acceptance: test control/endpoint/error state machines and DMA/IRQ races. A real
host verifies enumeration, CDC traffic, HID reports, endpoint-type exercises,
suspend/resume and reconnect; bootloader handoff/recovery receives its own record.

## REQ-0069 Radio packet engine and protocol integration boundary
@depends(REQ-0056, REQ-0057, REQ-0062, REQ-0071)

Implement RADIO TX/RX for all documented hardware packet modes, including BLE
PHY encodings, IEEE 802.15.4 and proprietary modes. Provide channel/rate/power,
address/packet layout, CRC/whitening, RSSI, applicable CCA/energy detection,
timing/shortcuts, DMA-buffer and event/error handling with source-backed limits.
Respect oscillator requirements, radio-specific DMA rules and revision workarounds.

Expose explicit ownership of RADIO, timers, PPI and crypto resources for an
optional protocol stack. Direct bare-metal ownership and SoftDevice/other-stack
ownership shall be mutually coordinated; preserving S140 does not authorize
concurrent direct register access. Stack selection and protocol behavior are
separate integration milestones and do not replace the packet-engine driver.

Acceptance: native tests cover packet bounds, mode changes and cancel/disable
races. Compatible independent peers verify TX/RX and representative mode/PHY
paths; record packet errors and actual RF setup without implying certification.

## REQ-0070 NFC tag peripheral
@depends(REQ-0056, REQ-0057, REQ-0062, REQ-0072)

Implement NFCT field detection, activation/selection, frame TX/RX, automatic
framing/CRC/parity facilities, buffer limits, frame-delay configuration, sleep,
errors and field-loss cleanup. Provide a bounded NFC-A Type 2 tag example above
the peripheral transport. Expose supported low power field-wake behavior.

NFC pads, antenna/matching and UICR pin role shall be explicit board
configuration. Missing antenna wiring does not remove NFCT from the driver
scope; repurposing pins must not happen during normal GPIO initialization.

Acceptance: tests cover event/frame ordering, malformed/oversize reception,
timeout, field loss and stop. An appropriate antenna and independent NFC reader
verify activation, example exchange and wake behavior with board changes recorded.

## REQ-0071 Random generation and cryptographic peripherals
@depends(REQ-0055, REQ-0056, REQ-0057, REQ-0062)

Implement RNG generation/configuration and ECB, CCM and AAR operations with
validated key/input/output layout, counts, completion/errors and explicit secret
buffer lifetime. Serialize shared CCM/AAR resources. Implement CRYPTOCELL control
and every documented CC_* subblock: AES, CHACHA, CTL, DIN, DOUT, HASH, HOST_RGF,
MISC, PKA, RNG and RNG_SRAM. Provide working operations for the documented
accelerator functions, including associated DMA, interrupts and power lifecycle.

Pin authoritative register/API sources and any separately selected software
adapter dependency. Adapters shall preserve fixed storage and the restricted
runtime; neither an enable-register wrapper nor an unexamined vendor-library
placeholder satisfies CryptoCell implementation. Source/licensing gaps remain
open work; full-peripheral acceptance cannot silently exclude this block.

Acceptance: published known-answer vectors exercise deterministic functions,
boundary/error cases and shared ownership. Test RNG lifecycle and documented
health/error handling, retaining statistical or security-certification claims as
separate evidence. Run on-device vectors without exposing key material in logs.

## REQ-0072 Flash, information, protection and memory monitoring
@depends(REQ-0055, REQ-0056, REQ-0014, REQ-0019, REQ-0031)

Implement NVMC bounded programming/erase and readiness handling; typed read-only
FICR identity/calibration queries; explicit UICR provisioning; ACL region access
controls; MWU region monitoring/events; and revision-aware APPROTECT/CTRL-AP
configuration/status and debugger recovery operations. CTRL-AP is a debugger
interface, not a fictitious CPU peripheral; provide the corresponding tooling
and documented access contracts.

The default application may modify only its explicitly assigned writable region.
Retain MBR/SoftDevice, storage and bootloader boundaries. UICR/pin-role changes,
debug lock and mass erase are explicit provisioning/recovery actions with their
own requested ranges, effects and recovery procedure; ordinary begin/flash does
not perform them. Coordinate flash operations with power, execution and interrupts.

Acceptance: test range/alignment/protection and failure handling without touching
protected data. Exercise reversible flash/MWU/ACL behavior in reserved regions;
record separately executed provisioning/debug-lock/recovery tests and their
board/source identities, preserving existing bootloader images.

## REQ-0073 Core services, debug/trace and memory integration
@depends(REQ-0055, REQ-0009, REQ-0012, REQ-0031)

Complete explicit Cortex-M4F NVIC, SCB/vector/fault, SysTick, FPU and FPU-interrupt,
MPU, DWT, ITM, ETM, TPIU and FPB integration as present in the selected silicon.
Provide usable configuration/query interfaces, fixed ownership and safe default
fault reporting. Debugger-controlled functions may use explicit tooling rather
than CPU-side drivers. Describe AHB and code/data RAM aliasing and apply required
barriers and DMA ordering; interconnect description alone is not a new DMA engine.

Supplied startup shall identify its silicon/errata responsibilities and reserved
core resources. External BSP/RTOS startup shall retain ownership through explicit
bindings. Unrouted trace pins constrain physical capture, not metadata support.

Acceptance: compiler/assembly checks and native tests cover priority, vector,
MPU-range and time-wrap rules. Physical tests exercise fault/interrupt entry,
FPU, MPU and available debug/trace paths with resources and unavailable pins stated.

## REQ-0074 Complete-peripheral delivery and board qualification
@depends(REQ-0053, REQ-0054, REQ-0055, REQ-0056, REQ-0057, REQ-0058, REQ-0059, REQ-0060, REQ-0061, REQ-0062, REQ-0063, REQ-0064, REQ-0065, REQ-0066, REQ-0067, REQ-0068, REQ-0069, REQ-0070, REQ-0071, REQ-0072, REQ-0073, REQ-0035, REQ-0036, REQ-0037, REQ-0038, REQ-0039)

Publish a per-instance/mode matrix distinguishing source review, typed metadata,
working backend/API, native tests, cross-build/link, and physical acceptance.
Every required peripheral shall have a functional implementation, explicit
resource/clock/pin contracts and a complete example or test composition. Existing
facades shall cover compatible operations; additional capabilities shall have
usable documented native/functional interfaces without forcing unrelated APIs.
Keep inclusion/storage selectable and record actual flash/static-RAM budgets.

Complete implementation requires every inventory row and its documented modes
accounted for, including errata workarounds and normal/error/cleanup behavior.
Permanent unsupported stubs, unavailable sources or missing drivers keep the
implementation milestone open. Physical qualification is reported separately:
connected-board behavior, inaccessible pads, absent peers and unmeasured electrical
properties retain their exact status and cannot be relabeled as tested.

Acceptance: all inventory entries have requirement/plan/evidence links; shared
native, negative compiler, isolated-header, Cortex-M4F, source/installed-package
and representative multi-peripheral compositions pass. Independently execute
the specified physical procedures before claiming complete board qualification;
retain pending observations explicitly without reopening the earlier increments.
