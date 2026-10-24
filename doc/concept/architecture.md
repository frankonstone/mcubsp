# Architecture and layer boundaries

`mcubsp` separates hardware facts, hardware sequencing, functional behavior,
and board wiring:

```text
application
    ↓
functional driver
    ↓
MCU backend ← board binding
    ↓             ↑
MMIO policy ← MCU description
```

## Core

`include/mcubsp/core` defines register metadata, typed units, MMIO contracts,
fixed ownership, and transfer state. Core code contains no MCU addresses.

## MCU descriptions

`include/mcubsp/mcu/<mcu>` records source-backed memory, clock, peripheral, pin,
interrupt, DMA, and capability metadata. Descriptions do not start hardware or
depend on a board.

## Backends and drivers

`include/mcubsp/backend/<mcu>` implements native register sequencing for a
peripheral family. `include/mcubsp/driver` provides functional APIs and external
device drivers. A common driver does not require unrelated MCUs to imitate one
vendor's register layout.

## Boards and execution

`include/mcubsp/board` maps exact board connectors and devices to MCU pins.
`board/<board>` owns startup, linker configuration, interrupt entry points, and
physical tests. Execution policies bind critical sections, time, ISR context,
progress, and notification without requiring an RTOS.

See [Register and metadata model](register-model.md) and
[Execution and transfers](execution-and-transfers.md) for the core contracts.
The [bare-metal application integration guide](application-integration.md)
shows how startup, `main()`, execution hooks, static driver objects, transfer
buffers, and interrupt handlers fit around those layers.
