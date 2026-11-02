# nRF52840 peripheral software checkpoint

This 1 November 2026 increment implements part of PLN-0044–PLN-0053. It does
**not** complete those ten items. Hardware is unavailable for this work; no
new image upload, electrical measurement or peripheral-peer result is claimed.
Earlier image-bound observations retain their original hashes and scope.

The [2 November serial delivery](nrf52840-serial.md) supersedes this record's
serial backlog and facade availability statements. The following interface and
verification details describe the original 1 November checkpoint.

The [2 November native delivery](nrf52840-native-delivery.md) supersedes the
remaining implementation backlog below. This file remains the historical
1 November checkpoint; it is not evidence for the newer images.

## Implemented interfaces

- `backend/nrf52840/uarte.hpp`: finite UARTE0/UARTE1 EasyDMA TX/RX with
  independent operation handles, hardware flow-control bindings, actual counts,
  explicit partial-RX stop, IRQ/poll service, deadline cancellation and failed
  stop recovery. TX waits for TXSTOPPED. RX waits for RXTO and flushes residual
  FIFO data to a private five-byte DMA buffer; excess data remains an observable
  overrun. Failed cancellation retains memory until `shutdown()` succeeds.
- `backend/nrf52840/spim.hpp`: finite SPIM0–SPIM3 transactions, all SPI modes
  and bit orders, optional data/CS pins, asymmetric counts, ORC and fixed-capacity
  segment storage with CS held across segments. STOPPED precedes returning
  memory or deasserting CS. SPIM3 additionally requires an explicitly dedicated
  whole RAM AHB slave and reviewed silicon words; the pool excludes other DMA
  clients from that region. The application must keep CPU/ISR accesses through
  both RAM aliases out of that slave while transmitting. SPIM3 shutdown applies
  erratum 195. PPI and suspension are not exposed by this increment.
- `backend/nrf52840/twim.hpp`: TWIM0/TWIM1 finite write/read and both orders
  of repeated-START transactions, actual counts, distinct NACK causes, shared
  serial/pin/clock ownership and STOPPED-gated memory release. Timing uses
  100/250/390 kHz nominal settings; the 390 kHz encoding applies erratum 219.
  A requested 400 kHz rate therefore needs at least 25,000 ppm tolerance.
  Low-frequency-only die pads reject fast bus configuration.
- `backend/nrf52840/temperature.hpp`: one-shot signed quarter-degree TEMP
  measurements, HFXO ownership, 17 factory linearity coefficients for erratum
  66, fixed transfer storage and cancellation. Erased calibration data fails
  initialization. Conversion uncertainty remains a physical measurement item.
- `backend/nrf52840/qdec.hpp`: validated phase/optional LED routing, all native
  sample/report periods, debounce, polarity/precharge configuration, signed
  read/clear snapshots, double-transition/overflow reporting and bounded stop.
  Generation-stamped PPI endpoints hold the shared resource until route teardown;
  CPU snapshot tasks and shutdown refuse live routes. Debounce keeps a selected
  QDEC LED on continuously, as specified by the peripheral.
- `backend/nrf52840/rng.hpp`: finite CPU-buffered RNG generation, explicit bias
  correction and VALRDY_STOP one-byte handoff, bounded service and cancellation.
  This is the standalone RNG peripheral; CryptoCell RNG/health interfaces remain
  separate work. Host tests do not establish entropy quality or certification.
- `backend/nrf52840/nvmc.hpp`: explicitly reserved-region word programming and
  page erase, copied protected ranges, alignment/readiness checks, one program
  per erased word, readback and shared `flash_mutation` ownership. POFWARN is
  checked and temporarily disabled through the existing power owner for erratum
  242. A failed READY wait retains mutation/power ownership until recovery;
  successful recovery returns NVMC to read-only and restores the previous POF
  configuration. No UICR, debug-lock or mass-erase operation is implicit.

UARTE and TWIM can use the compatible common `driver::uart` and
`driver::i2c_controller` wrappers. SPIM uses its native asymmetric segment API.
The board `io` profile remains GPIO/CPU-driven I2C; these native controllers do
not silently enable unsupported board facade features. Use one context,
compatible execution policy, GPIO owner and DMA buffer pool per physical MCU.
Do not access caller DMA storage until its operation is consumed, or until
explicit shutdown succeeds after a failed cancellation. Keep all backend and
callback storage alive throughout that recovery.

The callable address policy supports simulated address spaces. Native use keeps
the real pointer; all translated pointers still pass peripheral-specific Data
RAM/count/alignment and pool-overlap checks. Flash-backed TX spans are rejected.

## Source and revision boundaries

The baseline remains the pinned Nordic PS 4413_417 v1.11, sections 4.3, 4.6,
4.7, 6.18, 6.21, 6.25, 6.28, 6.31 and 6.34. The pinned nrfx v3.2.0
`nrf52_erratas.h` was fetched for read-only cross-check and matched its existing
manifest SHA-256. `serial_revision.hpp` uses its FICR 0x130/0x134 words,
separately from INFO.VARIANT and ROM identification. Explicitly reviewed modern
values enable 16-bit UARTE counts/two stop bits. Unknown/Engineering A values
retain 255-byte/one-stop-bit limits; affected single-byte SPIM cases are rejected
and SPIM3 initialization fails closed. This does not identify the actual board
or close PLN-0042's full source/stepping/errata gate.

