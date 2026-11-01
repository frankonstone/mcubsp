# nRF52840 serial families

This 2 November 2026 delivery implements the PLN-0045 software interfaces and
the relevant PLN-0042/0043/0044 prerequisites. Physical qualification is pending:
the user has only a Pro Micro nRF52840, currently unavailable. No upload or
electrical measurement was performed. PLN-0045 remains in progress until its
peer and timing exit criteria are met. The broader prerequisite plans retain
their unrelated unfinished scope.

## Interfaces and ownership

| Family | Instances | Contract |
| --- | --- | --- |
| CPU UART | UART0 | Independent finite TX/RX; bounded caller-buffer RX queue; CTS/RTS; byte/error/RXTO service |
| DMA UART | UARTE0/1 | Independent finite TX/RX; bounded caller-buffer RX queue; CTS/RTS; ENDRX handoff; RXTO/private FIFO flush |
| CPU SPI master | SPI0/1/2 | All four modes and both bit orders; 8-bit wire frames; optional data/CS pins; bounded multi-segment CS transaction |
| DMA SPI master | SPIM0/1/2/3 | All four modes and both bit orders; asymmetric DMA counts/ORC; bounded multi-segment CS transaction; STOPPED before release |
| DMA SPI slave | SPIS0/1/2 | Active-low CSN; all four modes and both orders; explicit TX/RX spans, DEF/ORC, counts and OVERREAD/OVERFLOW flags |
| CPU I²C master | TWI0/1 | Existing finite CPU backend and address/device leases retained |
| DMA I²C master | TWIM0/1 | Finite read/write; both repeated-START orders; counts/NACK causes; STOPPED before release |
| DMA I²C target | TWIS0/1 | Two address slots; READ/WRITE requests; explicit stretched-clock response; bounded one-write/one-read transaction; counts/error source |

Every native instance has an explicit binding. GPIO and serial personalities
share the resource registry, including the SPI/TWI aliases at IDs 3 and 4 and
the UART0/UARTE0 alias. Caller DMA buffers use the same fixed lease pool.
No generic DMA channel is invented. RAM/count/overlap checks happen before a
task can grant DMA access. Failed cancellation retains DMA storage, pins,
instance and HFXO until a later hardware acknowledgment permits recovery.

`receive_stream(initial_buffers, maximum_bytes)` uses four fixed metadata slots
by default. Every initial buffer and later submission is caller-owned. Take
completed blocks with `take_received_block()` and explicitly requeue storage;
sequence numbers preserve ordering. The total byte budget truncates the last
block. Empty/overlapping submissions fail; an exhausted queue ends with an
observable overrun. Completed blocks are never automatically reused, and must
be consumed before starting another receive. Cancellation/explicit stop exposes
the partial count. RXTO is a STOP acknowledgment, not an idle-line detector.

UARTE keeps automatic ENDRX→STARTRX shortcuts disabled. Poll/IRQ service proves
the previous DMA block ended, publishes it, programs the next prepared block
and issues STARTRX explicitly. Hardware flow control can provide FIFO
backpressure. Without it the application must meet the FIFO service latency;
no gap-free throughput or unlimited stream is promised. A failed handoff write
cannot attribute the previous block's AMOUNT to a buffer whose START was never
issued. Tail data is flushed into private RAM and reported as overrun.

SPIS holds the CPU semaphore while configuring buffers, arms leases before
RELEASE and enables END→ACQUIRE. Each submission grants at most one CSN
transaction. ENDRX alone never releases storage. Cancellation requests ACQUIRE
and waits for CPU ownership; a master holding CSN low can delay this. Subsequent
ungranted transactions emit DEF. Granted excess clocks emit ORC and expose
OVERREAD/OVERFLOW alongside actual stored/transmitted counts. Shutdown applies
Nordic's residual-byte reset after disable.

