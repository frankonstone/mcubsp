# W5100 Ethernet

For the application API, use the [attached-device guide](../getting-started/components.md)
and [linked IO example](../../examples/io/w5100.cpp). They create
`device::w5100` from an `hw.spi.device(...)` handle. The lower-level composition
below uses the device register protocol and a direct controller transport.

The Arduino Ethernet Shield R3 contains an original WIZnet W5100. `mcuhw`
drives it through the UNO R4 Minima SPI backend, opens socket 0 in MACRAW mode,
and presents fixed receive/transmit slots suitable for an `mcuip`
`NetworkDevice`. The driver provides raw Ethernet frames; it does not expose
the W5100 hardware TCP/UDP socket API or use the Arduino Ethernet library.

The `application_execution_policy` used below is defined in the
[bare-metal application integration guide](../concepts/application-integration.md).

## Hardware and network setup

Stack the shield on the UNO and connect Ethernet to the local network. The
important signals are D13/SCK, D12/MISO, D11/MOSI, D10/W5100 chip select, and
D4/SD-card chip select. Drive D4 high before using SPI so the uninitialized SD
card cannot contend for MISO.

Use a locally administered unicast MAC address and a static IPv4 address that
is unused on the subnet. The values below are examples; replace them for your
network. A duplicate IPv4 or MAC address will produce misleading packet loss.

## Composition

```cpp
#include <array>
#include <cstddef>
#include <span>

#include <mcuhw/backend/ra4m1/gpio.hpp>
#include <mcuhw/backend/ra4m1/spi.hpp>
#include <mcuhw/board/arduino_uno_r4_minima.hpp>
#include <mcuhw/core/direct_mmio.hpp>
#include <mcuhw/driver/gpio.hpp>
#include <mcuhw/device/ethernet/w5100_spi.hpp>
#include <mcuhw/integration/mcuip/w5100.hpp>

namespace board = mcuhw::board::arduino_uno_r4_minima;

using spi_binding = mcuhw::backend::ra4m1::spi_binding<
    1, board::d12_miso, board::d11_mosi, board::d13_sck, board::d10_ssl>;

constexpr auto spi_timing = mcuhw::backend::ra4m1::solve_spi_timing({
    .clock_hz = 48'000'000U,
    .target_hz = 6'000'000U,
    .maximum_error_ppm = 0U,
});
static_assert(spi_timing.has_value());

template<class ExecutionPolicy>
class ethernet_port {
    using gpio_type = mcuhw::backend::ra4m1::gpio_backend<
        mcuhw::core::direct_mmio>;
    using gpio_pin_type = mcuhw::driver::gpio_pin<gpio_type>;
    using spi_type = mcuhw::backend::ra4m1::spi_master_backend<
        spi_binding, mcuhw::core::direct_mmio, ExecutionPolicy>;
    using transport_type =
        mcuhw::device::w5100_spi_immediate_transport<spi_type>;
    using device_type = mcuhw::integration::mcuip::
        w5100_macraw_network_device<transport_type>;

public:
    explicit ethernet_port(ExecutionPolicy& execution)
        : spi_{mmio_, execution}, transport_{spi_}, device_{transport_},
          sd_cs_{gpio_, mcuhw::core::pin_id{1U, 3U}} {}

    bool initialize() {
        if (!sd_cs_.initialize({
                .direction = mcuhw::driver::gpio_direction::output,
                .initial_level = mcuhw::driver::gpio_level::high,
                .output_type = mcuhw::driver::gpio_output_type::push_pull,
                .pull = mcuhw::driver::gpio_pull::none,
                .drive = mcuhw::driver::gpio_drive::middle,
            })) {
            return false;
        }
        if (!spi_.initialize(*spi_timing, {
                .mode = mcuhw::driver::spi_mode::mode0,
                .bit_order =
                    mcuhw::driver::spi_bit_order::most_significant_first,
                .frame_width = mcuhw::driver::spi_frame_width::bits8,
            })) {
            return false;
        }
        if (!device_.initialize({
                .mac = {{0x02U, 0x00U, 0x00U, 0x04U, 0x51U, 0x00U}},
                .command_poll_limit = 256U,
                .stable_read_limit = 4U,
            })) {
            return false;
        }

        for (std::size_t slot = 0; slot < receive_buffers_.size(); ++slot) {
            if (!device_.arm_receive(slot, receive_buffers_[slot])) {
                return false;
            }
        }
        return true;
    }

    template<class FrameHandler>
    void service(FrameHandler&& process_ethernet_frame) {
        device_.poll();

        for (std::size_t slot = 0; slot < receive_buffers_.size(); ++slot) {
            if (const auto received = device_.take_receive(slot)) {
                if (!received->error) {
                    process_ethernet_frame(std::span{
                        receive_buffers_[slot].data(), received->size});
                }
                // A completed slot must be armed again before it can receive.
                static_cast<void>(
                    device_.arm_receive(slot, receive_buffers_[slot]));
            }
        }
    }

    bool transmit(std::size_t slot, std::span<const std::byte> frame) {
        return device_.start_transmit(slot, frame);
    }

    auto take_transmit_result(std::size_t slot) {
        return device_.take_transmit_result(slot);
    }

    const auto& metrics() const { return device_.metrics(); }

private:
    mcuhw::core::direct_mmio mmio_;
    gpio_type gpio_{mmio_};
    spi_type spi_;
    transport_type transport_;
    device_type device_;
    gpio_pin_type sd_cs_;
    std::array<std::array<std::byte, device_type::buffer_size>,
               device_type::rx_descriptor_count> receive_buffers_{};
};
```

