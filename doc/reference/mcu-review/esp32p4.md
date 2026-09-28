# ESP32-P4 description review

This is the source review record for `PLN-0015`, completed 13 September 2026.
The declared devices are `ESP32-P4NRW16X` and `ESP32-P4NRW32X` in QFN104,
covering production revisions v3.0, v3.1 and v3.2. The selected common-contract
identity is NRW16X. This is declarative metadata only; it does not perform
startup, MMIO, cache or permission setup, DMA execution, multicore boot,
interrupt dispatch, board configuration, or RTOS integration.

## Pinned sources and applicability

| Source | Revision | Applied scope |
| --- | --- | --- |
| ESP32-P4 TRM | pre-release v0.7, 20 August 2026 | HP/LP CPU and buses, memory map, clock/reset, GPIO, interrupt matrix, four DMA engines and initial peripheral blocks |
| ESP32-P4 Series Datasheet | pre-release v0.7, 14 July 2026 | Part identities, PSRAM capacities, QFN104 pins, CPU maxima and initial direct IO functions |
| ESP32-P4 Series SoC Errata | v1.3, 10 July 2026 | Production v3.0/v3.1/v3.2 applicability boundary |
| ESP32-P4 Chip Revision v3.x User Guide | v3.x | Production revision selection; EOL v1.3 is excluded |

Every source reference in the public description retains the preliminary
revision. `GAP-ESP32P4-002` requires re-review when either primary document is
repinned.

## Reviewed increment

| Area | Source-reviewed result | Deliberately deferred breadth |
| --- | --- | --- |
| Identity/ownership | Two PSRAM-capacity variants, QFN104, two HP cores and one LP core with reset ownership | Executable startup and concurrent core operation |
| Memory/cache | HP SPM, ROM/L2MEM, LP ROM/SRAM, external flash/RAM windows and direct uncached aliases | Permission/cache controller programming |
| Clock/reset | HP and LP roots/domains, maxima, representative gates and reset controls | Complete clock/reset fields and runtime transitions |
| Pins | All 55 GPIO package positions, GPIO0–15 LP ownership, strapping flags, analog routes and representative direct/matrix routing | Complete matrix selector and direct-function inventory |
| Interrupts | Peripheral source IDs distinct from 32 CPU inputs; independently programmable CPU0/CPU1 matrix | Remaining source rows, security and core-local configuration |
| DMA | GDMA-AHB, GDMA-AXI, VDMA and 2D-DMA are distinct; topology, request encodings, descriptor alignment and memory reachability are explicit | Complete engine registers and specialized image-transfer rules |
| Timers/PWM | Native 52-bit system timer, 54-bit TIMG, 8-output/20-bit LEDC and two 6-output MCPWM blocks with dead time | Remaining event/fade/capture/fault registers |
| I2C/UART/SPI/ADC | Actual instance counts, bases and representative registers with clear/status side effects, routes, events and DMA requests | LP variants, full protocol modes and remaining registers |

LEDC and MCPWM are deliberately separate capabilities. Neither is modeled as
Renesas GPT or STM32 TIM hardware, and a request for an unavailable I3C
capability fails at compile time.

## Source gaps and conflict

- `GAP-ESP32P4-001` is `unavailable-in-source-exclusion-accepted`: TRM v0.7
  announces I3C but supplies no authoritative register model. D09 keeps I3C in
  the denominator and exposes no capability, instance or placeholder.
- `GAP-ESP32P4-002` is `preliminary-source`: every affected definition carries
  the pinned v0.7 provenance and must be revisited on a source update.
- `GAP-ESP32P4-003` is `source-conflict-recorded`: Figure 11.2-1 and the series
  datasheet specify a 400 MHz HP CPU, while nearby TRM clock prose says
  360 MHz. The production-v3.x identity retains 400 MHz and does not conceal
  the contradictory text.

## Verification boundary

`PLN-0013` records the series-datasheet unit, all 66 published TRM chapters and
the unavailable I3C unit in the public `complete_source_inventory`.

Host tests instantiate the common description contract and check part/package
identity, HP/LP ownership, memory aliases/cache policy, clock/reset limits,
QFN104 routing, interrupt-matrix semantics, all four DMA kinds, real timer/PWM
differences, register side effects and the explicit gaps. Negative compile
cases reject GPIO55 and I3C capability use. The RISC-V metadata probe includes
the public description, and the all-three translation unit now instantiates
the same contract for RA4M1, STM32H753 and ESP32-P4.

These checks establish source-traceable description consistency. They are not
evidence of boot, multicore scheduling, live MMIO, DMA completion, board
execution, or driver behavior; those belong to later plans.
