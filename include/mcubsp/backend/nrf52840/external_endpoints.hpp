#pragma once
// @req-file REQ-0057
#include <mcubsp/backend/nrf52840/system_context.hpp>
namespace mcubsp::backend::nrf52840 {
// Explicit ownership adapter for a pre-existing BSP driver. This is not a radio
// or crypto driver. The supplied callback must stop that driver's hardware and
// make its borrowed state quiescent before returning success. New native
// drivers can use the same resource/domain identities directly.
template <endpoint_domain Domain, class Context>
    requires(Domain == endpoint_domain::radio || Domain == endpoint_domain::aar ||
             Domain == endpoint_domain::ccm)
class external_endpoint_owner {
  public:
    using quiesce = system_result (*)(void *) noexcept;
    external_endpoint_owner(Context &c, quiesce stop, void *state) noexcept
        : c_(c), stop_(stop), state_(state) {}
    external_endpoint_owner(const external_endpoint_owner &) = delete;
    external_endpoint_owner &operator=(const external_endpoint_owner &) = delete;
    [[nodiscard]] system_result initialize() noexcept {
        if (owned_ || !stop_)
            return std::unexpected(system_error::invalid);
        auto r = c_.claim(resource, Domain);
        if (!r)
            return r;
        owned_ = true;
        c_.publish(resource);
        return {};
    }
    [[nodiscard]] system_endpoint task(unsigned offset) const noexcept {
        return endpoint(offset, endpoint_kind::task);
    }
    [[nodiscard]] system_endpoint event(unsigned offset) const noexcept {
        return endpoint(offset, endpoint_kind::event);
    }
    [[nodiscard]] system_result shutdown() noexcept {
        if (!owned_)
            return {};
        if (c_.referenced(resource))
            return std::unexpected(system_error::busy);
        auto stopped = stop_(state_);
        if (!stopped)
            return stopped;
        auto r = c_.release(resource);
        if (r)
            owned_ = false;
        return r;
    }

  private:
    system_endpoint endpoint(unsigned offset, endpoint_kind kind) const noexcept {
        if (!owned_)
            return {};
        auto e = c_.endpoint(resource, base + offset, kind);
        return c_.valid(e, kind) ? e : system_endpoint{};
    }
    static constexpr unsigned resource = Domain == endpoint_domain::radio ? 76 : 77;
    static constexpr std::uintptr_t base =
        Domain == endpoint_domain::radio ? 0x40001000 : 0x4000F000;
    Context &c_;
    quiesce stop_;
    void *state_;
    bool owned_{};
};
} // namespace mcubsp::backend::nrf52840
