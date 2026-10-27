#include <concepts>
#include <cstdint>
#include <expected>
#include <utility>

#include <mcubsp/backend/ra4m1/dmac.hpp>
#include <mcubsp/core/mmio.hpp>
#include <mcubsp/execution/fake_policy.hpp>

namespace {
struct example_mmio {
    template<std::unsigned_integral Value>
    [[nodiscard]] std::expected<Value, mcubsp::core::mmio_error>
    read(std::uintptr_t) noexcept { return Value{}; }
    template<std::unsigned_integral Value>
    [[nodiscard]] std::expected<void, mcubsp::core::mmio_error>
    write(std::uintptr_t, Value) noexcept { return {}; }
};
using backend_type = mcubsp::backend::ra4m1::dmac_channel_backend<
    0, example_mmio, mcubsp::execution::fake_policy>;
} // namespace

// @req REQ-0013 REQ-0018 REQ-0019 REQ-0030 REQ-0038
int mcubsp_dmac_example() {
    example_mmio mmio;
    mcubsp::execution::fake_policy execution;
    backend_type backend{mmio, execution};
    if (!backend.initialize()) return 1;
    auto started = backend.copy({
        .source = 0x2000'0000U,
        .destination = 0x2000'0100U,
        .count = 16U,
        .width = mcubsp::driver::dma_width::bits32,
    });
    if (!started) return 2;
    auto transfer = std::move(*started);
    static_cast<void>(transfer.cancel());
    static_cast<void>(transfer.take());
    return backend.shutdown() ? 0 : 3;
}
