# Arduino UNO R4 Minima W5100 Ethernet

The attached-device refactor in REQ-0046 adds a checked `io` composition and
changes polling to a configurable payload budget (64 bytes by default). The
physical results below describe the earlier recorded runs; their throughput
figures have not been remeasured for the refactored code. See the
[components guide](../../../../doc/getting-started/components.md) for the current API.

PLN-0024 uses the stacked Arduino Ethernet Shield R3 as an external raw-frame
device. It does not model the RA4M1 as having a native Ethernet MAC, and it does
not use the W5100 hardware TCP/UDP sockets, the Arduino Ethernet library, lwIP,
or dynamic storage.

## Reviewed source identities

- WIZnet `W5100 Datasheet`, version 1.2.8, 23 May 2019, 72 pages,
  1,424,167 bytes, SHA-256
  `a85784e25babf8fd7de152885c9faf46e438a47e26231806db4ab8745811dcac`.
  Official source:
  <https://docs.wiznet.io/img/products/w5100/W5100_DS_V128E.pdf>.
- Arduino `Shield_Eth01b.sch`, created 16 June 2008, one page, 61,391
  bytes, SHA-256
  `3176effca3c4b4eb52f2500e647259e5eda293ffe78519189bf3aa6e381dd563`.
  Official archive:
  <https://www.arduino.cc/en/uploads/Main/arduino-ethernet-shield-schematic.pdf>.
  This original-shield schematic establishes the W5100 SPI and active-low SS
  topology, but predates the R3 microSD circuit. The connected R3 shield and
  the earlier SPI/DMAC hardware procedures establish D10 as W5100 CS and D4 as
  the microSD CS that must remain high. The UNO binding resolves D10 to P112;
  D4 isolation is deliberately owned by the board composition, not the W5100
  protocol driver.

The implementation follows the datasheet's original-W5100 protocol: SPI mode
0, MSB first, separately framed 32-bit read/write streams (`0x0f` and `0xf0`),
socket 0 MACRAW mode, an 8 KiB socket-0 RX/TX allocation, two-byte received
length headers, and masked circular-memory addressing. Because this W5100 has
no version register, initialization identifies it using MR software-reset
completion, the `RMSR/TMSR = 0x55` reset values, and SHAR write/read-back.

## Build and native verification

```sh
cmake --preset host
cmake --build --preset host
ctest --preset host

cmake --preset uno-r4-minima
cmake --build --preset uno-r4-minima
```

The native W5100 model covers reset and identity, register commands, RX/TX
wrap, backpressure, stale SEND_OK state, malformed and oversized receive
lengths, transport and command timeouts, cancellation, recovery, and slot
reuse. A separate translation unit compiles the adapter against the actual
sibling `mcuip::NetworkDevice` concept. The instrumented Cortex-M4 composition
links the sibling MCUIP sources with exceptions and RTTI disabled.

## Physical procedure and current result

```sh
./tools/run-uno-r4-minima-tests.sh ethernet
```

The firmware uses MAC `02:00:00:04:51:00`, static address
`192.168.188.240/24`, and the independent gateway peer `192.168.188.1`. It
initiates ARP through `mcuip::EthernetIpv4Host`, then exchanges an 8-byte ICMP
echo and a 1,480-byte ICMP echo. It closes and reopens W5100 socket 0 and repeats
the small echo to prove traffic after clean recovery.

The accepted functional run on 22 September 2026 passed all three registered
cases. It observed ARP resolution, three ICMP echo replies, 60-byte and
1,514-byte raw frames, four completed transmissions, one successful recovery,
and zero receive or SPI transport errors. The instrumented image used 142,248
bytes of flash and 22,760 bytes of static RAM.

The final non-instrumented `-Os` comparison uses 64 maximum-sized ICMP packets
and four outstanding requests. Four replies require about 6.1 KiB, which stays
within socket 0's 8 KiB RX allocation; an eight-packet trial exceeded that
capacity and produced packet loss. DWT cycle timing includes MCUIP, W5100
register access, and peer round trips. All three accepted runs completed 64/64
requests without loss or SPI transport errors:

| SPI path | Clock | Payload goodput | RTT min / average / max | Flash | RAM |
| --- | ---: | ---: | ---: | ---: | ---: |
| Interrupt-driven | 6 MHz | 100,285 bit/s | 231.817 / 448.294 / 492.144 ms | 37,220 B | 21,112 B |
| W5100 bounded polling | 6 MHz | 159,763 bit/s | 147.049 / 281.000 / 303.230 ms | 35,840 B | 21,112 B |
| W5100 bounded polling | 12 MHz | 175,569 bit/s | 133.787 / 256.995 / 279.367 ms | 35,844 B | 21,112 B |

The fast path removes asynchronous-operation allocation, interrupt dispatch,
and wait overhead while preserving the W5100 requirement that every byte use a
separately chip-selected 32-bit command. At 6 MHz it improves goodput by 59.3%;
raising the clock to 12 MHz adds another 9.9%, for 75.1% over the original path.
The datasheet specifies a 70 ns minimum SCLK period (14.29 MHz maximum), so
12 MHz is the fastest exact RA4M1 divider below that limit. This is acknowledged
IPv4-payload goodput through the MCUIP ICMP path, not TCP or full-duplex
line-rate throughput. Run the comparison with:

```sh
./tools/run-uno-r4-minima-tests.sh ethernet-metrics
./tools/run-uno-r4-minima-tests.sh ethernet-metrics-fast6
./tools/run-uno-r4-minima-tests.sh ethernet-metrics-fast12
```

Physical link-loss recovery is exercised separately:

```sh
./tools/run-uno-r4-minima-tests.sh ethernet-recovery
```

During the accepted run, the Ethernet cable was manually disconnected for
approximately five seconds and then reconnected. The firmware recorded 20
missing peer replies, detected the loss, reopened the W5100 MACRAW socket, and
received ICMP replies again with zero SPI transport errors.

The evidence JSON and MCUCOV report are written below
`build/uno-r4-minima/artifacts/`.

These runs prove clean socket reinitialization and physical traffic restoration
after a manual cable disconnect/reconnect. PLN-0024 is complete. Independent
electrical SPI timing and Ethernet waveform quality are not claimed.
