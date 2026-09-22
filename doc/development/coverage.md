# Description coverage inventory

Initial inventory for `PLN-0001`, reviewed 12 September 2026, incrementally
populated by `PLN-0004`, `PLN-0014` and `PLN-0015`, and reconciled across the
complete source denominator by `PLN-0013` on 13 September 2026. It tracks
source coverage independently from functional driver support. The authoritative
machine-readable rows are the three `complete_source_inventory` arrays exposed
by the public descriptions.

## State vocabulary

Description states are `unreviewed`, `described`, `verified`, `deferred`,
`unavailable-in-source`, and `not-applicable`. Driver states are `planned`,
`deferred`, and `not-applicable`. A description may be complete while its driver
is deferred; a planned driver never makes a description verified.

The denominator is a stable source unit: one reference-manual hardware chapter,
plus one device/package unit where that information comes from a separate
datasheet. Every chapter includes all of its registers, fields, routes and
constraints. Implementation slices must expand the chapter into child items and
may mark the parent `verified` only after every child is accounted for.

| Target | Denominator | Source-backed now | Source gaps | Initial state |
| --- | ---: | ---: | ---: | --- |
| RA4M1 | 48 | 48 | 0 | 48 `verified` |
| STM32H753 | 61 | 61 | 0 | 61 `verified` |
| ESP32-P4 | 68 | 67 | 1 | 67 `verified`, 1 `unavailable-in-source` |
| **Total** | **177** | **176** | **1** | 176 `verified`, 1 `unavailable-in-source` |

RA4M1 uses R01UH0887EJ0110 chapters 1–48. STM32H753 uses one DS12117
identity/package unit plus RM0433 chapters 2–61; documentation conventions,
security notice and revision history are not hardware units. ESP32-P4 uses one
series-datasheet identity/package unit, the 66 published TRM v0.7 chapters, and
the announced but unavailable I3C chapter.

## Common priority inventory

| Coverage area | RA4M1 source | STM32H753 source | ESP32-P4 source | Description | Driver support |
| --- | --- | --- | --- | --- | --- |
| Identity and package | R01UH ch.1; R01DS tables 1.13–1.14 | DS12117 tables 1–2 and §§5, 8–9 | Series Datasheet §§1–2, 6 and Appendix A | All three complete inventories `verified` | `not-applicable` |
| CPU and memory | R01UH chs.2, 4, 15, 43–44 | RM0433 chs.2–5; PM0253 core details | TRM chs.1–5 and 9 | All applicable units `verified` | `not-applicable` |
| Clocks, reset and power | R01UH chs.3, 5–12, 47 | RM0433 chs.6–10 | TRM chs.11–12, 15, 25–26 | All applicable units `verified`; ESP clock conflict remains explicit | `not-applicable` |
| Pins and routing | R01UH ch.19 and package tables | RM0433 chs.11–13; DS12117 tables 8–9 | TRM ch.10; Series Datasheet §2 and Appendix A | All applicable units `verified` | `not-applicable` |
| Interrupts and events | R01UH chs.13, 18, 20 | RM0433 chs.19–20 | TRM chs.13–14 and 24 | All applicable units `verified` | `not-applicable` |
| DMA and reachability | R01UH chs.14, 16–17 | RM0433 chs.14–18 | TRM chs.6–8; errata by silicon revision | All applicable units `verified` | `planned` |
| GPIO | R01UH ch.19 | RM0433 ch.11 | TRM ch.10 | All applicable units `verified` | `planned` |
| Timers and basic PWM | R01UH chs.21–26 | RM0433 chs.37–46 | TRM chs.16–19, 51, 58–60 | All applicable units `verified`; native timer kinds remain distinct | `planned` |
| I2C | R01UH ch.29; SCI simple-IIC in ch.28 | RM0433 ch.47 | TRM ch.47 | All applicable units `verified` | `planned` |
| UART | R01UH SCI ch.28 | RM0433 chs.48–49 | TRM ch.45 | All applicable units `verified` | `planned` |
| SPI | R01UH ch.31; SCI simple-SPI in ch.28 | RM0433 ch.50 | TRM ch.46 | All applicable units `verified` | `planned` |
| ADC | R01UH chs.35, 37 | RM0433 ch.25 | TRM chs.64–65 | All applicable units `verified` | `planned` |

Chapter references use the exact local manual indexes. Timer/PWM references
include event/output-control blocks that constrain real output identities; they
are not a claim that the timer layouts are interchangeable.

## Historical RA4M1 PLN-0004 increment

The table below preserves the initial increment boundary. Its deferred column is
historical; every row was reconciled by `PLN-0013` in the complete source-unit
inventory.

