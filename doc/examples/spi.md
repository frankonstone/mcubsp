# SPI

This example configures the UNO R4 Minima SPI header as a 1 MHz, mode-0,
most-significant-bit-first controller and performs one full-duplex transaction.

The `application_execution_policy` and board-owned interrupt functions used
below are defined in the
[bare-metal application integration guide](../concepts/application-integration.md).

## Wiring

Connect D13/SCK, D12/MISO, D11/MOSI, D10/CS, and a common ground. Check the
device voltage, SPI mode, maximum clock, and chip-select polarity. MISO must be
released when a device is not selected if several devices share the bus.

## Code

The transaction helper receives the application-owned controller. The SPI
backend remains at application scope so its interrupt handlers and operation
pool remain valid for the complete transfer.

```cpp
#include <array>
#include <utility>

#include <mcuhw/backend/ra4m1/spi.hpp>
#include <mcuhw/board/arduino_uno_r4_minima.hpp>
#include <mcuhw/core/direct_mmio.hpp>
#include <mcuhw/driver/spi.hpp>

namespace board = mcuhw::board::arduino_uno_r4_minima;

void use_identity(const std::array<std::uint16_t, 3>& identity);

using binding = mcuhw::backend::ra4m1::spi_binding<
    1, board::d12_miso, board::d11_mosi, board::d13_sck, board::d10_ssl>;

constexpr auto timing = mcuhw::driver::solve_spi_timing({
    .clock_hz = 48'000'000U,
    .target_hz = 1'000'000U,
    .maximum_error_ppm = 0U,
});
static_assert(timing.has_value());

template<class Spi>
int read_id(Spi& spi, mcuhw::execution::deadline deadline) {
    const std::array<std::uint16_t, 1> command{0x9FU};
    std::array<std::uint16_t, 3> identity{};
    const std::array segments{
        mcuhw::driver::spi_segment{.transmit = command},
        mcuhw::driver::spi_segment{.receive = identity, .dummy = 0xFFU},
    };

    auto started = spi.transaction(segments);
    if (!started) {
        return 2;
    }
    auto transfer = std::move(*started);
    const auto waited = transfer.wait_until(deadline);
    const auto completed = transfer.take();
    if (!waited || !completed || completed->frames != 4U) {
        return 3;
    }

    use_identity(identity);
    return 0;
}
```

The surrounding application supplies initialization, the deadline, and IRQ
dispatch:

```cpp
mcuhw::core::direct_mmio mmio;
application_execution_policy execution;

using backend_type = mcuhw::backend::ra4m1::spi_master_backend<
    binding, mcuhw::core::direct_mmio, application_execution_policy>;
backend_type spi{mmio, execution};

int main() {
    initialize_monotonic_clock();
    configure_spi1_interrupt_routes();

    if (!spi.initialize(*timing, {
            .mode = mcuhw::driver::spi_mode::mode0,
            .bit_order = mcuhw::driver::spi_bit_order::most_significant_first,
            .frame_width = mcuhw::driver::spi_frame_width::bits8,
        })) {
        return 1;
    }
    enable_spi1_interrupts();
    const auto deadline = mcuhw::execution::make_deadline(
        execution.now(), 4'800'000U);
    const bool read = deadline && read_id(spi, *deadline) == 0;
    disable_spi1_interrupts();
    if (!read) {
        static_cast<void>(spi.shutdown());
        return 2;
    }
    return spi.shutdown() ? 0 : 3;
}

extern "C" void SPI_RXI_Handler() noexcept {
    static_cast<void>(spi.handle_receive_interrupt());
    clear_spi_rxi_request();
}

extern "C" void SPI_TXI_Handler() noexcept {
    static_cast<void>(spi.handle_transmit_empty_interrupt());
    clear_spi_txi_request();
}

extern "C" void SPI_IDLE_Handler() noexcept {
    static_cast<void>(spi.handle_idle_interrupt());
    clear_spi_idle_request();
}

extern "C" void SPI_ERI_Handler() noexcept {
    static_cast<void>(spi.handle_error_interrupt());
    clear_spi_eri_request();
}
```

Those handler names are explanatory. The vector table and ICU event selection
must use the same physical IRQ lines.

Chip select remains asserted across every segment in one transaction. This is
useful for the common “command, then response” protocol. Separate transactions
produce separate chip-select assertions.

The frame arrays use `uint16_t` because the same API supports 8- and 16-bit
frames. In 8-bit mode, keep every transmitted value at or below `0xFF`. Both
buffers and the segment array must outlive the transfer.

The original W5100 is unusual: every register byte uses its own separately
framed 32-bit SPI command. Use the W5100 transport from the Ethernet example
instead of expressing a W5100 register stream as one generic transaction.

## Hardware check

With the SPI fixture connected:

```sh
./tools/run-uno-r4-minima-tests.sh spi
```

All-zeros or all-ones responses usually indicate incorrect chip select,
unpowered hardware, a floating MISO line, or the wrong SPI mode.
