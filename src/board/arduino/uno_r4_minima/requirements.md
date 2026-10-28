# Arduino UNO R4 Minima requirements

Board identity: `arduino/uno_r4_minima`. Scope: RA4M1 / R7FA4M1AB3CFM#AA0; the first bare-metal release and the W5100 Ethernet Shield R3 extension.

Read this with the [common requirements](../../../../requirements.md), whose decision
register and shared contracts apply within the selected scope. The
[board implementation plan](implementation-plan.md) owns target-specific work;
[the common plan](../../../../implementation-plan.md) owns shared milestones.

## Applicable common contracts

| Area | Common requirements |
| --- | --- |
| Description, package and provenance | REQ-0005–REQ-0013; REQ-0040 |
| Initial GPIO, PWM, I2C, UART, SPI, ADC and DMAC release | REQ-0021, REQ-0022, REQ-0024–REQ-0030; REQ-0041 |
| Startup, physical acceptance and resource measurements | REQ-0031, REQ-0035–REQ-0038 |
| Application facade, attached devices and selected-board builds | REQ-0045–REQ-0052 |

This table is a reading guide, not an additional set of trace relationships or
a claim that every capability is implemented. Explicit dependencies remain on
the canonical requirement definitions below and in the common document.

The representative physical release scope remains D04/D14 in the common decision register. W5100 networking is a follow-on extension and does not reopen that release.

## Board-specific requirements

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
The adapter and combined test application shall reside in `../integration`,
with no dependency from mcubsp back to that project.
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