| Chapter/unit | Reviewed child slice | State | Deferred to PLN-0013 |
| --- | --- | --- | --- |
| 1 | Exact `R7FA4M1AB3CFM#AA0` / `PLQP0064KB-C` identity and 52 GPIO-capable bond-outs | `verified` | Non-selected family/package variants |
| 4 | 256-KiB code flash, 32-KiB SRAM and 8-KiB data-flash regions | `verified` | Option-setting and detailed flash-controller regions |
| 5 | Shared MCU reset source and status semantics | `verified` | Complete reset-status field inventory |
| 8 | Six roots; ICLK/PCLKA/B/C/D/FCLK domains, limits and divider relation; principal control registers | `verified` | Remaining clock-control register fields and usage notes |
| 10 | Module-stop controls used by the initial peripheral/resource set | `verified` | Non-initial low-power controls |
| 12 | PRCR clock/low-power groups and key encoding | `verified` | LVD protection group details |
| 13 | 32 selectable CPU lines, separate event encodings, and initial-peripheral events | `verified` | Non-initial event-table rows and NMI/wakeup detail |
| 16 | All 12 numbered DMAC register subsections, four-channel stride and initial requests | `verified` | Extended transfer-mode behavioral implementation (no driver in description phase) |
| 17 | DTC SRAM descriptor layout, constraints, activation distinction and control-register facts | `verified` | Complete activation/ELC catalogue |
| 19 | GPIO aliases, PFS/PWPR semantics, all selected-package pins and initial-peripheral routes | `verified` | Alternate functions outside the initial slice |
| 22 | All 24 numbered GPT register subsections, eight instances, 32/16-bit widths and A/B output identities | `verified` | Non-driver operational sequences |
| 28 | SCI0/1/2/9 asynchronous-UART register and event slice | `verified` | FIFO, smart-card and full simple-IIC/simple-SPI detail |
| 29 | IIC0/1 register set, signals, events and DMA requests | `verified` | Broader operational/usage-note constraints |
| 31 | SPI0/1 register set including 32/16-bit SPDR aliases, signals, events and DMA requests | `verified` | Broader operational/usage-note constraints |
| 35 | ADC14 initial scan/data registers, analog pin routes, clocks, events and DMA request | `verified` | Compare-window, sensor and calibration detail |

`GAP-RA4M1-001` is `manual-revision-scope-accepted`: no separate device errata was
available from the official part page at the baseline review. Therefore these
definitions are scoped to the two Rev.1.10 documents and are not a claim of
silicon-wide verification.

## Historical STM32H753 PLN-0014 increment

The table below preserves the initial increment boundary. All deferred breadth
was reconciled by `PLN-0013`; the corrected interconnect decision is recorded
in the final source-unit inventory and common gate report.

| Chapter/unit | Reviewed child slice | State | Deferred to PLN-0013 |
| --- | --- | --- | --- |
| DS unit | Exact VI/LQFP100 identity, all eight packages, revision IDs, selected bond-outs and initial AF routes | `verified` | Complete pins/AFs for all package variants |
| 2 | Flash, ITCM, DTCM, AXI SRAM, SRAM1-4 and backup SRAM regions with cache/DMA properties | `verified` | Detailed memory attributes and controller integration |
| 8 | Clock roots/domains, initial mux/divider edges, timer kernels, corrected ADC limit, initial gates/resets | `verified` | Remaining RCC/PWR fields and usage notes |
| 11 | GPIO register slice and corrected AFRH offset | `verified` | Full pin/AF inventory |
| 13 | Resource/interconnect constraints used by initial routes | `verified` | Corrected full bus-matrix reconstruction |
| 14 | MDMA reachability including ITCM/DTCM | `verified` | Complete MDMA register catalogue |
| 15 | DMA1/2 constraints, initial stream registers and non-TCM reachability | `verified` | Remaining stream registers and behavioral notes |
| 16 | BDMA D3-only reachability and constraints | `verified` | Complete BDMA registers |
| 17 | Initial DMAMUX1 request IDs, generator registers and errata semantics | `verified` | Remaining request rows and generators |
| 19 | Fixed NVIC routes for initial blocks, including shared timer lines | `verified` | Remaining vector entries |
| 25 | ADC1-3 instances, representative registers, request IDs and errata scope | `verified` | Injected/oversampling/watchdog breadth |
| 38 | TIM1/TIM8 advanced capabilities and representative register model | `verified` | Full usage-note and trigger inventory |
| 39 | TIM2-5 widths, outputs and capability differences | `verified` | Full usage-note and trigger inventory |
| 41 | TIM12-17 widths and output identities | `verified` | Full usage-note and trigger inventory |
| 47 | I2C1-4 instances and initial register/event/DMA slice | `verified` | Full protocol modes and constraints |
| 48 | USART/UART instances and initial register/event/DMA slice | `verified` | Full protocol and instance distinctions |
| 49 | LPUART instance and shared initial register shape | `verified` | LPUART-specific behavioral breadth |
| 50 | SPI1-6 instances and initial register/event/DMA slice | `verified` | Full I2S/audio and instance distinctions |

