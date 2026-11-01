# Pro Micro native UART0 loopback

Connect **P0.06 (TX, connector label 006)** to **P0.08 (RX, label 008)** on the
actual clone. Confirm those connector labels against its wiring. The application
uses 3.3 V GPIO levels. Its eight-byte pattern includes binary values, not just
text. The onboard LED changes state after each matching transmit/receive round,
with a nominal 250 ms pause. A failure after LED setup holds it on.

```sh
cmake --build --preset promicro-nrf52840 --target others.promicro_nrf52840.example.uart-loopback
python3 tool/hardware.py flash --board others/promicro_nrf52840 \
  --elf output/build/promicro-nrf52840/example/others.promicro_nrf52840.example.uart-loopback.elf \
  --mount /Volumes/NICENANO --dry-run
```

Use the actual UF2 mount and remove `--dry-run` for the intended upload. The
application uses legacy UART0, requests HFXO explicitly, and selects nominal
115200 baud with the product table's actual rate of 115942. Rate tolerance is
1%; physical oscillator/wire accuracy is not measured by a matching echo.
There is no USB application device, radio stack or watchdog reset cycle.

The example uses the native `uart_controller` and existing transfer handles.
It polls while waiting and bounds each round to one second in the default DWT
clock. `mcubsp_uart_status` records success/failure and `mcubsp_uart_rounds`
counts successful rounds in RAM. The original board serial facade remains
unavailable; UARTE and streaming receive are later work. No physical loopback
observation has been recorded for this image yet.

For standalone source/package consumption, configure this directory with the
same board toolchain as the system example. Its target is `uart_loopback`.
