# Waveshare ESP32-P4-NANO implementation plan

Implements the [board requirements](requirements.md) and their applicable
[common contracts](../../../../requirements.md). Shared foundation, description-gate,
application/device, repository and later MCUOS work stays in the
[common implementation plan](../../../../implementation-plan.md).

The items below retain their existing IDs, explicit requirement links, status,
prerequisites and verification notes. Read them in delivery order, not numeric
ID order. MCU description/backend work is owned here for this initial carrier
but remains reusable by other boards on the same MCU.

Revision-v1.0 observations remain development evidence outside the accepted production-v3.x silicon range. SDMMC/SDIO, audio, Ethernet and wireless claims retain the individual limits recorded in PLN-0019.

A `done` status records the stated increment only. Physical observations and
unexecuted acceptance cases remain distinct in each item and in the
[support matrix](../../../../doc/reference/support-matrix.md).

## PLN-0015 Describe ESP32-P4 using the shared model
@status(done)
@link(REQ-0003, REQ-0004, REQ-0005, REQ-0006, REQ-0007, REQ-0010, REQ-0011, REQ-0012, REQ-0013, REQ-0034, REQ-0040)

Prerequisite: PLN-0003 and the ESP32-P4 source baseline from PLN-0001. May be interleaved with RA4M1 and STM32 description work.

- Describe HP/LP and memory domains, GPIO matrix/IO MUX, reset/clocks, interrupt matrix, DMA engines and the same initial peripheral categories. Represent real basic/motor PWM hardware rather than copying GPT semantics.
- Use RISC-V/ESP-specific metadata where appropriate while preserving common concepts. Make unsupported capabilities and source gaps explicit.
- Extend through remaining documented blocks in reviewed increments. Compile real definitions against the same tests and the RISC-V metadata probes; do not equate metadata with working boot or multicore execution.

Exit: the declared ESP32-P4 baseline is described and reviewed. Unpublished content is handled under the common gate. Bare-metal backends and board execution belong to PLN-0019.

## PLN-0019 Implement ESP32-P4 bare-metal backends
@status(in_progress)
@link(REQ-0003, REQ-0004, REQ-0009, REQ-0010, REQ-0011, REQ-0013, REQ-0014, REQ-0016, REQ-0017, REQ-0019, REQ-0024, REQ-0025, REQ-0026, REQ-0027, REQ-0028, REQ-0031, REQ-0034, REQ-0036, REQ-0037, REQ-0041, REQ-0043)

Prerequisite: PLN-0018 and the already accepted ESP32-P4 descriptions. Use the board's native connectors, on-board devices, direct header connections and ordinary peers for target-specific evidence.

Current ESP32-P4-NANO development evidence uses a custom second-stage loader and a separately linked factory application. The loader owns only initialization, partition validation and the fixed factory-selection policy; `mcutest`, MCUCOV and the functional GPIO/UART/I2C/I2S/audio/SDMMC/SDIO/SPI/Ethernet code reside in the application. ESP-IDF completes image/cache/libc/constructor and heap initialization, then the application takes control before main-task creation, so no FreeRTOS scheduler is started. The 158-case native host suite includes ESP32-P4 I2C timing, matrix routing, write/read/repeated-START command lists, explicit STOP, bounded FIFO validation, address/data NACK attribution, arbitration-loss, timeout, cancellation and recovery; I2S fractional timing, Philips-slot and directional GPIO-matrix routing, AHB-PDMA clock/reset/descriptor/completion, cache-policy, cancellation and ES8311 sequence tests; SD-card protocol initialization, capacity decoding and sector read/write plus native SDMMC clock, dedicated and matrix pin routing, slot/card selection, LDO, command and IDMAC behavior; scheduler-independent SDIO CMD5/RCA/CCCR/CIS/function-enable/block-size/four-bit enumeration; GPSPI2 timing, mode/order/width/CS configuration, one-command multi-segment ownership, completion, error, cancellation and CPU-buffer bounds; and native EMAC/RMII clock, route, MDIO, IP101 identity/link, fixed descriptor-ring, cache and MCUIP-adapter contracts. On the connected revision-v1.0 device, eight registered mcutest cases pass: GPIO2 register readback; I2C0 100 kHz register-select/repeated-START reads of the on-board ES8311 codec identity `0x8311` at address `0x18` on GPIO8/GPIO7; I2S0/ES8311 48 kHz capture of 4,032 bytes with 2,016 nonzero analog-microphone samples followed by a completed 193,536-byte, 1,008 ms, 1 kHz DAC tone under GPIO53 amplifier control; native SDMMC identification of the 61,454,336-sector SDHC card plus sector-0 read and last-sector write/read/restore verification on GPIO39-GPIO45; native SDIO slot-1 enumeration of the GPIO14-GPIO19 ESP32-C5 wireless coprocessor after GPIO54 reset, including two advertised functions, RCA `0x0001`, CCCR `0x32`, CIS `0x01000`, Espressif identity `0x0092/0x6666`, function-1 ready, 512-byte block sizes and four-bit mode; one GPSPI2 1 MHz four-frame, two-segment command on expansion GPIO20-GPIO23; a UART0 finite transfer whose marker is physically received through the on-board CH343 USB bridge after TX-done completion; and native EMAC/RMII/SMI initialization plus physical identification of the on-board IP101GRI at PHY address 1 with ID1 `0x0243`, ID2 `0x0c54`, OUI `0x0090c3` and model `0x05`. The PHY reported BMSR `0x7849` and link down because no Ethernet peer was connected, so raw-frame exchange and the physical MCUIP smoke path remain open. Revision v1.0 uses different I2C0 APB-gate/reset bits, retained as an explicit development-board binding without altering the production-v3.x description. The MCUCOV FULL-profile capture reports 180/279 functions, 1129/1886 statements, 1275/2010 lines, 486/1102 branches and 72/880 MC/DC conditions. The on-board codec provides physical I2C transaction and live microphone/DMA evidence; the test deliberately produces an audible tone, while independent level/frequency/distortion measurement, timing/protocol capture and injected I2C faults are not claimed. The SDMMC result proves 4-bit physical block I/O and restoration but does not claim filesystem or multi-block support. The C5 result proves the physical SDIO link, identity and function readiness but not ESP-Hosted packet transport, remote Wi-Fi API operation, scanning, association or IP connectivity. UART RX/error stimulus still needs a direct loopback if that behavior is claimed. The SPI result proves controller execution and idle completion but, with no external slave, loopback or logic analyzer attached, does not claim returned data or electrical timing/CS-waveform behavior. The Ethernet result proves native MAC/pin/clock/reset setup and SMI communication with the physical PHY; live link, electrical RMII traffic, raw frames and protocol traffic still require an ordinary cabled peer. The v1.0 result remains development-only evidence outside the accepted v3.x silicon range.

Choose the board, toolchain/boot path and initial core ownership. Implement the common driver set against existing Espressif descriptions, validating actual clock, routing, memory/cache and DMA behavior. Keep selected boot support separate from functional drivers and ensure the delivered execution mode does not require an RTOS scheduler. Run common and physical target tests; concurrent multicore/LP support needs separate evidence.

Exit: all three declared bare-metal targets have real backends supporting the shared interfaces, with host/cross-build evidence and their selected board-native/directly connected target procedures. Description compatibility and runtime portability are separately demonstrated; observation limits remain explicit.
