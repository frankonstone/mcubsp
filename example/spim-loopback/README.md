# Pro Micro SPIM EasyDMA loopback

Build `others.promicro_nrf52840.example.spim-loopback` with the board preset.
This directory also builds against a source checkout (`MCUBSP_SOURCE_DIR`) or
an installed package (`CMAKE_PREFIX_PATH`) with the matching board toolchain
and `MCUBSP_BOARD=others/promicro_nrf52840`.

Connect P0.08 MOSI to P0.07 MISO. P0.06 is SCK and P0.12 is active-low CS.
Do not attach a second driver to those signals. All signals use 3.3 V logic.
The application uses mode 0, MSB first, 1 MHz and an eight-byte RAM pattern.
Successful comparisons toggle the built-in LED. `mcubsp_spim_status` and
`mcubsp_spim_rounds` expose progress for a debugger.

The native backend supports explicit SPIM0--SPIM3 bindings, optional data/CS
pins, asymmetric buffers, ORC and a bounded number of segments with CS held
across the transaction. This example selects SPIM0. SPIM3 additionally requires
a reviewed silicon identity and an explicit dedicated AHB RAM slave reservation
in the application's storage/linker layout for erratum 198. CPU/ISR accesses
through either RAM alias must avoid that slave while SPIM3 transmits. The pool
reserves its whole span against other DMA users. No PPI or SUSPEND API is
exposed here; the SPIM3 limitations for those paths remain separate work.

Consume or cancel the transfer before teardown. Failed stop acknowledgment
retains DMA memory, CS, clock and instance until `shutdown()` establishes
quiescence. Physical loopback, independent slave traffic, wire timing and
SPIM3 RAM-layout acceptance remain pending. This increment adds no legacy SPI
or SPIS slave implementation.
