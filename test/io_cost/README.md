# I/O facade size comparison

Run from the repository root, with the GNU Arm Embedded toolchain on `PATH`:

```sh
python3 tool/report-io-cost.py --output-dir build/io-cost
```

Or configure CMake with `-DMCUHW_BUILD_IO_COST_REPORT=ON` and build the
`mcuhw_io_cost_report` target. This target deliberately uses the Arm toolchain
even when the main build runs host tests.

Every run compiles and links fresh paired probes for GPIO, UART blocking,
UART asynchronous, and full-duplex SPI DMA. Both sides use the existing checked
RA4M1 backends, the same execution hooks, direct MMIO, clocks, buffer sizes,
compiler flags, restricted runtime and linker script. The driver side assembles
and polls its backend explicitly. The facade side adds board composition,
duration conversion, shared resource ownership and operation lifetime checks.

These are complete linked size probes, **not runnable firmware**: they have no
reset vector, clock startup, or data/bss initialization. Never flash the ELFs.
The injected execution hooks assume a running 48 MHz DWT counter, as does the
facade timeout conversion in these probes. The board startup cost is deliberately
excluded from both sides. Firmware costs also include startup, vectors and any
application-specific interrupt routing.

`report.md` compares flash and static RAM. `report.json` records compiler and
binutils versions, exact commands, input hashes, ELF hashes, section sizes and
compiler stack records. The source hash includes all public mcuhw headers and
probe sources, so a result cannot be mistaken for a measurement of an older
checkout. No pre-existing firmware artifacts are read.

Flash includes code, constants, constructor tables, unwind tables if present,
and the load image of initialized data. Static RAM is `.data + .bss`; the linker
script allocates no heap. Buffers have identical storage duration on both sides.
Stack records describe individual compiler-generated function frames, not the
sum along a call chain or an interrupt nesting bound. They are retained for
inspection instead of being presented as total stack usage.

These measurements establish linked footprint for four concrete uses. They do
not measure execution latency, prove hardware behavior, or promise that every
configuration has the same overhead. Enabling another peripheral intentionally
allocates its backend state; disabled peripheral backends should be absent.
