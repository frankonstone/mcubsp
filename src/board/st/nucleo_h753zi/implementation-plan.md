# ST NUCLEO-H753ZI implementation plan

Implements the [board requirements](requirements.md) and their applicable
[common contracts](../../../../requirements.md). Shared foundation, description-gate,
application/device, repository and later MCUOS work stays in the
[common implementation plan](../../../../implementation-plan.md).

The items below retain their existing IDs, explicit requirement links, status,
prerequisites and verification notes. Read them in delivery order, not numeric
ID order. MCU description/backend work is owned here for this initial carrier
but remains reusable by other boards on the same MCU.

Native USB and Ethernet completion remains separate from the accepted GPIO/PWM/UART/I2C/SPI/ADC backend increment. The common Ethernet boundary retains its NUCLEO acceptance case.

A `done` status records the stated increment only. Physical observations and
unexecuted acceptance cases remain distinct in each item and in the
[support matrix](../../../../doc/reference/support-matrix.md).

## PLN-0014 Describe STM32H753 using the shared model
@status(done)
@link(REQ-0003, REQ-0004, REQ-0005, REQ-0006, REQ-0007, REQ-0010, REQ-0011, REQ-0012, REQ-0013, REQ-0033, REQ-0040)

Prerequisite: PLN-0003 and the STM32 sources from PLN-0001. No RA4M1 driver or MCUOS prerequisite.

- Implement real STM32H753 part/package, memory/core metadata, clocks/reset, pins, interrupts, DMA and the same initial peripheral categories using the common contracts.
- Represent native differences explicitly, including alternate-function routing, clock-domain relations, timer capabilities and DMA-accessible memory/cache properties. Revise shared contracts across all descriptions when needed.
- Extend in reviewed blocks through the remaining documented STM32H753 inventory. Validate with the shared tests and Cortex-M7 metadata compile probes.

Exit: the declared STM32 baseline is described and reviewed, with source gaps explicit. Bare-metal runtime backends and board execution are deferred to PLN-0018.

## PLN-0018 Implement STM32H753 bare-metal backends
@status(done)
@link(REQ-0003, REQ-0004, REQ-0009, REQ-0013, REQ-0014, REQ-0017, REQ-0019, REQ-0021, REQ-0022, REQ-0024, REQ-0025, REQ-0026, REQ-0027, REQ-0028, REQ-0029, REQ-0031, REQ-0033, REQ-0036, REQ-0037, REQ-0041)

Prerequisite: the accepted STM32 descriptions and PLN-0017. The RA4M1 release is not a prerequisite. Use NUCLEO-H753ZI MB1364 Rev E and keep board configuration separate from MCU metadata.

1. Inventory the physical board assembly, jumper state and relevant solder bridges. Before changing a bridge, record its schematic designator, original and target state, purpose and restoration procedure.
2. Complete startup, clocks, memory layout, vectors, GPIO and debug procedures using ST-LINK and built-in board features.
3. Implement the common driver set against the existing descriptions. Use built-in LEDs/buttons, ST-LINK virtual COM, documented point-to-point jumper/bridge loopbacks, native USB where selected, and the on-board Ethernet PHY/connector with an ordinary network peer. Ethernet scope here is clock/pin setup, MDIO/PHY control, link state and raw MAC-frame exchange, not an implicit TCP/IP stack.
4. Validate cache/MPU behavior and DMA constraints with internal memory/peripheral known-answer tests and native-connector traffic where that gives independent observation.
5. Reuse functional tests and external-device sources with hardware bindings changed. Revalidate all three description contracts if the port exposes a model error. Record any unmeasured timing, analog, electrical or fault behavior as an explicit non-claim.

Completed 19 September 2026. The NUCLEO-H753ZI MB1364 Rev E startup boots
from `0x08000000`, verifies reset-state HSI at 64 MHz, owns the vector table and
AXI-SRAM runtime, and drives on-board LD2 through the shared GPIO interface.
The STM32H753 backends now implement TIM/PWM, USART, I2C, SPI and ADC against
the same functional interfaces used by RA4M1. They retain STM-native timing
encodings, compile-time package/AF bindings, finite operation pools, typed
errors, cancellation and final-event completion. GPIO, startup/clock and the
Cortex-M7 barrier path remain covered by their earlier shared implementations.

The native host suite instantiates all backends with instrumented MMIO and fake
events: 123 mcutest cases pass, and all 23 CTest entries pass including negative
compile constraints. Public headers build independently; the Cortex-M7 probe
and barrier-instruction check pass. The all-backend Nucleo image evaluates its
timing solvers at compile time, links without a hosted runtime or heap, and its
build reports flash/static-RAM use.

