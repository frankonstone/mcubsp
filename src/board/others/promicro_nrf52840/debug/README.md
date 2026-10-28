# Pro Micro debug and trace

These optional scripts need an SWD probe. UF2/USB does not provide a debug
transport. Loading the scripts performs no reset, flash programming, UICR write,
CTRL-AP recovery or target-memory write. Start OpenOCD with your probe interface;
for example, for a CMSIS-DAP probe:

```sh
openocd -f interface/cmsis-dap.cfg -c 'transport select swd' \
  -f target/nrf52.cfg \
  -f src/board/others/promicro_nrf52840/debug/trace.tcl
```

Connect GDB with the ELF corresponding to the running application:

```sh
arm-none-eabi-gdb output/build/promicro-nrf52840/example/others.promicro_nrf52840.example.system.elf \
  -ex 'target extended-remote :3333' \
  -x src/board/others/promicro_nrf52840/debug/inspect.gdb
```

`mcubsp_identity`, `mcubsp_debug_units` and `mcubsp_faults` read factory/core
registers. Use GDB `hbreak <function>` and `watch <variable>` for FPB instruction
breakpoints and DWT data watchpoints; GDB/OpenOCD manages their finite comparator
slots. Nordic specifies six instruction and two literal FPB comparators, plus
four DWT comparators. Read the capabilities rather than assuming a slot is free.
The firmware execution clock uses CYCCNT; neither these scripts nor a trace
session may reset it or disable its counter.

## Trace resource handoff

Before calling the trace procedures, the application must hold HFXO running
through `clock_controller::acquire_hf()`, relinquish ITM/TPIU/ETM and their pins,
and be halted. This is an explicit debugger-owned session: the Tcl script cannot
inspect the application's C++ leases. The normal system example does not acquire
HFXO for trace or write ITM packets automatically. An application using these
features must prepare that handoff in its own composition.
Trace pins must use S0S1 or H0H1 drive with GPIO sensing disabled; the script
checks those settings before changing the mux. High drive may be needed for the
actual board wiring and analyzer load.

| Mode | Silicon trace pins | Capture |
| --- | --- | --- |
| SWO | P1.00 | External UART/logic analyzer at the selected baud |
| Parallel | P0.07 TRACECLK; P1.00/P0.12/P0.11/P1.09 DATA0–3 | External four-bit trace analyzer |

P1.00 and P0.11 conflict with the default I2C pins. Physical access to all trace
pins on this board revision is unverified; silicon support does not mean the
board exposes them. Parallel ETM capture needs a suitable trace analyzer; a
basic SWD probe alone is insufficient. SWO carries ITM, not ETM instruction trace.

After the handoff, use GDB's monitor commands:

```text
monitor halt
monitor mcubsp_trace_start swo 1 2000000
continue
```

The explicit `1` confirms that the application has relinquished the trace units
and pins. A parallel session uses `monitor mcubsp_trace_start parallel 1`.
SWO baud must divide the 32 MHz trace clock exactly. Parallel TRACECLK is 16 MHz;
trace IDs are ITM=1 and ETM=2. The parallel formatter is enabled. The ETM script
checks ETMv3.5 identity, programs the ETM-M4 registers under the programming
handshake and initializes registers without an architected reset value. It does
not use the ETMv4 layout found on newer MCUs.

Interrupt GDB execution and stop the session before returning these resources:

```text
monitor halt
monitor mcubsp_trace_stop
```

Stop disables the trace producers, waits for their handshakes, disables capture
and restores the pin mux and previous inactive ITM settings. It preserves
DEMCR.TRCENA and DWT/CYCCNT. Any ambiguous write or timeout retains the script's
ownership state; correct the cause and retry `mcubsp_trace_stop` before starting
another session. Debug Interface mode affects power consumption; current
measurements need a separate run with the probe detached.

## Software verification

`test/workflow/test_nrf_debug.py` uses a sparse Tcl hardware model to check
read-only loading, invalid requests, finite handshakes, ETM programming order,
DWT preservation and failed-cleanup recovery. It joins CTest/native reports when
`tclsh` is available. OpenOCD 0.12.0 also parsed the real configuration and ran a
mocked parallel start/stop using its Jim Tcl interpreter. No probe was connected
for these checks; physical trace capture remains pending.
