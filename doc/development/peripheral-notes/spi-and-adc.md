# SPI and ADC implementation notes for RA4M1

The portable SPI and ADC contracts use caller-owned spans and the same fixed,
move-only transfer state used by the other asynchronous drivers. They allocate
no memory and require the request descriptors and buffers to outlive the
returned transfer.

## SPI

`solve_spi_timing` searches SPBR and BRDV and selects the fastest rate that
does not exceed the target device clock. Configuration explicitly selects all
four modes, MSB/LSB order, 8/16-bit frames, and CS polarity.

One `transaction` contains one or more segments. A missing transmit span emits
the segment's dummy value; a missing receive span discards incoming frames.
The RA4M1 backend drives SSL0 as an owned GPIO so CS remains asserted across
every segment. Completion is published only from SPII after `IDLNF` reports
idle, never from transmit-buffer-empty. RX overrun, mode fault, parity fault,
MMIO failure, cancellation, and timeout all disable the peripheral interrupts
and deassert CS before caller storage can be reused.

On UNO R4 Minima, SPI1 is D10/SSL, D11/MOSI, D12/MISO, and D13/SCK. A physical
loopback fixture connects D11 to D12; observe D10 and D13 with a logic analyzer
to independently verify polarity, rate, and final-frame CS timing.

```sh
cmake --preset arm-cortex-m4 -B build/uno-r4-minima -DMCUHW_BOARD_TESTS=arduino/uno_r4_minima
cmake --build build/uno-r4-minima --target board-tests
tools/run-uno-r4-minima-tests.sh spi
```

## ADC

`adc14_binding` associates each enabled analog channel with its package pin at
compile time. Initialization selects 12- or 14-bit right-aligned raw results,
VREFH0, and a validated sampling-state count (at least six for the supported
RA4M1 clock ratios). There is no volts conversion hidden in the driver.

`convert` accepts a finite, duplicate-free channel list and equally-sized
result span. Hardware scans selected channels in ascending channel order; the
backend reads the channel result registers into the caller's requested order.
The scan-end interrupt is the completion point. Cancellation clears ADST and
drops all spans synchronously, so a late interrupt cannot access caller data.

On the UNO test fixture, P000/AN000 and P001/AN001 correspond to A1 and A2.
Known-level testing should connect one input to GND and the other to 3.3 V,
then verify both raw thresholds and reversed request ordering. Never exceed
the VREFH0/AVCC0 input range.

With A1 tied to GND and A2 tied to 3.3 V, run:

```sh
tools/run-uno-r4-minima-tests.sh adc
```

The runner writes separate RTT result and coverage artifacts for each image.
Cross-building an image is not physical evidence; artifacts are registered only
after the corresponding fixture runs successfully.

The standalone [SPI](../../examples/spi.md) and
[ADC](../../examples/adc.md) guides include their code, wiring, lifecycle, and
bring-up hints. Native tests cover the corresponding success and failure paths.
