# Waveshare ESP32-P4-NANO requirements

Board identity: `waveshare/esp32p4_nano`. Scope: ESP32-P4 description baseline and the ESP32-P4-NANO board integration, including its explicitly separate revision-v1.0 development evidence.

Read this with the [common requirements](../../../../requirements.md), whose decision
register and shared contracts apply within the selected scope. The
[board implementation plan](implementation-plan.md) owns target-specific work;
[the common plan](../../../../implementation-plan.md) owns shared milestones.

## Applicable common contracts

| Area | Common requirements |
| --- | --- |
| HP/LP-aware description model and three-MCU gate | REQ-0005–REQ-0013; REQ-0040 |
| Selected functional driver contracts | REQ-0021, REQ-0024–REQ-0028 |
| Boot integration, memory/DMA and scoped target acceptance | REQ-0013, REQ-0031, REQ-0035–REQ-0037; REQ-0041 |
| Protocol-neutral Ethernet boundary | REQ-0043 |
| Application facade, attached devices and build workflows | REQ-0038, REQ-0045–REQ-0052 |

This table is a reading guide, not an additional set of trace relationships or
a claim that every capability is implemented. Explicit dependencies remain on
the canonical requirement definitions below and in the common document.

Revision-v1.0 observations remain development evidence outside the accepted production-v3.x silicon range. SDMMC/SDIO, audio, Ethernet and wireless claims retain the individual limits recorded in PLN-0019.

## Board-specific requirements

## REQ-0034 ESP32-P4 description and subsequent bare-metal port
@depends(REQ-0003, REQ-0005)

The initial description phase shall pin reviewed documentation and silicon/package scope and implement ESP32-P4 in the shared model. Describe HP/LP domains and CPU ownership constraints, GPIO matrix/IO MUX routing, interrupt matrix, clock/reset relations, memory/cache properties, DMA engines and actual timer/PWM capabilities without mimicking RA4M1 GPT or Arm core registers. Preliminary or missing definitions shall remain explicit source gaps.

After the common gate, select a physical board, boot/toolchain integration and initial core-ownership model, then implement the agreed functional driver set through Espressif-specific backends on bare metal. Hardware multicore/LP concurrency is a separate validated capability. MCUOS availability is not a prerequisite.

Acceptance: real ESP32-P4 definitions satisfy the common description tests before any driver work. Later, common driver tests and physical bare-metal examples pass using target bindings. Description-only success shall not imply verified boot, cache, DMA or multicore execution.