`GAP-STM32H753-001` is `correction-overlay-applied` for these slices. DMA
reachability does not consume the defective RM0433 Table 3; the full corrected
matrix is reconciled through the pinned correction overlay. See
`stm32h753-description-review.md`.

## Historical ESP32-P4 PLN-0015 increment

The table below preserves the initial increment boundary. All published units
were reconciled by `PLN-0013`; I3C remains the sole unavailable source unit.

| Chapter/unit | Reviewed child slice | State | Deferred to PLN-0013 |
| --- | --- | --- | --- |
| Datasheet unit | Exact NRW16X/NRW32X PSRAM capacities, QFN104 package, 55 GPIO bond-outs, v3.x scope | `verified` | Non-GPIO electrical detail |
| 1 | HP/LP bus domains and 32-/128-bit widths | `verified` | Detailed arbitration and monitor registers |
| 2–3 | Two HP and one LP 32-bit RISC-V cores, ownership, maxima and reset vectors | `verified` | Core-local CSR/debug breadth and executable startup |
| 6 | GDMA-AHB/AXI channel topology, descriptor rules, selectors, requests and shared reachability | `verified` | Complete channel/register catalog and behavioral driver sequences |
| 7 | Four-channel VDMA topology and extra MIPI-memory reachability | `verified` | Complete VDMA registers and specialized transfer rules |
| 8 | Seven-channel 2D-DMA topology and general memory reachability | `verified` | Reordering/color-conversion registers and constraints |
| 9 | Cached/direct memory map, selected-part PSRAM capacity, peripheral bases and DMA address space | `verified` | Complete permission/cache-controller registers |
| 10 | All QFN104 GPIOs, HP/LP ownership, strapping flags, representative IO-MUX routes and matrix legality | `verified` | Complete selector encodings and remaining direct functions |
| 11 | HP/LP clock graph, frequency limits, selected gates and resets | `verified` | Complete clock/reset register fields; source conflict stays explicit |
| 13 | 128-source/two-core/32-input interrupt matrix and initial source IDs/routes | `verified` | Remaining source rows, security controls and core-local interrupts |
| 16–17 | System timer and two timer groups with native 52-/54-bit widths | `verified` | Remaining timer/watchdog registers and usage rules |
| 45–47 | Five UARTs, two GP-SPIs and two HP I2Cs with initial register/event/DMA slices | `verified` | LP instances, remaining registers and protocol modes |
| 58 | LEDC: four 20-bit timers and eight PWM generators | `verified` | Fade/event task and remaining channel registers |
| 59 | Two MCPWM blocks with three timers/operators, six outputs and dead time | `verified` | Capture/fault/operator breadth |
| 65 | ADC1/ADC2 pins, controller registers, event and GDMA request | `verified` | LP ADC and remaining digital-controller detail |
| I3C announced chapter | No authoritative register content in TRM v0.7 | `unavailable-in-source` | `GAP-ESP32P4-001`; no capability is exposed |

The review also records `GAP-ESP32P4-003`: TRM v0.7's clock diagram and the
series datasheet say 400 MHz, while nearby TRM prose says 360 MHz. Production
v3.x device metadata retains 400 MHz and keeps the discrepancy visible. See
`esp32p4-description-review.md`.

## PLN-0013 reconciliation result

| Target | Accounted source units | Description | Driver support |
| --- | --- | --- | --- |
| RA4M1 | R01UH chapters 1–48 | 48 `verified` | Initial set `planned`; other blocks `deferred` or `not-applicable` |
| STM32H753 | DS12117 identity/package unit and RM0433 chapters 2–61, with ES0392/correction overlays | 61 `verified` | Initial set `planned`; other blocks `deferred` or `not-applicable` |
| ESP32-P4 | Series-datasheet unit and all 66 published TRM v0.7 chapters | 67 `verified` | Initial set `planned`; other blocks `deferred` or `not-applicable` |
| ESP32-P4 I3C | Announced but unpublished source unit | 1 `unavailable-in-source`; exact exclusion accepted at the common gate | `deferred` |

Each C++ row gives the source-unit identifier/title, first authoritative PDF
page, description and driver states, category-accounting flags, decision text,
and revisioned provenance. Compile-time validation rejects duplicate units,
unfinished states, incomplete category accounting, and unavailable material
misreported as verified.

## Accounting rules

1. A child item records target, source document/revision, section/table or page,
   package/silicon applicability, description state and driver state.
2. `unavailable-in-source` is never included in described or verified totals.
   It stays in the denominator and links to the source-gap register.
3. Errata applicability is a child item of every affected block. An unresolved
   conflict prevents the block from becoming `verified`.
4. `not-applicable` requires a source-backed reason, such as package absence; it
   is not a synonym for deferred work.
5. The initial cross-MCU slices follow the table order above so the shared model
   is tested against real differences before any driver is implemented.
