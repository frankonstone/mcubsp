# ST NUCLEO-H753ZI requirements

Board identity: `st/nucleo_h753zi`. Scope: STM32H753 on the MB1364 revision E carrier; STM32 descriptions, bare-metal backends and native USB/Ethernet extensions.

Read this with the [common requirements](../../../../requirements.md), whose decision
register and shared contracts apply within the selected scope. The
[board implementation plan](implementation-plan.md) owns target-specific work;
[the common plan](../../../../implementation-plan.md) owns shared milestones.

## Applicable common contracts

| Area | Common requirements |
| --- | --- |
| Description model and three-MCU gate | REQ-0005–REQ-0013; REQ-0040 |
| Reusable functional driver contracts | REQ-0021, REQ-0022, REQ-0024–REQ-0029 |
| Startup, cache/DMA constraints and target acceptance | REQ-0013, REQ-0031, REQ-0035–REQ-0037; REQ-0041 |
| Protocol-neutral Ethernet and MCUIP integration boundary | REQ-0043 |
| Application facade, attached devices and build workflows | REQ-0038, REQ-0045–REQ-0052 |

This table is a reading guide, not an additional set of trace relationships or
a claim that every capability is implemented. Explicit dependencies remain on
the canonical requirement definitions below and in the common document.

Native USB and Ethernet completion remains separate from the accepted GPIO/PWM/UART/I2C/SPI/ADC backend increment. The common Ethernet boundary retains its NUCLEO acceptance case.

## Board-specific requirements

## REQ-0033 STM32H753 description and subsequent bare-metal port
@depends(REQ-0003, REQ-0005)

The initial description phase shall select and record the STM32H753 device/package scope, review the locally available RM0433 reference manual, DS12117 datasheet and ES0392 errata, and implement the documented hardware in the shared model. Describe memory/cache/MPU properties, register semantics, clock/reset domains, pin routes, interrupt sources and DMA reachability without inheriting RA4M1 layout assumptions. PM0253 alone is insufficient for peripheral definitions.

After the common gate, the driver phase shall select a physical board and implement STM32 backends for the agreed functional driver set. Board bring-up, live cache/MPU operations and runtime driver verification belong to that later phase, not description validation. MCUOS is not a prerequisite for either.

STM32 backend implementation and board-native bring-up shall use ST-LINK debug/trace and virtual COM, built-in LEDs/buttons, documented solder bridges or point-to-point jumpers, the on-board Ethernet PHY/connector with an ordinary network peer, native connectors, and internal memory/peripheral known-answer tests. Ethernet work in this phase covers source-backed clock/pin setup, MDIO/PHY control, link-state verification and raw MAC-frame exchange, followed by the explicit MCUIP integration required by REQ-0043. Every physical board modification shall be recorded with its schematic designator, original state, resulting state, purpose and restoration procedure. Claims remain limited to the behavior independently observed by those target-specific procedures.

Acceptance: the same description contract tests used for RA4M1 and ESP32-P4 instantiate real STM32H753 definitions before any driver work. Later, common driver contract tests, cross-builds and bare-metal target examples pass after hardware/board binding changes. Target evidence is reported under REQ-0036 with its actual observation path and without claiming unmeasured electrical behavior.