Relevant Nordic errata:
[58](https://docs.nordicsemi.com/r/bundle/errata_nrf52840_enga/page/err/nrf52840/engineeringa/latest/anomaly_840_58.html),
[66](https://docs.nordicsemi.com/r/bundle/errata_nrf52840_rev3/page/err/nrf52840/rev3/latest/anomaly_840_66.html),
[127](https://docs.nordicsemi.com/r/bundle/errata_nrf52840_enga/page/err/nrf52840/engineeringa/latest/anomaly_840_127.html),
[131](https://docs.nordicsemi.com/r/bundle/errata_nrf52840_enga/page/err/nrf52840/engineeringa/latest/anomaly_840_131.html),
[195](https://docs.nordicsemi.com/r/bundle/errata_nrf52840_rev3/page/err/nrf52840/rev3/latest/anomaly_840_195.html),
[198](https://docs.nordicsemi.com/r/bundle/errata_nrf52840_rev3/page/err/nrf52840/rev3/latest/anomaly_840_198.html),
[219](https://docs.nordicsemi.com/r/bundle/errata_nrf52840_rev3/page/err/nrf52840/rev3/latest/anomaly_840_219.html),
[242](https://docs.nordicsemi.com/r/bundle/errata_nrf52840_rev3/page/err/nrf52840/rev3/latest/anomaly_840_242.html).

NVMC users must reserve a nonexecuting storage area in their linker layout and
pass the actual protected ranges. The supplied Pro Micro profile keeps
MBR/SoftDevice below 0x26000, application flash below 0xEC000, existing storage
at 0xEC000–0xF4000 and bootloader/settings at and above 0xF4000. The example
probes' 0xE0000 page is a simulated/compile-only test range, not a grant to
overwrite that address in an arbitrary application. Interrupts may be delayed
by flash operations and flash-resident execution stalls; applications must budget
watchdog, radio/USB deadlines and power loss. No actual flash contents changed.

## Verification and unfinished delivery

The [software evidence record](evidence/nrf52840-peripherals/2026-11-01-software.json)
records native cases, public-header checks, actual service-body probes,
repository/source/relocated-package builds and image identities. The new examples
are [UARTE loopback](../../example/uarte-loopback/README.md),
[SPIM loopback](../../example/spim-loopback/README.md) and
[native services](../../example/native-services/README.md). Builds never upload
these images. Coverage instrumentation, peak-stack/runtime throughput, hosted CI
and every new physical procedure remain unexecuted.

Final checks pass: 68 CTest entries, 488 merged cases (69 new native cases and
one new workflow case), 254 isolated public headers on the host and three
target toolchains, actual service-body probes, nine repository/source/package
image builds and simulated-mount runner plans. No allocation/exception runtime
symbols appear in the three checked application families, and each linked
composition has one reset handler. IRQ regression tests verify completion-event
acknowledgment, late stop recovery and preservation of quiescence after a failed
event-clear write. Focused mcutrace validation reports no diagnostics; project-wide
validation has zero errors and retains warnings for unfinished evidence.

The larger native result exceeded the trace importer's fixed 65,534-byte JSON
string pool. Native builds now retain the combined summary and publish eight
bounded trace reports, preserving every original test ID, requirement tag and
producer hash. Workflow tests cover large reports, exactly-once inclusion and
invalidation after failed producers. Coverage graph export selects the shards
from its own build; actual coverage instrumentation was not run here.

| Plan | Current increment | Remaining software scope |
| --- | --- | --- |
| PLN-0044 | Concrete UARTE/SPIM/TWIM buffer/clock/resource integration | Other peripheral/crypto DMA integrations and their quiescence contracts |
| PLN-0045 | Finite UARTE0/1, SPIM0–3, TWIM0/1 | Bounded streaming; legacy SPI; SPIS/TWIS; remaining native modes, facade/device-lease integration and errata review |
| PLN-0046 | TEMP and QDEC | PWM0–3, SAADC, COMP/LPCOMP, remaining native sequencing/integration |
| PLN-0047 | Explicit-region NVMC word/page operations | QSPI/XIP, other NVMC modes, FICR/UICR/protection/MWU APIs and debugger tooling |
| PLN-0048 | No new audio runtime | I2S and PDM finite/streaming APIs and examples |
| PLN-0049 | No USB application runtime | USB endpoint/class operation, CDC/HID, suspend/resume and intentional bootloader handoff |
| PLN-0050 | Standalone RNG | ECB/CCM/AAR and every required CryptoCell engine/control/DMA path and known-answer compositions |
| PLN-0051 | No packet-engine runtime | All RADIO mode/PHY paths and explicit stack/resource boundary |
| PLN-0052 | No NFC runtime | NFCT/tag/field-wake paths and explicit pin/provisioning contract |
| PLN-0053 | No complete-delivery claim | Full instance/mode matrix, missing implementations, concurrent compositions and complete software qualification |

All ten items remain incomplete. Physical qualification additionally requires
the actual board, independent peers and suitable timing/analog/audio/RF/NFC/power
equipment as specified by each plan. Missing hardware does not account for the
remaining software rows above.