Give the port application lifetime and call its bounded polling service from
the main loop:

```cpp
application_execution_policy execution;
ethernet_port<application_execution_policy> ethernet{execution};

int main() {
    if (!ethernet.initialize()) {
        return 1;
    }

    for (;;) {
        ethernet.service([](std::span<const std::byte> frame) {
            process_network_frame(frame);
        });
        run_other_application_work();
    }
}
```

The immediate W5100 transport shown here is polling rather than
interrupt-driven. Its execution-policy object must still outlive
`ethernet_port` because the contained SPI backend also exposes the generic
asynchronous transaction API.

`process_ethernet_frame` is a callback supplied by the application or network
stack.
Transmit frame storage must remain valid until `take_transmit_result()` reports
completion. Only one W5100 frame is physically transmitted at a time; fixed
slots provide bounded queueing without allocation.

The immediate transport uses bounded register polling and gives substantially
better throughput on the original W5100 than building one asynchronous object
for every byte. It still generates the separately chip-selected 32-bit command
required for each W5100 register byte.

Call `service()` frequently. If no receive slot is armed, frames remain in the
chip and `receive_backpressure` increases. On a transport or socket error,
inspect `last_error()` and metrics; `recover()` reopens MACRAW socket 0 without
reconstructing the object. Link state is inferred through traffic because the
original W5100 does not provide the same PHY status interface as newer parts.

## Add IPv4 with MCUIP

Create an `mcuip::EthernetIpv4Host`, feed every received frame to its
`process_frame()` method, and submit the returned reply frame through a free
transmit slot. Give the host the same MAC configured above and a valid static
configuration, for example `192.168.188.240/24` only if that address belongs to
your subnet and is unused. ARP resolution must complete before an IPv4 packet
can be sent to a new peer.

## Hardware checks

```sh
./tools/run-uno-r4-minima-tests.sh ethernet
./tools/run-uno-r4-minima-tests.sh ethernet-metrics-fast12
```

The first command checks initialization, ARP/ICMP exchange, full-size frames,
and socket recovery. The metrics command measures end-to-end ICMP payload
goodput and round-trip latency; it is not a raw SPI bandwidth measurement.

For cable-loss recovery and the network values used by the repository's
physical fixture, see the
[UNO W5100 hardware procedure](../boards/arduino-uno-r4-minima/ethernet-w5100.md).
