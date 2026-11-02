# Nordic register baseline and system services

This is an implementation checkpoint for PLN-0042/PLN-0043. Both plans remain
in progress. A reproducible MDK register import and tested system services are
available; the complete product-specification and errata gate is still open.

## Source identity and coverage

[source-manifest.json](source-manifest.json) records SHA-256 identities for the
inputs. The imported baseline is Nordic
[nrfx v3.2.0](https://github.com/NordicSemiconductor/nrfx/tree/7ef620bedd3fd41828e0f81523a1d08a986b8a0e/mdk)
(commit `7ef620bedd3fd41828e0f81523a1d08a986b8a0e`): `nrf52840.svd`,
`nrf52840.h`, `nrf52840_bitfields.h`, `nrf52_erratas.h`,
`system_nrf52840.c` and `system_nrf52.c`. Other pinned cross-checks are
[Arm CMSIS 5.9.0 core_cm4.h](https://github.com/ARM-software/CMSIS_5/blob/5.9.0/CMSIS/Core/Include/core_cm4.h)
and [Zephyr v4.2.0 nrf52840.dtsi](https://github.com/zephyrproject-rtos/zephyr/blob/v4.2.0/dts/arm/nordic/nrf52840.dtsi).

The public `mcu/nrf52840/registers/` headers describe 43 source register families;
`catalogue.hpp` accounts for all 74 SVD instances. Expanding the source arrays and
clusters yields 2,142 register entries and 5,949 field entries across those
instances. Derived instances share family definitions. These numbers describe
that source, including its aliases and superset layouts; they are not a measured
percentage of the current chip's implemented registers. Instance restrictions
such as TIMER0–2's four channels and RAM blocks' two/six sections are enforced
by the native services and recorded in `mcu/nrf52840/system.hpp`.

Command, set/clear alias, W1C, zero-clear event and side-effecting read operations
have distinct types. FIFO reads marked `modifyExternal` require the explicit
`register_ref::read_and_clear()` operation. Writable masks, source reset masks,
fields, enum encodings and write-once flags are retained. Write-once flags are
metadata; they do not provide a software provisioning state machine.

Reproduce the import offline after obtaining the exact files listed in the
manifest into a local directory:

```sh
python3 tool/nrf52840-descriptions.py --source-dir /path/to/pinned-inputs --check
```

The command checks hashes before parsing and compares every output without
changing the checkout. Omitting `--check` regenerates the source facts. It does
not fetch sources or run a register-by-register product-specification audit.
The [Nordic BSD notice](NORDIC-LICENSE.txt) is retained in the checkout and package.

The [Product Specification v1.11](https://docs.nordicsemi.com/r/bundle/ps_nrf52840/page/keyfeatures_html5.html)
and its [instantiation table](https://docs.nordicsemi.com/r/bundle/ps_nrf52840/page/memory.html)
are now pinned by [content identity](product-specification.json). The vendor-authored
PDF was obtained from its distributor mirror; it is kept in the ignored local
manual cache rather than distributed with the library.

`mcu/nrf52840/product.hpp` now describes all 53 register families from the current
product tables: 1,563 detailed registers and 4,476 fields, including CTRL-AP as a
debug access port rather than CPU MMIO. The CryptoCell subset has 11 subblocks,
199 registers and 345 fields, including 192 registers absent from the MDK. The
expanded parser recovers the two high-order HOST_BOOT fields previously missed
by the 343-field supplement. Use these types for current product work.
The seven legacy CC_HOST_RGF registers
use a different base/offset pair but resolve to the same absolute addresses.
Mixed read-only/write-once fields and side-effecting data ports are explicit.
`SRAM_DATA` permits writes and requires `read_with_side_effect()` for a read;
ordinary reads and read-modify-write are unavailable. Write-once metadata does
not itself enforce a provisioning lifecycle.

[The register audit](product-register-audit.json) accounts for all 1,567 entries
in the specification's register overview tables: 1,366 match MDK absolute
addresses, 192 are supplied by the product supplement, four UICR addresses are
reserved and five belong to CTRL-AP rather than CPU MMIO. This is an address
reconciliation, not proof of every field, mode or revision-specific behavior.
[The field audit](product-field-audit.json) reconciles 4,122 MDK field matches,
346 supplementary fields and eight reviewed access differences, with no
unresolved field-shape/reset differences. The differences cover seven older
CryptoCell access descriptions and CLOCK.LFRCMODE.STATUS: its prose defines a
read-only status bit despite the table's broad RW annotation. Both generated
views exclude that status bit from writes. W1C classifications account for MDK
`modifiedWriteValues` attributes rather than treating those as RW conflicts.
Literal field enums have checked encoders that reject unlisted values; valid
cross-field combinations and instance-specific modes still need driver review.
Family types retain the source template layouts. A field's presence in a family
does not establish availability on every shared-layout instance; package and
instance restrictions remain separate constraints.
Reproduce the tables and types from the pinned PDF/SVD:

```sh
python3 tool/nrf52840-product.py --pdf doc/_vendor/manual/nrf52840-ps-v1.11.pdf \
  --source-dir /path/to/pinned-inputs --check
```

[The instance audit](product-instance-audit.json) covers all 85 instantiation
table rows. The FPU row at `0x40026000` is IRQ 38 only; it does not describe an
FPU register block there. Independent POWER/CLOCK and MWU/ACL controls are not
collapsed into exclusive personalities merely because addresses coincide.

`product/pins.hpp` and [package-pins.json](package-pins.json) describe aQFN73,
QFN48 and WLCSP94 bonding, fixed functions and standard-drive/10 kHz-only pads.
Package pin bindings reject unbonded pads; unknown FICR package codes stay unknown.
These are chip/package facts, not a claim about this clone's connector exposure.
The GPIO backend rejects high drive on restricted die pads before MMIO.
`memory.hpp` separates the CPU code-RAM alias from Data RAM accepted by EasyDMA,
checks spans/alignment without overflow and records the two/six-section RAM
geometry. This does not implement peripheral-specific DMA sequencing.

`core_registers.hpp` adds 12 selected Cortex-M4F components with 382 typed register
views and 729 field entries. [core-facts.json](core-facts.json) records the CMSIS,
M4, Armv7-M and ETMv3.5 sources and semantic overlays. These include keyed writes,
byte priority registers, read-only NVIC active bits, read-clearing debug status,
32 ITM ports with separate access widths, FPB code/literal comparator differences
and the ETM-M4 inventory. The DCRSR selector includes floating-point registers;
TPI DEVTYPE corrects a missing shift in CMSIS 5.9. DWT cycle/data matching is
restricted to comparators 0/1, and LNK1ENA/FFCR.TrigIn are read-only.
Unknown or integration-dependent reset bits remain unknown. Reproduce with
the exact Arm PDFs and CMSIS header listed in the manifests (Poppler required):

```sh
python3 tool/nrf52840-core.py --source-dir /path/to/pinned-inputs --check
```

The [Arm Apache notice](ARM-LICENSE.txt) accompanies the derived core facts in
the checkout and installed package. Declarative types do not take ownership of
debug hardware or enforce cross-register programming sequences.

Complete instance/mode/errata sequencing review and actual board/stepping
qualification remain open. Accordingly,
`complete_product_description` is explicitly false. The old three-MCU gate
continues to cover its original targets only.

## Explicit native runtime

The subsequent [coordination/UART checkpoint](../../acceptance/nrf52840-dma-uart.md)
adds `backend/nrf52840/resources.hpp`, `dma_buffers.hpp` and the explicit native
legacy `uart.hpp` backend. TWI/GPIO/system clients share resource and pin
ownership; future DMA drivers use the bounded handoff/quiescence contract.
`uart_config.hpp` preserves UART/UARTE timing differences. The later
[peripheral checkpoint](../../acceptance/nrf52840-peripherals.md) adds finite
UARTE0/1, SPIM0–3 and TWIM0/1 DMA runtimes, TEMP/QDEC/RNG and bounded NVMC
word/page operations. Board facade integration, streaming, legacy SPI and
SPIS/TWIS runtime work remain pending. See these checkpoints for
ownership, clock, cancellation and physical-evidence boundaries.

`backend/nrf52840/system_context.hpp` borrows one MMIO object, execution policy
and existing GPIO backend. Use one context per physical MCU and share it among
all system-service clients. Keep the context, policy, GPIO backend, callback data
and driver objects alive until their users have been shut down. Configuration
and teardown are foreground operations; interrupt callbacks may acknowledge
and service their already-configured sources. This is a single-core bare-metal
composition, not an RTOS concurrency adapter.

| Header | Delivered software API |
| --- | --- |
| `clock_power.hpp` | Reference-counted HFXO/LFCLK requests, oscillator/external LF selection, HFXO debounce, calibration, calibration timer, reset/retention, power-fail/regulator controls, guarded RAM power, System ON idle and System OFF entry |
| `events.hpp` | Eight GPIOTE channels, GPIO port sensing/latch, 20 programmable and 12 fixed PPI channels with fork, six 32-channel groups, EGU0–5 and software IRQ delivery |
| `timer.hpp` | TIMER0–4, width/mode/prescaler, capture/compare/shortcuts and events/tasks |
| `rtc_watchdog.hpp` | RTC0–2, configurable compile-time prescaler, overflow-extended time, bounded-distance software alarm; watchdog channels/feed/start/irreversible shutdown rules |
| `core.hpp` | NVIC subscriptions with bounded storage, shared-vector priority checks, vector relocation, SysTick, FPU, MPU, DWT/ITM controls and fault-status queries |

The board facade still advertises GPIO and finite I2C. The additional APIs are
explicit native services; they are not automatically initialized by `hw.begin()`.
The [system example](../../../example/system/README.md) composes the services
without starting USB, a radio stack or a System OFF reset. Its current version
requires RTC interrupt/alarm progress for the visible success pulses and starts
a three-second watchdog fed only after a completed RTC cycle. A separate
`system-watchdog` image deliberately withholds feeds to test reset behavior.

Core reservations protect the default DWT execution clock and hard-float FPU
ownership. Supply explicit `core_reservations` when an external BSP/RTOS owns
SysTick, MPU or vectors. Stop an owned SysTick and disable an owned MPU before
releasing their core controller. Do not disable trace/DWT underneath a clock.

PPI endpoints carry context, resource and generation identity. Routes retain
endpoints until successful disable/readback; stale, foreign and invalid-address
endpoints are rejected. Release a route before its timer/pin. Groups retain their
member channels. To build a self-disabling group, initialize an empty group,
initialize the route to its disable task, then call `set_members()` with all PPI
channels disabled. Use `clear_members()` before tearing down that cycle.

RTC overflow must be serviced before an entire 24-bit period elapses. At the
default 1,024 Hz, this is approximately 4.55 hours; the enabled overflow IRQ and
`poll()` provide the extension during normal idle. `now()` supports existing
32-bit execution deadlines; `ticks()` exposes the 64-bit count with typed errors.
A time-read/overflow-acknowledgment fault poisons the clock until explicit shutdown
and reinitialization. Nonintegral tick frequencies use `ticks()` rather than the
integer-frequency execution adapter. Alarm CC0 is reserved while armed. System
OFF resets this time origin; it is not a continuation of System ON sleep.

The watchdog TIMEOUT event can be observed or routed; it cannot cancel the
impending reset. The watchdog cannot be stopped once started; its resource and clock stay owned
until reset. Ambiguous START errors are treated as potentially committed.
RAM shutdown requires an explicit mask of sections the memory owner has proved
unused by code, vectors, stack, data or DMA. Regulators require an explicit board
circuit allowance. Sleep predicates are checked with the saved interrupt mask
held; applications must also exclude active clients outside this context.

Fixed PPI channels 20–31 validate their immutable silicon bindings from PS p.493.
They never write nonexistent EEP/TEP registers. `external_endpoints.hpp` adapts an
existing radio/AAR/CCM owner using an explicit quiescence callback; it does not
implement those drivers. AAR and CCM share one exclusive personality identity.
A fixed route retains both primary endpoints and its optional fork, and group
membership retains the fixed channel through disable/readback as for channels 0–19.

POWER events support separate interrupt enable/acknowledgement and leased PPI
endpoints. CLOCK events have leased endpoints; CTTO has a dedicated interrupt
and consume API because its expiry stops the calibration timer. Synchronous
clock handshakes retain ownership of their other acknowledgements. POWER and
CLOCK share IRQ 0 but clear only their own enabled interrupt sources. A failed
calibration-timer start retains the clock until explicit stop/recovery.

`lf_configuration` distinguishes an oscillator, low-swing external clock and
full-swing external clock. External input requires `lf_source::crystal` and the
specified XL1/XL2 wiring. Sharing/inherited clocks checks latched SRC and the
current LFCLKSRC bypass/external bits; only SRC is present in LFCLKSRCCOPY.
`hf_debounce()` requires a stopped HFXO. RTC and watchdog initialization also
accept this configuration. These APIs do not assert that the board has the selected oscillator or wiring.

Debugger-owned FPB/DWT inspection and ITM/TPIU/ETM-M4 configuration are described
in [the debug procedure](../../../src/board/others/promicro_nrf52840/debug/README.md).
[Arm content identities](arm-references.json) pin the Cortex-M4 and ETM-M4 manuals.
The scripts require an explicit firmware resource/pin handoff and a halted target;
they preserve the execution policy's DWT counter. Software mocks and OpenOCD
configuration parsing do not establish physical capture or pin accessibility.

## Errata incorporated into this slice

- [20](https://docs.nordicsemi.com/r/bundle/errata_nrf52840_rev3/page/err/nrf52840/rev3/latest/anomaly_840_20.html): synchronize LFCLK before RTC access.
- 78: shut down timers rather than issuing STOP alone.
- [87](https://docs.nordicsemi.com/r/bundle/errata_nrf52840_rev3/page/err/nrf52840/rev3/latest/anomaly_840_87.html): clear FPU exception flags and pending FPU IRQ before native idle.
- [155](https://docs.nordicsemi.com/r/bundle/errata_nrf52840_rev3/page/err/nrf52840/rev3/latest/anomaly_840_155.html): retain/release the channel clock for fast GPIOTE input edges. `fast_edges=false` requires edges at least 1.3 microseconds apart.
- 173: read back GPIO LATCH after clearing.
- [179](https://docs.nordicsemi.com/r/bundle/errata_nrf52840_rev3/page/err/nrf52840/rev3/latest/anomaly_840_179.html): a referenced RTC PPI endpoint excludes CPU time/configuration access until routes are released. Use a separate RTC for the execution clock.
- 209: reject calibration of LFRC ultra-low-power mode.
- [210](https://docs.nordicsemi.com/r/bundle/errata_nrf52840_rev3/page/err/nrf52840/rev3/latest/anomaly_840_210.html): configure input before sensing.
- 213: a running watchdog blocks System OFF through the context.
- [246](https://docs.nordicsemi.com/r/bundle/errata_nrf52840_rev3/page/err/nrf52840/rev3/latest/anomaly_840_246.html): apply the documented RAM-arbitration workaround during power-controller initialization; its active-clock current cost remains a measurement item.
- [277](https://docs.nordicsemi.com/r/bundle/errata_nrf52840_rev3/page/err/nrf52840/rev3/latest/anomaly_840_277.html): use LDETECT for owned port sensing and restore the previous mode on shutdown.

Identity queries now return FICR INFO plus the ROM part/revision fields used
by the pinned MDK errata predicates. The system example stores all seven values
in RAM, and the debugger inspection command reads the corresponding registers.
There is still no UF2/USB runtime readback for those values.

These workarounds use the published nRF52840 predicates/conditions. They do not
establish the user's chip stepping or replace the full revision audit. Supplied
startup still depends on the factory bootloader's reset-time silicon setup;
external startup must supply its own reviewed reset/errata contract.

## Outstanding scope

PLN-0042 still requires complete instance/mode/errata sequencing review,
revision-applicable errata identity and actual board/silicon identity readback.
Product/core table imports and package bonding are delivered; this does not
establish all runtime capability combinations, board wiring or silicon stepping.
PLN-0043 still requires exhaustive source/errata review and any resulting
sequencing corrections, alongside the specified physical tests and actual
debugger/trace capture. The fixed PPI and debugger script increments are
implemented and software-tested.
The delivered APIs must not be described as the completion of either plan.
Current software and physical observations are recorded separately in
[the acceptance checkpoint](../../acceptance/nrf52840-system.md).

## Native peripheral delivery

The [native delivery guide](../../acceptance/nrf52840-native-delivery.md) and
[per-instance/mode matrix](implementation-matrix.json) describe the current
working APIs, fixed resource/buffer contracts, representative software tests,
source/installed examples and pending physical acceptance for PLN-0044–0053.
The older source audits above remain metadata evidence, separate from those
implementation and verification states.
