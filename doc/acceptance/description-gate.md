# Three-MCU description gate

Gate review for `PLN-0013` and `PLN-0017`, 13 September 2026.

## Outcome

The description gate is accepted for the declared RA4M1, STM32H753 and
ESP32-P4 baselines. The public descriptions expose 177 source units: 176 are
reviewed and verified, while the announced ESP32-P4 I3C unit is explicitly
`unavailable-in-source`. No unavailable unit is counted as verified. Driver
support remains a separate state.

The user's request to implement this gate follows the previously disclosed I3C
gap and accepts its exact exclusion for this baseline. This is recorded as D09:
I3C has no capability, instance, register placeholder, or support claim until a
new authoritative ESP32-P4 source revision is pinned and reviewed. The
pre-release ESP32-P4 baseline and the recorded 360/400 MHz source conflict stay
visible and trigger re-review on any source update.

## Reconciled denominators

| Target | Declared denominator | Verified | Unavailable | Scope |
| --- | ---: | ---: | ---: | --- |
| RA4M1 | 48 | 48 | 0 | `R7FA4M1AB3CFM#AA0`, 64-pin LQFP, R01UH/R01DS Rev.1.10 |
| STM32H753 | 61 | 61 | 0 | xI device set/eight packages; RM0433 Rev.8, DS12117 Rev.10, ES0392 Rev.15 |
| ESP32-P4 | 68 | 67 | 1 | NRW16X/NRW32X, QFN104, production v3.x, preliminary TRM/datasheet v0.7 |
| Total | 177 | 176 | 1 | I3C is the sole unavailable source unit |

The machine-readable inventories live beside each target model in
`coverage.hpp`. Every row identifies the unit and first PDF page, separates
description from driver state, records whether register/field/route categories
are accounted for, and retains revisioned provenance and a decision. The common
validator rejects duplicate rows, unfinished description states, incomplete
category accounting, and unavailable units presented as verified.

## Native differences retained

| Concern | RA4M1 | STM32H753 | ESP32-P4 |
| --- | --- | --- | --- |
| Pins | PFS selection on package pins | GPIO alternate-function numbers and package membership | direct IO-MUX functions plus programmable GPIO matrix; HP/LP ownership |
| Clock graph | six selectable roots feeding divided ICLK/PCLK/FCLK domains | SYS/HCLK/PCLK tree, timer kernels and ADC kernel mux | HP root/CPU/memory/system/APB chain plus independent LP domains and fractional dividers |
| Interrupts/events | ICU event codes routed to 32 selectable CPU lines | fixed NVIC lines, including shared vector lines | peripheral sources independently routed through each HP-core matrix to 32 inputs |
| DMA/memory | distinct DMAC and SRAM-descriptor DTC | DMA1/2, BDMA and MDMA with TCM/D1/D2/D3 reachability differences | GDMA-AHB, GDMA-AXI, VDMA and 2D-DMA with cached/direct and MIPI memory domains |
| Timers/PWM | GPT0/1 32-bit, GPT2-7 16-bit, exactly A/B outputs | advanced/general/basic timer families; up to seven complementary channel identities | 52-bit system timer, 54-bit TIMG, 20-bit/eight-output LEDC and six-output MCPWM |

The shared contract therefore compares concepts and constraints, not register
layout. Vendor encodings, register types, capability sets and native instance
records remain accessible in their target namespaces.

## Verification boundary

The gate requires host metadata tests, independent public-header compilation,
one translation unit containing all three descriptions, and Cortex-M4,
Cortex-M7 and ESP32-P4 RISC-V compile probes. It does not execute MMIO or prove
startup, board wiring, interrupt dispatch, DMA completion, cache maintenance,
multicore operation, functional drivers, or RTOS behavior.
