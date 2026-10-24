# ADC

This example samples a potentiometer on UNO A1 with the RA4M1 14-bit ADC.

The `application_execution_policy` and board-owned interrupt functions used
below are defined in the
[bare-metal application integration guide](../../../../../doc/concept/application-integration.md).

## Wiring

Connect the two outside potentiometer terminals to the board's analog
reference rails and the wiper to A1. Confirm that the wiper can never exceed
the MCU's allowed analog input range. A value near ground should approach zero;
a value near the reference should approach full scale.

## Code

The sampling helper receives the application-owned ADC controller. Its backend
must remain visible to the scan-end ISR until the transfer completes.

```cpp
#include <array>
#include <utility>

#include <mcubsp/backend/ra4m1/adc.hpp>
#include <mcubsp/board/arduino_uno_r4_minima.hpp>
#include <mcubsp/core/direct_mmio.hpp>
#include <mcubsp/driver/adc.hpp>

namespace board = mcubsp::board::arduino_uno_r4_minima;

void consume_adc_sample(std::uint16_t sample);

using binding = mcubsp::backend::ra4m1::adc14_binding<
    mcubsp::backend::ra4m1::adc_input<0, board::a1_an000>>;

template<class Adc>
int sample_a1(Adc& adc, mcubsp::execution::deadline deadline) {
    const std::array channels{mcubsp::driver::adc_channel{0U}};
    std::array<std::uint16_t, 1> samples{};
    auto started = adc.convert(channels, samples);
    if (!started) {
        return 2;
    }

    auto transfer = std::move(*started);
    const auto waited = transfer.wait_until(deadline);
    const auto completed = transfer.take();
    if (!waited || !completed || completed->samples != 1U) {
        return 3;
    }

    const std::uint16_t raw = samples[0];
    consume_adc_sample(raw);
    return 0;
}
```

The surrounding application supplies static storage and scan-end dispatch:

```cpp
mcubsp::core::direct_mmio mmio;
application_execution_policy execution;

using backend_type = mcubsp::backend::ra4m1::adc14_backend<
    binding, mcubsp::core::direct_mmio, application_execution_policy>;
backend_type adc_hardware{mmio, execution};
mcubsp::driver::adc_controller adc{adc_hardware};

int main() {
    initialize_monotonic_clock();
    configure_adc_scan_end_interrupt();

    if (!adc.initialize({
            .resolution = mcubsp::driver::adc_resolution::bits14,
            .sample_states = 13U,
        })) {
        return 1;
    }
    enable_adc_scan_end_interrupt();
    const auto deadline = mcubsp::execution::make_deadline(
        execution.now(), 480'000U);
    const bool sampled = deadline && sample_a1(adc, *deadline) == 0;
    disable_adc_scan_end_interrupt();
    if (!sampled) {
        static_cast<void>(adc.shutdown());
        return 2;
    }
    return adc.shutdown() ? 0 : 3;
}

extern "C" void ADC_SCAN_END_Handler() noexcept {
    static_cast<void>(adc_hardware.handle_scan_end_interrupt());
    clear_adc_scan_end_request();
}
```

The application must connect that symbolic handler to the RA4M1 ADC140
scan-end event and use a timeout appropriate for the configured clock.

The channel number is the ADC channel in the binding, not the Arduino header
number. The binding makes an unsupported pin/channel pairing a compile-time
error. The channel list and result storage must remain alive until `take()`.

Long source impedance and cable capacitance can prevent the sample capacitor
from settling. Increase `sample_states`, buffer the signal, or reduce the
source impedance if readings depend on the previous channel. Average multiple
samples for noise reduction, but do not describe the result as calibrated
voltage unless the reference and gain/offset errors were measured.

## Hardware check

```sh
./tool/run-uno-r4-minima-tests.sh adc
```

Turn the potentiometer through its range. The physical test checks movement
and useful range; it does not claim calibrated absolute accuracy.