The Cortex-M7 board image is also an instrumented mcutest/mcucov suite. Eight
requirement-tagged tests run on the Nucleo and pass with no registry overflow.
The SWD runner extracts the binary coverage frame from cache-cleaned AXI SRAM
and produces JSON, HTML and VS Code-compatible LCOV reports. The imported target-specific report covers
startup/clock plus every STM32 backend module in this plan: 96/171 functions,
512/1062 statements, 564/1061 lines, 185/602 branches and 19/502 MC/DC
conditions. mcucov records 46 unsupported or constant-evaluated constructs as
not instrumented rather than presenting them as coverage. mcutrace imports both
the on-board mcutest and mcucov artifacts with no validation error.

The reproducible `run-nucleo-h753zi-native.sh` procedure passed on the connected
board with STLINK V3J5M2 at 3.28 V. Its result mask was `0x000000ff`: the
built-in LED passed set/read/clear through the common GPIO driver; TIM1
configured ARR=3199/CCR1=800 for the 20 kHz 25% case; USART3 BRR=556 and the
physical PD8/PD9 ST-LINK VCP delivered `MCUBSP H753 USART3 OK`; I2C1 and SPI1
initialized on the source-backed Arduino pins; ADC1 calibrated and converted
floating A0/PA3 within the 14-bit range. The test also enabled the Cortex-M7
D-cache, cleaned a 64-byte aligned AXI-SRAM DMA1 source, and independently read
back `0xCACE0000..3` through SWD. No board jumper or solder bridge was changed.

`doc/board/nucleo-h753zi/README.md` records the board/source identity, received
configuration, commands, resource model, observation limits and non-claims.
PWM waveform measurement, controlled/calibrated ADC levels, real I2C/SPI
targets, peripheral-DMA traffic and supervised fault/electrical cases were not
executed and are not claimed. A D11-D12 SPI bridge and controlled A0 ties are safe
direct procedures but were not installed and are recorded as unexecuted,
not simulated evidence. Ethernet and native USB are independent board-native
extensions tracked by PLN-0023 rather than silently folded into this agreed
GPIO/PWM/UART/I2C/SPI/ADC backend exit.

Exit: the agreed STM32 backends pass native tests, Cortex-M7 builds and the selected board-native/direct-wired procedures; board modifications and observation limits are documented. No MCUOS Cortex-M7 port is required.

## PLN-0023 Implement Nucleo native USB and Ethernet connectivity
@status(in_progress)
@link(REQ-0003, REQ-0004, REQ-0009, REQ-0013, REQ-0033, REQ-0036, REQ-0037, REQ-0041, REQ-0043)

Prerequisite: PLN-0018 startup, memory/cache rules and board binding. Use the received
MB1364 Rev E assembly and ordinary USB/Ethernet peers; record any required
solder-bridge or jumper change before making it.

- Extend the source-backed STM32 description only where the USB or Ethernet
  backend needs currently absent MAC/PHY, MDIO, DMA descriptor, USB OTG or pin
  metadata. Re-run the three-description gate after shared-model changes.
- Bring up the on-board Ethernet PHY and connector: source/validate clocks and
  pins, read/write PHY registers over MDIO, observe link state, and exchange
  raw Ethernet frames with an ordinary peer. Keep this backend protocol-neutral,
  then add the explicit target adapter required by REQ-0043 against the actual
  sibling `../mcuip` public contracts. Do not add lwIP.
- Cross-build and link MCUIP with the Nucleo backend, then exchange ARP plus an
  ICMP echo or UDP datagram with an independent peer. Exercise link cycling,
  descriptor exhaustion, completion ordering and cache maintenance using fixed
  storage with the data cache enabled.
- Select the native USB role and connector path, implement the bounded endpoint
  and transfer subset required by the chosen smoke protocol, and exchange data
  with an ordinary host. Keep USB clock, FIFO and cache/DMA constraints explicit.
- Record link/USB captures, image sizes, cache maintenance, board state and
  repeatable host-side procedures. Keep unmeasured fault/electrical behavior as
  explicit non-claims.

Exit: Ethernet MDIO/PHY/link/raw-frame, the MCUIP protocol smoke path and the
selected native USB smoke path pass repeatable board-native procedures with
source-backed metadata and bounded storage. No lwIP, adapter hardware or MCUOS
dependency is implied.
