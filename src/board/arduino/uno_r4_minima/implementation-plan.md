# Arduino UNO R4 Minima implementation plan

Implements the [board requirements](requirements.md) and their applicable
[common contracts](../../../../requirements.md). Shared foundation, description-gate,
application/device, repository and later MCUOS work stays in the
[common implementation plan](../../../../implementation-plan.md).

The items below retain their existing IDs, explicit requirement links, status,
prerequisites and verification notes. Read them in delivery order, not numeric
ID order. MCU description/backend work is owned here for this initial carrier
but remains reusable by other boards on the same MCU.

The representative physical release scope remains D04/D14 in the common decision register. W5100 networking is a follow-on extension and does not reopen that release.

A `done` status records the stated increment only. Physical observations and
unexecuted acceptance cases remain distinct in each item and in the
[support matrix](../../../../doc/reference/support-matrix.md).

## PLN-0004 Describe RA4M1 foundational resources and initial peripherals
@status(done)
@link(REQ-0005, REQ-0006, REQ-0007, REQ-0009, REQ-0010, REQ-0011, REQ-0012, REQ-0013, REQ-0040)

Prerequisite: PLN-0003.

- Describe device/package identity, memory, clock/module-stop/reset/protection data, ICU events, DMAC/DTC constraints and PFS routing.
- Describe GPIO, GPT, IIC, SCI/UART, SPI, ADC and DMAC registers, fields, capabilities and dependencies. Separate GPT instances from actual A/B output identities.
- Audit addresses, widths, reset masks, side effects, protection rules and package availability against source tables and usage notes. Record unresolved discrepancies explicitly.
- Instantiate the common tests with real RA4M1 data. Board aliases may be declarative, but do not add startup, ISR dispatch or functional peripheral code.

Exit: RA4M1's foundational and initial-peripheral descriptions are reviewed and pass metadata tests; the remaining inventory is assigned to PLN-0013.

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
   MCUIP into mcubsp, add lwIP or depend on the Arduino Ethernet library.
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
