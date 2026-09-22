# RA4M1 foundational description review

Reviewed 13 September 2026 for `PLN-0004` against the locally pinned
R01UH0887EJ0110 Rev.1.10 hardware manual and the R01DS0355EJ0110 Rev.1.10
device/package baseline recorded in `source-manifest.json`.

## Declared scope

- Device: `R7FA4M1AB3CFM#AA0`.
- Package: `PLQP0064KB-C`, 64-pin LQFP.
- Description only: no startup, vector ownership, ISR dispatch, register
  sequencing, transfer engine, or functional driver is included.
- Every exported table owns a `source_ref`. Exact locations are attached to the
  identity, memory, clocks, resets/protection, routes, events, DMA constraints,
  register catalogues, and capability data.

## Reviewed facts

| Area | Source locations | Encoded result |
| --- | --- | --- |
| Identity/package | R01UH §§1.3–1.7, Figure 1.5, PDF pp.57–74; R01DS Tables 1.13–1.14 | Exact part/package; 256-KiB code flash, 32-KiB SRAM, 8-KiB data flash; 52 GPIO-capable package pins |
| Address space | R01UH §4.1, PDF p.92 | Code flash `0x00000000`, SRAM `0x20000000`, data flash `0x40100000` |
| Clocks | R01UH Tables 8.1–8.2 and §§8.2.1–8.2.4, PDF pp.129–136 | Six selectable roots and separate ICLK/PCLKA/B/C/D/FCLK limits; divider and source control metadata |
| Module stop | R01UH §§10.2.2–10.2.5, PDF pp.175–178 | Shared DMAC/DTC, grouped GPT, SCI/IIC/SPI and ADC14 gates; gates are not misrepresented as per-instance resets |
| Protection/reset | R01UH §§5.2–5.3 and 12.1–12.2.1, PDF pp.93–104 and 223–225 | Shared Cortex-M4 software reset; PRCR key/group data; PFS and GPT protection references |
| ICU | R01UH §§13.2.6–13.4 and Tables 13.3–13.4, PDF pp.233–247 | Event encodings remain distinct from the 32 selectable CPU lines; DELSR remains distinct from IELSR/DTC selection |
| DMAC/DTC | R01UH chs.16–17, PDF pp.292–345 | Four DMAC channels, all 12 numbered DMAC register subsections, byte/halfword/word widths, 65535 bounded static count, and a distinct 16-byte SRAM DTC descriptor |
| GPIO/PFS | R01UH §§19.2–19.6 and Tables 19.5–19.15, PDF pp.357–379 | 8/16/32-bit PFS aliases, read-only PIDR field, PWPR reset/unlock facts, exact selected-package availability, initial peripheral routes |
| GPT | R01UH ch.22, PDF pp.392–512 | Eight instances at `0x100` stride; GPT0/1 are 32-bit, GPT2–7 are 16-bit; each instance has exactly A and B output identities; 24 numbered register subsections catalogued |
| SCI/IIC/SPI/ADC | R01UH chs.28, 29, 31 and 35 | Initial asynchronous-UART, IIC0/1, SPI0/1, and ADC14 scan/data slices with native widths, mixed field semantics, clocks, events, DMA requests and routes |

## Explicit limits and discrepancies

- `GAP-RA4M1-001` is accepted as a manual-revision scope boundary. The official
  part page exposed no separate device errata. Definitions are scoped to
  Rev.1.10 rather than asserted across every silicon revision; physical driver
  work must still inspect `MCUVER` and applicable technical updates.
- The hardware-manual text extraction is a navigation aid. Addresses and widths
  in the grouped catalogues, plus the reset masks and reserved-bit rules on the
  strongly typed representative registers, were checked against the linked PDF
  pages. Package rows were checked the same way. No conflict was found within
  the implemented slice.
- Register catalogues group repeated manual subsections (for example GTCCRA–F,
  ADDR0–14, and SARL/SARU0–2) while retaining the first address and source
  location. Strongly typed aliases cover the representative registers consumed
  by the common description contract.
- `PLN-0013` now accounts for all 48 chapters in the public
  `complete_source_inventory`. The earlier 13-row ledger remains as the
  strongly typed initial increment; its former deferred breadth is retained as
  source-unit coverage rather than as a driver-support claim.

## Verification

- The host restricted C++23 build compiles each RA4M1 header independently.
- The real-description contract is instantiated for `ra4m1::description`.
- Runtime metadata tests cover identity/provenance, coverage accounting,
  register aliases/semantics, protection, pins/routes, clocks, ICU routing,
  DMAC/DTC constraints, and GPT identities.
- Negative compile tests reject a pin absent from the 64-pin package and an
  unsupported capability.
- Cortex-M4, Cortex-M7, and ESP32-P4 RISC-V metadata-only builds compile the
  RA4M1 headers without board, RTOS, or target-selection macros.
