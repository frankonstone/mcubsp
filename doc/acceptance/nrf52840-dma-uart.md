# nRF52840 coordination and initial UART software checkpoint

PLN-0044 and PLN-0045 are **in progress**. This checkpoint delivers the shared
coordination foundation and the first native legacy UART0 backend. Concrete
peripheral DMA drivers, complete serial families and physical qualification
retain their own pending scope.

The source baseline is Nordic PS 4413_417 v1.11, identified in the existing
[manifest](../reference/nrf52840/product-specification.json). Relevant contracts
are sections 4.6/6.1.1–6.1.2, the current serial MAXCNT tables, CryptoCell
6.6.13.2/3, UART 6.33 and UARTE 6.34. Existing stepping/startup gaps from
PLN-0042 remain open. Reviewing these interfaces does not identify this board's
silicon revision.

## Delivered software

- One shared resource registry with generation-stamped claims, reference holds,
  atomic multi-pin reservations and explicit ownership errors. All serial
  personalities, COMP/LPCOMP, AAR/CCM and EGU/SWI share their configuration
  identities. POWER/CLOCK/APPROTECT and NVMC/ACL keep independent controls;
  flash mutation/permission operations additionally need the explicit
  `flash_mutation` guard in their eventual drivers.
- Existing TWI, GPIO and native system clients use the registry. Native PPI/core
  channel reservations retain their existing finer-grained context identities.
  Registry claims do not discover raw MMIO users or make an external BSP safe
  to reconfigure. Source-backed disable/PPI/IRQ/enable ordering remains a driver
  responsibility before ownership is released.
- A bounded DMA buffer pool retains resource ownership until a caller-supplied
  nonblocking observation confirms hardware quiescence. It checks capacity,
  count units, memory reachability/alignment, conflicting spans, exhaustion,
  stale generations and failed recovery. Readers may share; overlapping writes
  are excluded. One pool per registry prevents independent overlap domains.
  Destruction with live leases invokes the execution policy's lifetime failure
  hook and retains claims. Caller buffers and owners must outlive every lease.
- Serial EasyDMA constraints are distinct from CryptoCell's 30-bit byte sizes
  and busy/command protocol. Crypto algorithms supply alignment and completion
  rules; internal RNG-SRAM paths and actual crypto operations remain for their
  driver stage. DIN SRC_MEM_SIZE is a start command, and FIRST/LAST in DIN/DOUT
  memory-size words are excluded from writable masks by the detailed prose.
- UART/UARTE baud tables preserve their different encodings and documented
  actual rates. Native bindings validate distinct GPIOs, paired CTS/RTS, formats
  and low-frequency-only pads. The CPU-driven UART backend accepts UART0 only;
  compile-time checks reject a UARTE binding there.
- Native UART0 finite TX/RX uses existing transfer handles. TX waits for the
  final TXDRDY and issues STOPTX before success. RX clears RXDRDY before RXD,
  waits for RXTO and reports excess FIFO tails as overruns. Errors preserve
  the count copied into the caller buffer. Timeout/cancel has a wrapping-clock
  deadline and a second finite poll budget; a late original RXTO remains usable
  during recovery. ISR fault/cancel paths observe stop once and defer any pending
  hardware recovery to foreground shutdown. Failed CPU cleanup detaches spans and retains hardware
  ownership until explicit shutdown. Reconstruct a faulted backend after
  successful shutdown. Polling and caller-owned interrupt service use the same
  state machine, with separate TX/RX operation slots and no queue.
  `received_count()` reports the copied prefix and `error_source()` retains all
  observed UART error bits even when the common result reports one primary cause.

## Composition and remaining scope

Use one `resource_registry`, one compatible execution policy and one shared
GPIO backend per physical MCU. Constructors without an explicit registry use
the fixed, constant-initialized compatibility registry. Independent simulated
MCUs can supply separate registries. Configuration/pin claims/teardown are
foreground-only; buffer retains, handoffs and peripheral service can be used
under the shared critical-section policy. A quiescence callback must not wait
for IRQ progress while that policy is masked.

UART0 initialization borrows an already initialized native clock controller and
holds HFXO until shutdown. Initialize the DWT execution clock or supply a bounded
borrowed clock. The default stop interval is 200 ms in 64 MHz cycle-clock units;
other execution clocks provide their own interval. ISR service requires an
explicit dispatcher subscription and interrupt enable. Shutdown refuses active
operations; consume/cancel handles while the backend and buffers remain alive.
Remove the caller-owned IRQ subscription before destroying the backend or its
callback state.
Polling users explicitly call `poll()` while waiting, as in the example. A
transfer's wait method requires IRQ service or a progress-capable execution
adapter; the default cycle-clock wait hook does not poll this peripheral.

The board facade remains GPIO and finite I2C. Native UART0 is usable explicitly;
UARTE0/1 runtime, bounded streaming receive, complete SPI master/slave and
TWIM/TWIS runtimes remain pending. This coordination pool is not a generic DMA
engine and does not establish per-peripheral DMA stop/completion correctness.
All new UART/coordination images have software evidence only. The earlier
[RTC/watchdog observations](nrf52840-system.md) retain their original image hashes.

The [loopback example](../../example/uart-loopback/README.md) provides a linked
UART0 composition. Physical TX/RX, flow-control peers, independent baud timing,
all configurations and silicon-specific qualification remain open. Current
verification and image identities are recorded in the
[software evidence record](evidence/nrf52840-dma-uart/2026-10-28-software.json).

## Verification run on 28 October 2026

All 65 CTest entries and 418 merged native cases pass. The increment adds 16
coordination cases, three UART configuration cases and 14 native UART cases.
All 245 public headers and actual service-body probes compile on host,
Cortex-M4, Cortex-M7 and ESP32-P4. MDK/product/core imports reproduce offline.
Blink, TMP102, both system variants and the UART loopback link and emit UF2.
Source/relocated-package system and UART builds pass, including installed
metadata/service probes, retained license notices, one startup handler per
checked consumer, allocation/exception symbol checks and fake-mount runner
planning. Linked UART section sizes are 10,448 bytes text, 20 data and 4,548 BSS;
these do not measure peak stack or runtime throughput. No new firmware was
uploaded during this software increment. Hosted CI and coverage were not run.