TWIS enables only during `listen()`. Both address events suspend the clock;
`poll()` exposes `pending_request()` with direction, matched slot/address and
previous phase counts. The caller then supplies missing storage with
`provide_receive()`/`provide_transmit()` and calls `resume()`. A repeated START
may switch direction once. Each direction is prepared at most once per listen;
a consumed buffer cannot silently be overwritten. While a repeated-start
request is stretched, the previous received prefix is stable for constructing
the reply; its lease remains held against other DMA clients. Extra same-direction
requests require cancellation/new listen. Master DNACK is reported normally;
OVERREAD/OVERFLOW cause STOP with error bits and actual counts. STOPPED and an
observed disable precede returning leases. Address information is retained even
when the master aborts with STOP before foreground service. There is no hidden
event queue: delayed service producing ambiguous address events fails explicitly.

## Common I/O composition

The Pro Micro profile now supports serial and SPI. Default serial is CPU UART0,
TX P0.06/RX P0.08, so ordinary text/flash spans work. Default SPI is SPIM1,
SCK P0.20/MOSI P0.22/MISO P0.24/default CS P0.17; it accepts byte spans in Data
RAM. Default I²C remains CPU TWI0, SCL P0.11/SDA P1.00.
These serial/SPI routes are explicit native-pin defaults, not an assertion about
Arduino numbering on every clone. Applications can replace all routes.

Use `io::serial_selection<uart_binding<...>>` for UART0/UARTE0/1,
`io::spi_selection<spi_binding<...>>` or `spim_binding<...>` for each master,
and `io::i2c_selection<twim_binding<...>>` for either DMA I²C master. Existing
`i2c_binding` selects the CPU TWI backend. Binding-specific timing solvers and
all CTS/RTS pins participate in facade reservations. Selected peripherals share
one board-context GPIO/native service context, HFXO controller and DMA pool.
SPI and I²C device leases retain existing CS/address/exclusive-operation rules.
The facade is foreground-polled; native APIs additionally support caller-owned
IRQ dispatch. Native SPIS/TWIS deliberately expose their hardware protocols
outside the common master bus/device interface.

The facade adds `read_stream_async()`, `take_read_block()`, `queue_read_buffer()`
and `stop_read()` when its selected backend supports them. DMA TX/RX still
requires Data RAM; flash-backed literal TX fails validation. SPI DMA uses byte
segments, not 16-bit host storage for 8-bit frames. CPU SPI can use either
common representation. A failed native SPI cancellation may be recovered with
`recover_select()` after consuming its result and receiving late STOPPED/READY;
it quiesces the backend, permits device release and board shutdown. Retired
operation slots remain retired; reconstruct the board after shutdown before
reusing a faulted controller. Failed initialization that retains hardware also
retains facade claims until successful shutdown.

## Relevant source gate

The pinned PS 4413_417 v1.11 supplies CPU SPI §6.24, SPIM §6.25, SPIS §6.26,
TWI §6.30, TWIM §6.31, TWIS §6.32, UART §6.33 and UARTE §6.34. The new
controllers use the already reconciled typed descriptions. The sequencing
review checked these source-specific details:

| Mode | Reviewed facts and restrictions |
| --- | --- |
| CPU SPI | ENABLE=1; READY 0x108/RXD 0x518/TXD 0x51C; one outstanding wire byte; no STOP task; READY before CS release |
| SPIS | ENABLE=2; ACQUIRE/RELEASE 0x24/0x28; SEMSTAT 0x400; END_ACQUIRE bit 2; MISO 0x50c/MOSI 0x510; STATUS 0x440 W1C; END releases DMA |
| TWIS | ENABLE=9; READ/WRITE 0x168/0x164; both SUSPEND shortcuts; PREPARERX/TX 0x30/0x34; ERRORSRC 0x4d0; MATCH 0x4d4; STOPPED releases DMA |
| UART/UARTE | Different baud encodings; native flow-control pair; bounded FIFO/drain semantics; ENDRX handoff and RXTO/FLUSHRX ordering |
| SPIM/TWIM | STOPPED-gated ownership; native PTR/MAXCNT/AMOUNT; zero-count pointer handling; repeated START/CS segment ordering |

