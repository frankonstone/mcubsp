#pragma once
// @req-file REQ-0014 REQ-0062
#include <array>
#include <expected>
#include <limits>
#include <span>
#include <mcubsp/mcu/nrf52840/resources.hpp>
#include <mcubsp/mcu/nrf52840/peripherals.hpp>

namespace mcubsp::backend::nrf52840 {
enum class coordination_error : std::uint8_t {
    invalid, busy, exhausted, stale, active, faulted, count, not_quiescent
};
using coordination_result = std::expected<void, coordination_error>;
using peripheral = mcu::nrf52840::peripheral;
struct resource_token {
    const void *registry{}, *owner{};
    peripheral personality{};
    std::uint32_t generation{};
    friend constexpr bool operator==(resource_token, resource_token) = default;
};

// One explicit registry and one compatible critical-section policy per MCU.
// Tokens/retains do not disable hardware, remove PPI routes or restore pins.
// Drivers must finish that bounded sequencing before releasing their claim.
class resource_registry {
  public:
    resource_registry() = default;
    resource_registry(const resource_registry &) = delete;
    resource_registry &operator=(const resource_registry &) = delete;
    template<class Execution>
    [[nodiscard]] std::expected<resource_token, coordination_error>
    claim(Execution &e, peripheral p, const void *owner,
          std::span<const core::pin_id> pins = {}) noexcept {
        if (!owner || !mcu::nrf52840::valid(p) || e.in_isr())
            return std::unexpected(coordination_error::invalid);
        return e.critical([&]() -> std::expected<resource_token, coordination_error> {
            auto index = identity(p);
            auto &slot = slots_[index];
            if (slot.owner) return std::unexpected(coordination_error::busy);
            if (slot.generation == std::numeric_limits<std::uint32_t>::max())
                return std::unexpected(coordination_error::exhausted);
            for (std::size_t i = 0; i < pins.size(); ++i) {
                if (!mcu::nrf52840::gpio_supported(pins[i]))
                    return std::unexpected(coordination_error::invalid);
                if (pins_[pin_index(pins[i])].owner) return std::unexpected(coordination_error::busy);
                for (std::size_t j = 0; j < i; ++j)
                    if (pins[i] == pins[j]) return std::unexpected(coordination_error::invalid);
            }
            slot.owner = owner;
            slot.personality = p;
            ++slot.generation;
            for (auto pin : pins) pins_[pin_index(pin)] = {owner,index};
            return resource_token{this, owner, p, slot.generation};
        });
    }
    [[nodiscard]] bool valid(resource_token t) const noexcept {
        if (t.registry != this || !t.owner || !mcu::nrf52840::valid(t.personality)) return false;
        const auto &s = slots_[identity(t.personality)];
        return s.owner == t.owner && s.personality == t.personality && s.generation == t.generation;
    }
    template<class Execution> [[nodiscard]] bool claimed(Execution &e, peripheral p) const noexcept {
        return e.critical([&] { return mcu::nrf52840::valid(p) && slots_[identity(p)].owner != nullptr; });
    }
    template<class Execution> [[nodiscard]] coordination_result release(Execution &e, resource_token t) noexcept {
        if (e.in_isr()) return std::unexpected(coordination_error::invalid);
        return e.critical([&]() -> coordination_result {
            if (!valid(t)) return std::unexpected(coordination_error::stale);
            auto index = identity(t.personality);
            auto &s = slots_[index];
            if (s.references) return std::unexpected(coordination_error::active);
            for (auto &pin : pins_) if (pin.owner == t.owner && pin.resource == index) pin = {};
            s.owner = nullptr;
            return {};
        });
    }
    // Retains are suitable for DMA/PPI clients in foreground or ISR. Published
    // token storage and callbacks must outlive every retain.
    template<class Execution> [[nodiscard]] coordination_result retain(Execution &e, resource_token t) noexcept {
        return e.critical([&]() -> coordination_result {
            if (!valid(t)) return std::unexpected(coordination_error::stale);
            auto &s = slots_[identity(t.personality)];
            if (s.references == std::numeric_limits<std::uint16_t>::max())
                return std::unexpected(coordination_error::exhausted);
            ++s.references;
            return {};
        });
    }
    template<class Execution> [[nodiscard]] coordination_result drop(Execution &e, resource_token t) noexcept {
        return e.critical([&]() -> coordination_result {
            if (!valid(t)) return std::unexpected(coordination_error::stale);
            auto &s = slots_[identity(t.personality)];
            if (!s.references) return std::unexpected(coordination_error::invalid);
            --s.references;
            return {};
        });
    }
    // GPIO configuration/claim/release is foreground-only. ISR handoffs retain
    // existing claims and never mutate this table. GPIO has no execution policy
    // of its own; applications still supply one foreground owner per MCU.
    [[nodiscard]] bool claim_gpio(core::pin_id pin, const void *owner) noexcept {
        if (!owner || !mcu::nrf52840::gpio_supported(pin)) return false;
        auto &slot=pins_[pin_index(pin)];
        if (slot.owner) return false;
        slot={owner,mcu::nrf52840::peripheral_count};return true;
    }
    void release_gpio(core::pin_id pin, const void *owner) noexcept {
        if (!mcu::nrf52840::valid_pin(pin)) return;
        auto &slot=pins_[pin_index(pin)];
        if (slot.owner==owner && slot.resource==mcu::nrf52840::peripheral_count) slot={};
    }
  private:
    template<class, std::size_t> friend class dma_buffer_pool;
    static constexpr unsigned identity(peripheral p) noexcept {
        return static_cast<unsigned>(mcu::nrf52840::ownership_identity(p));
    }
    static constexpr unsigned pin_index(core::pin_id p) noexcept { return p.port * 32U + p.index; }
    struct slot { const void *owner{}; peripheral personality{}; std::uint32_t generation{}; std::uint16_t references{}; };
    std::array<slot, mcu::nrf52840::peripheral_count> slots_{};
    struct pin_slot { const void *owner{}; unsigned resource{}; };
    std::array<pin_slot, 64> pins_{};
    const void *buffer_pool_{}; // one shared overlap domain per physical MCU
};
// Compatibility storage for constructors without an explicit registry. A
// physical application shares this with its new native peripheral clients.
inline constinit resource_registry default_resources;
} // namespace mcubsp::backend::nrf52840
