# MCU source baseline

Baseline reviewed 12 September 2026 for `PLN-0001`. This document fixes the
description scope; it does not claim that any description or driver exists.
Local-file identities and SHA-256 values are in
[source-manifest.json](source-manifest.json). Searchable copies of the local
manuals are indexed in [the generated manual index](../_generated/manual/README.md).

## Selection rules

- MCU descriptions are family/device metadata, independent of a physical
  board. A later driver plan may choose a narrower board binding without
  changing this baseline.
- A definition is usable only when its source revision and section/table can be
  recorded. Errata and published correction notices override the base manual.
- Preliminary material is allowed as an explicitly versioned input. Missing
  register content remains `unavailable-in-source` and cannot be represented by
  a placeholder or counted as verified.
- The initial driver set is GPIO, timer/basic PWM, I2C, UART, SPI, ADC and DMA.
  Other blocks remain description work but driver support is deferred unless a
  later requirement adds it.

## Declared targets

| Target | Device and package scope | Silicon/revision assumption | Primary baseline |
| --- | --- | --- | --- |
| RA4M1 | `R7FA4M1AB3CFM#AA0`; `PLQP0064KB-C`, 64-pin LQFP | Revisions covered by Rev.1.10; inspect `MCUVER` and applicable updates per slice | R01UH0887EJ0110 Rev.1.10 plus R01DS0355EJ0110 Rev.1.10 |
| STM32H753 | `STM32H753VI/ZI/II/BI/XI/AI`; all eight DS12117 packages | Y/W (`REV_ID 0x1003`), X (`0x2001`) and V (`0x2003`), with per-revision errata | RM0433 Rev.8, DS12117 Rev.10, ES0392 Rev.15; PM0253 Rev.6 is core-only supplementary material |
| ESP32-P4 | `ESP32-P4NRW16X` and `ESP32-P4NRW32X`; QFN104 | Current-production v3.x; preserve v3.0/v3.1/v3.2 differences; exclude EOL v1.3 | TRM pre-release v0.7 dated 2026-08-20, Series Datasheet pre-release v0.7 dated 2026-07-14, Series SoC Errata v1.3 dated 2026-07-10, and the v3.x revision guide |

The STM32 package breadth is intentional: DS12117 makes package-dependent pins
and capabilities part of the device description, while the physical board is a
later choice. The two ESP32-P4 part numbers differ by in-package PSRAM capacity,
so that difference belongs in device metadata rather than a board alias.

## Official revision check

The vendor pages were checked on 12 September 2026.

- Renesas still publishes the RA4M1 hardware manual and datasheet as Rev.1.10
  and identifies `R7FA4M1AB3CFM#AA0` as an active 64-pin LQFP part.
- ST lists RM0433, DS12117 and ES0392 for STM32H743/753; the local revisions are
  Rev.8, Rev.10 and Rev.15 respectively. ES0392 is authoritative for the
  silicon-revision matrix and documentation errata.
- Espressif publishes v3.x as the new-design baseline. The local TRM v0.7 is
  dated 20 August 2026. The current series datasheet is pre-release v0.7 and the
  current series errata is v1.3; both remain remote revision-check inputs rather
  than locally pinned files.

Official entry points are recorded in `baseline_review` in the source manifest.
Do not silently replace a local PDF when a vendor updates an unversioned URL:
add the new file, checksum it, regenerate the Markdown copy, and review the
delta first.

## Published corrections and source gaps

| ID | Target | State | Affected scope | Resolution/constraint |
| --- | --- | --- | --- | --- |
| `GAP-RA4M1-001` | RA4M1 | `manual-revision-scope-accepted` | Device-revision-specific technical updates | The official part page exposes no separate errata sheet. The gate is accepted only for the pinned Rev.1.10 document scope, not as silicon-wide verification; query technical updates and check `MCUVER` during physical driver work. |
| `GAP-STM32H753-001` | STM32H753 | `correction-overlay-applied` | RM0433 Table 60 ADC kernel-clock limits, GPIO AFRH offset and Table 3 bus interconnect | Corrected ADC maxima and GPIO AFRH offset are applied. The complete interconnect accounting uses the correction overlay and per-engine RM0433 narrative; removed Table 3 routes are not exposed. |
| `GAP-ESP32P4-001` | ESP32-P4 | `unavailable-in-source-exclusion-accepted` | I3C register model | TRM v0.7 announces I3C but supplies no authoritative content. D09 keeps I3C in the denominator and excludes every capability, instance and placeholder until a new source revision is pinned and reviewed. |
| `GAP-ESP32P4-002` | ESP32-P4 | `preliminary-source` | All TRM- and datasheet-derived definitions | Both primary documents are pre-release. Record document version and silicon revision on every definition, apply v3.x errata, and re-review affected slices when a new revision is pinned. |
| `GAP-ESP32P4-003` | ESP32-P4 | `source-conflict-recorded` | HP CPU/CPLL maximum clock | TRM v0.7 Figure 11.2-1 and the series datasheet specify 400 MHz, while TRM §§11.2.3–11.2.4 prose says 360 MHz. PLN-0015 retains the production-v3.x 400 MHz device limit and keeps this contradiction explicit pending a corrected TRM. |

These states are source conditions, not failed implementation or test evidence.
Only `GAP-ESP32P4-001` is missing register content. D09 supplies the required
scope decision, so its explicit exclusion satisfies the current description
gate without turning missing source into verified metadata.