The pinned nrfx v3.2.0 `nrf52_erratas.h` hash remains
`3a78e02923587f03922b9939b3470cac2ca6002f138aece0bbb1c72cef3f5ef4`.
Its FICR part/revision words are distinct from INFO.VARIANT. Unknown/Engineering A
UARTE retains the conservative 255-byte/one-stop-bit limits. Affected SPIM
single-byte modes and unreviewed SPIM3 are rejected. SPIM3 requires an explicit
dedicated whole AHB RAM slave; `spim_binding<3, Sck, Mosi, Miso, Cs, Slave>` makes
the same reservation selectable through the facade. CPU/ISR accesses, including
RAM aliases, must also be excluded by the application/linker layout.

Relevant errata remain [58](https://docs.nordicsemi.com/r/bundle/errata_nrf52840_enga/page/err/nrf52840/engineeringa/latest/anomaly_840_58.html),
[127](https://docs.nordicsemi.com/r/bundle/errata_nrf52840_enga/page/err/nrf52840/engineeringa/latest/anomaly_840_127.html),
[131](https://docs.nordicsemi.com/r/bundle/errata_nrf52840_enga/page/err/nrf52840/engineeringa/latest/anomaly_840_131.html),
[195](https://docs.nordicsemi.com/r/bundle/errata_nrf52840_rev3/page/err/nrf52840/rev3/latest/anomaly_840_195.html),
[198](https://docs.nordicsemi.com/r/bundle/errata_nrf52840_rev3/page/err/nrf52840/rev3/latest/anomaly_840_198.html)
and [219](https://docs.nordicsemi.com/r/bundle/errata_nrf52840_rev3/page/err/nrf52840/rev3/latest/anomaly_840_219.html).
SPIS additionally applies [214](https://docs.nordicsemi.com/r/bundle/errata_nrf52840_rev3/page/err/nrf52840/rev3/latest/anomaly_840_214.html)
(base+0xa4/base+0xac writes after disable); the timing correction in
[270](https://docs.nordicsemi.com/r/bundle/errata_nrf52840_rev3/page/err/nrf52840/rev3/latest/anomaly_840_270.html)
is already included in PS v1.11. TWIS rejects unreviewed/Engineering A silicon
pending anomaly-83 duplicate-STOPPED qualification. SPIM PPI/suspend modes with
unreviewed errata are not exposed. TWIM's 400 kHz request uses the documented
390 kHz workaround and reports the actual nominal rate. These exclusions are
explicit; actual silicon identity and the full unrelated MCU errata gate remain
PLN-0042 work.

## Verification and physical follow-up

Examples: [bounded UARTE stream](../../example/serial-stream/README.md),
[SPIS target](../../example/spis-target/README.md) and
[TWIS target](../../example/twis-target/README.md). Their builds never upload.
The [software evidence record](evidence/nrf52840-serial/2026-11-02-software.json)
records 74 passing CTest entries, 526 native cases (38 added), 259 public headers
on four toolchains, nine linked source/repository/package images and three
simulated-mount runner plans. Image input hashes, one reset handler per image
and absence of allocation/exception runtime symbols were checked. Focused trace
validation has zero diagnostics. Project validation has zero errors and 71
warnings, including three coverage warnings for the new examples.

Native models exercise instances, register ordering, queue bounds, partial
results, overrun, alias exclusion, device leases, failed task/event writes and
late acknowledgments. Negative bindings and body/header probes compile on the
host, Cortex-M4F, Cortex-M7 and ESP32-P4. These checks do not measure electrical
behavior or timing. Remaining physical exit checks require independent UART
TX/RX/CTS/RTS peers, SPI master/slave peers, a stretching-capable I²C master and
independent rate/timing measurements. No coverage or throughput measurement,
hosted CI run, silicon readback or peer result is inferred from these builds.
