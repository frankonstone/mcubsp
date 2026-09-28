#include <cstdint>
#include <utility>

#include <mcuhw/core/ownership.hpp>
#include <mcuhw/core/transfer.hpp>
#include <mcuhw/execution/bare_metal_policy.hpp>

namespace {

enum class probe_error : std::uint8_t { failed };

struct probe_hooks {
    using critical_token = std::uint32_t;

    static inline mcuhw::execution::tick ticks{};

    static critical_token enter_critical() noexcept { return 0U; }
    static void leave_critical(critical_token) noexcept {}
    static mcuhw::execution::tick now() noexcept { return ticks; }
    static bool in_isr() noexcept { return false; }
    static void poll() noexcept { ++ticks; }
    static void notify() noexcept {}
    static void active_transfer_destroyed() noexcept {}
};

using policy_type = mcuhw::execution::bare_metal_policy<probe_hooks>;
using pool_type = mcuhw::core::operation_pool<void, probe_error, 1, policy_type>;

} // namespace

int mcuhw_execution_probe() {
    policy_type policy;
    mcuhw::core::peripheral_ownership<1, policy_type> ownership{policy};
    auto lease = ownership.try_claim(0);
    if (!lease) {
        return 1;
    }

    pool_type pool{policy};
    auto started = pool.try_start({.quiesce = pool_type::already_quiescent});
    if (!started) {
        return 2;
    }
    auto operation = std::move(*started);
    if (!pool.complete(operation.id())) {
        return 3;
    }
    return operation.take() ? 0 : 4;
}
