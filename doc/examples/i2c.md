# I2C

This example configures IIC1 on UNO A5/SCL and A4/SDA at 400 kHz and reads two
bytes from an addressed device register using a repeated START.

The `application_execution_policy` and board-owned interrupt functions used
below are defined in the
[bare-metal application integration guide](../concepts/application-integration.md).

## Wiring

Connect SCL to A5, SDA to A4, and ground to ground. I2C lines are open-drain
and need pull-up resistors to a voltage accepted by both devices. Many Grove
modules already include pull-ups; parallel modules can make the effective
resistance too low. Verify the sensor's address and voltage before power-up.

## Code

The register helper receives the application-owned bus. Keep the MMIO policy,
execution policy, and IIC backend at application scope so interrupt dispatch
can reach the same backend while this function waits.

```cpp
#include <utility>

#include <mcuhw/backend/ra4m1/iic.hpp>
#include <mcuhw/board/arduino_uno_r4_minima.hpp>
#include <mcuhw/core/direct_mmio.hpp>
#include <mcuhw/driver/i2c.hpp>

namespace board = mcuhw::board::arduino_uno_r4_minima;

void consume_sensor_value(std::uint8_t first, std::uint8_t second);

using binding = mcuhw::backend::ra4m1::iic_binding<
    1, board::a5_scl, board::a4_sda>;

constexpr auto timing = mcuhw::backend::ra4m1::solve_i2c_timing({
    .clock_hz = 24'000'000U,
    .target_hz = 400'000U,
    .maximum_error_ppm = 30'000U,
    .rise_time_ns = 120U,
    .fall_time_ns = 60U,
});
static_assert(timing.has_value());

template<class Bus>
int read_register(Bus& bus, mcuhw::execution::deadline deadline) {
    // Replace 0x50 with the unshifted seven-bit address from the datasheet.
    mcuhw::driver::i2c_device<Bus, 0x50U> device{bus};
    mcuhw::driver::i2c_register_request<2> request;
    auto started = device.read_register(0x20U, request, 2U);
    if (!started) {
        return 2;
    }

    auto transfer = std::move(*started);
    const auto waited = transfer.wait_until(deadline);
    const auto completed = transfer.take();
    if (!waited || !completed || completed->bytes_read != 2U) {
        return 3;
    }

    const auto first = request.data[0];
    const auto second = request.data[1];
    consume_sensor_value(first, second);
    return 0;
}
```

The surrounding application has this shape:

```cpp
mcuhw::core::direct_mmio mmio;
application_execution_policy execution;

using backend_type = mcuhw::backend::ra4m1::iic_controller_backend<
    binding, mcuhw::core::direct_mmio, application_execution_policy>;
backend_type i2c_hardware{mmio, execution};
mcuhw::driver::i2c_controller bus{i2c_hardware};

int main() {
    initialize_monotonic_clock();
    configure_iic1_interrupt_routes();

    if (!bus.initialize(*timing)) {
        return 1;
    }
    enable_iic1_interrupts();
    const auto deadline = mcuhw::execution::make_deadline(
        execution.now(), 4'800'000U);
    const bool read = deadline && read_register(bus, *deadline) == 0;
    disable_iic1_interrupts();
    if (!read) {
        static_cast<void>(bus.shutdown());
        return 2;
    }
    return bus.shutdown() ? 0 : 3;
}

extern "C" void IIC_TXI_Handler() noexcept {
    static_cast<void>(i2c_hardware.handle_transmit_empty_interrupt());
    clear_iic_txi_request();
}

extern "C" void IIC_TEI_Handler() noexcept {
    static_cast<void>(i2c_hardware.handle_transmit_end_interrupt());
    clear_iic_tei_request();
}

extern "C" void IIC_RXI_Handler() noexcept {
    static_cast<void>(i2c_hardware.handle_receive_interrupt());
    clear_iic_rxi_request();
}

extern "C" void IIC_ERI_Handler() noexcept {
    static_cast<void>(i2c_hardware.handle_error_interrupt());
    clear_iic_eri_request();
}
```

The symbolic handler and clear-function names stand for application board
support. On the RA4M1 they must match the IIC1 events selected in the ICU.

The address is the seven-bit value, not the eight-bit read/write byte printed
by some datasheets. For example, datasheet bytes `0xA0` and `0xA1` correspond
to address `0x50`.

`request` owns both the register command and receive storage, so it must
outlive the transfer. Inspect the underlying error cause to distinguish an
address NACK, data NACK, arbitration loss, timeout, or a stuck bus. A bus scan
is useful during bring-up, but normal firmware should address the expected
device explicitly.

## Hardware check

With the Grove gesture detector powered and connected:

```sh
./tools/run-uno-r4-minima-tests.sh i2c
```

If the device works at 100 kHz but not 400 kHz, measure rise time and check
pull-ups and cable capacitance before loosening the solver tolerance.
