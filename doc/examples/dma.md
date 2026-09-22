# DMA

This example copies 16 32-bit words with RA4M1 DMAC channel 0. DMA keeps the
CPU out of the data path, but it does not transfer buffer ownership to the
hardware safely unless the application also obeys the lifetime and memory
ordering rules.

The `application_execution_policy` and board-owned interrupt functions used
below are defined in the
[bare-metal application integration guide](../concepts/application-integration.md).

## Code

The copy helper receives the application-owned DMA channel. Keeping the
channel at application scope lets the transfer-end ISR reach the same operation
state that `main()` is waiting on.

```cpp
#include <array>
#include <utility>

#include <mcuhw/backend/ra4m1/dmac.hpp>
#include <mcuhw/core/direct_mmio.hpp>
#include <mcuhw/driver/dma.hpp>

void fill_source(std::array<std::uint32_t, 16>& source);
void consume_copy(const std::array<std::uint32_t, 16>& destination);

template<class Dma>
int copy_words(Dma& dma, mcuhw::execution::deadline deadline) {
    alignas(4) std::array<std::uint32_t, 16> source{};
    alignas(4) std::array<std::uint32_t, 16> destination{};
    fill_source(source);

    auto started = dma.copy({
        .source = reinterpret_cast<std::uintptr_t>(source.data()),
        .destination = reinterpret_cast<std::uintptr_t>(destination.data()),
        .count = source.size(),
        .width = mcuhw::driver::dma_width::bits32,
    });
    if (!started) {
        return 2;
    }

    auto transfer = std::move(*started);
    const auto waited = transfer.wait_until(deadline);
    const auto completed = transfer.take();
    if (!waited || !completed
        || completed->units != source.size()) {
        return 3;
    }

    consume_copy(destination); // safe only after completion
    return 0;
}
```

The surrounding application owns the channel and dispatches its transfer-end
event:

```cpp
mcuhw::core::direct_mmio mmio;
application_execution_policy execution;

using dma_type = mcuhw::backend::ra4m1::dmac_channel_backend<
    0, mcuhw::core::direct_mmio, application_execution_policy>;
dma_type dma{mmio, execution};

int main() {
    initialize_monotonic_clock();
    configure_dmac0_transfer_end_interrupt();

    if (!dma.initialize()) {
        return 1;
    }
    enable_dmac0_transfer_end_interrupt();
    const auto deadline = mcuhw::execution::make_deadline(
        execution.now(), 480'000U);
    const bool copied = deadline && copy_words(dma, *deadline) == 0;
    disable_dmac0_transfer_end_interrupt();
    if (!copied) {
        static_cast<void>(dma.shutdown());
        return 2;
    }
    return dma.shutdown() ? 0 : 3;
}

extern "C" void DMAC0_END_Handler() noexcept {
    static_cast<void>(dma.handle_transfer_end_interrupt());
    clear_dmac0_interrupt_request();
}
```

The symbolic vector and routing functions must be replaced by the actual
board-support names and ICU event selection.

Both arrays must stay alive and must not be accessed in conflicting ways while
the transfer is active. The addresses must be aligned for the selected width,
the count must fit the channel's finite-transfer limit, and both regions must
be reachable by the DMA engine.

The target must dispatch the DMAC transfer-end interrupt to
`handle_transfer_end_interrupt()`. On cached MCUs, clean source cache lines
before DMA reads and invalidate destination lines after DMA writes. The RA4M1
UNO target has different cache concerns from a Cortex-M7 target, so keep this
policy in board support rather than assuming all memory is coherent.

For a peripheral transfer, completion means the DMA engine and peripheral are
both quiescent. In full-duplex SPI, do not release either buffer merely because
one of the paired DMA channels finished first.

## Hardware check

DMA memory-copy validation needs no external wiring:

```sh
./tools/run-uno-r4-minima-tests.sh dmac
```
