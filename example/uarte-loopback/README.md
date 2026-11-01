# Pro Micro UARTE EasyDMA loopback

Build `others.promicro_nrf52840.example.uarte-loopback` with the board preset,
or configure this directory with `MCUBSP_SOURCE_DIR` (source consumption) or
`CMAKE_PREFIX_PATH` (installed package). Use `MCUBSP_BOARD=others/promicro_nrf52840`
and the matching board toolchain. Upload is a separate hardware procedure.

Connect P0.06 TX to P0.08 RX. This is 3.3 V logic UART, not USB serial. The
application compares an eight-byte binary pattern and toggles the built-in LED
after each successful exchange. Inspect `mcubsp_uart_status` and
`mcubsp_uart_rounds` with a debugger if one is available. No flow-control pins
are configured. The TX buffer is writable Data RAM; flash constants cannot be
passed directly to EasyDMA.

The example uses UARTE0. Select `peripheral::uarte1` explicitly in the binding
to use UARTE1; the shared UART0/UARTE0 ownership remains enforced. Native
polling is foreground-driven. Caller buffers, the shared DMA pool, context and
clock controller outlive the operations. A failed cancellation retains DMA
memory until explicit `shutdown()` succeeds; it is not permission to reuse it.

This example is prepared for physical testing. No new upload, electrical baud
measurement, independent-peer exchange or flow-control result is claimed.
