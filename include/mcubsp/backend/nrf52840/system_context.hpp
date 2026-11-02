#pragma once
// @req-file REQ-0056 REQ-0057 REQ-0058 REQ-0062 REQ-0073
#include <array>
#include <cstdint>
#include <expected>
#include <limits>
#include <optional>
#include <mcubsp/arch/barrier.hpp>
#include <mcubsp/backend/nrf52840/gpio.hpp>
#include <mcubsp/mcu/nrf52840/system.hpp>

namespace mcubsp::backend::nrf52840 {
enum class system_error : std::uint8_t {
    invalid,
    busy,
    mmio,
    timeout,
    not_initialized,
    faulted,
    unsupported
};
using system_result = std::expected<void, system_error>;
template <class T> using system_value = std::expected<T, system_error>;
enum class endpoint_kind : std::uint8_t { event, task, resource };
using endpoint_domain = mcu::nrf52840::ppi_personality;
struct system_endpoint {
    std::uintptr_t address{};
    const void *context{};
    unsigned resource{};
    std::uint32_t generation{};
    endpoint_kind kind{};
    endpoint_domain domain{};
};

struct core_reservations {
    bool cycle_counter{true}; // default Pro Micro execution policy borrows DWT
    bool systick{}, mpu{}, vectors{};
#if defined(__ARM_PCS_VFP)
    bool fpu{true}; // hard-float startup/ABI owns CP10/11
#else
    bool fpu{};
#endif
};

// One explicit context per physical MCU; all clients borrow the same GPIO owner
// and execution policy. Lifecycle/configuration is foreground-only. IRQ
// callbacks may acknowledge events; they cannot destroy/reconfigure a
// foreground client.
template <class Mmio, class Execution> class system_context {
  public:
    static constexpr unsigned resource_count = 96;
    Mmio &mmio;
    Execution &execution;
    gpio_backend<Mmio> &gpio;
    resource_registry &resources;
    const core_reservations reserved;
    explicit system_context(Mmio &m, Execution &e, gpio_backend<Mmio> &g,
                            core_reservations owned_by_bsp = {}) noexcept
        : mmio(m), execution(e), gpio(g), resources(g.resources()), reserved(owned_by_bsp) {}
    system_context(const system_context &) = delete;
    system_context &operator=(const system_context &) = delete;
    [[nodiscard]] system_result claim(unsigned id,
                                      endpoint_domain domain = endpoint_domain::ordinary) noexcept {
        if (id >= resource_count || execution.in_isr() ||
            (id == 76   ? domain != endpoint_domain::radio
             : id == 77 ? (domain != endpoint_domain::aar && domain != endpoint_domain::ccm)
                        : domain != endpoint_domain::ordinary))
            return std::unexpected(system_error::invalid);
        return execution.critical([&]() -> system_result {
            if (owned_[id])
                return std::unexpected(system_error::busy);
            if (generation_[id] == std::numeric_limits<std::uint32_t>::max())
                return std::unexpected(system_error::faulted);
            if (auto p = coordinated_resource(id,domain)) {
                auto shared = resources.claim(execution,*p,this);
                if (!shared) return std::unexpected(shared.error()==coordination_error::busy
                                                    ? system_error::busy : system_error::faulted);
                coordinated_[id]=*shared;
            }
            domains_[id] = domain;
            owned_[id] = true;
            published_[id] = false;
            ++generation_[id];
            return {};
        });
    }
    [[nodiscard]] std::optional<resource_token> coordinated_owner(unsigned id) const noexcept {
        return id<resource_count && owned_[id] ? coordinated_[id] : std::nullopt;
    }
    // Native RADIO/CCM/AAR bind a live DMA-storage predicate. Cached task
    // tokens stop being routable when their final RAM lease is returned.
    void bind_dma_endpoint_state(unsigned id, const bool *armed) noexcept {
        if (id == 76 || id == 77) dma_endpoint_state_[id - 76] = armed;
    }
    [[nodiscard]] bool referenced(unsigned id) noexcept {
        return execution.critical([&] { return id >= resource_count || references_[id] != 0; });
    }
    [[nodiscard]] system_result release(unsigned id) noexcept {
        return execution.critical([&]() -> system_result {
            if (id >= resource_count || !owned_[id])
                return std::unexpected(system_error::not_initialized);
            if (references_[id])
                return std::unexpected(system_error::busy);
            if (coordinated_[id]) {
                auto shared=resources.release(execution,*coordinated_[id]);
                if (!shared) return std::unexpected(system_error::busy);
                coordinated_[id].reset();
            }
            owned_[id] = false;
            published_[id] = false;
            if (id == 76 || id == 77) dma_endpoint_state_[id-76] = nullptr;
            return {};
        });
    }
    [[nodiscard]] system_endpoint endpoint(unsigned id, std::uintptr_t address,
                                           endpoint_kind kind) const noexcept {
        return {address, this,
                id,      id < resource_count ? generation_[id] : 0,
                kind,    id < resource_count ? domains_[id] : endpoint_domain::ordinary};
    }
    [[nodiscard]] bool published(unsigned id) const noexcept {
        return id < resource_count && published_[id];
    }
    [[nodiscard]] bool system_off_blocked() noexcept {
        return execution.critical([&] {
            if (active_work_)
                return true;
            for (unsigned id = 0; id < resource_count; ++id)
                if (id != 15 && (id < 60 || id > 63) && owned_[id])
                    return true;
            return false;
        });
    }
    void publish(unsigned id) noexcept {
        if (id < resource_count && owned_[id])
            published_[id] = true;
    }
    void invalidate(unsigned id) noexcept {
        if (id < resource_count)
            published_[id] = false;
    }
    [[nodiscard]] bool valid(system_endpoint e, endpoint_kind kind) const noexcept {
        if (e.context != this || e.resource >= resource_count || !owned_[e.resource] ||
            !published_[e.resource] || e.generation != generation_[e.resource] ||
            e.domain != domains_[e.resource] || e.kind != kind || e.address % 4)
            return false;
        auto id = e.resource;
        if (kind == endpoint_kind::task && (id == 76 || id == 77) &&
            dma_endpoint_state_[id-76] && !*dma_endpoint_state_[id-76]) return false;
        auto a = e.address;
        auto indexed = [](std::uintptr_t a, std::uintptr_t start, unsigned count) {
            return a >= start && a < start + 4 * count && (a - start) % 4 == 0;
        };
        if (id < 5) {
            auto b = mcu::nrf52840::timer_bases[id];
            auto n = mcu::nrf52840::timer_channels[id];
            return kind == endpoint_kind::event
                       ? indexed(a, b + 0x140, n)
                       : kind == endpoint_kind::task && (a == b || a == b + 4 || a == b + 8 ||
                                                         a == b + 12 || indexed(a, b + 0x40, n));
        }
        if (id < 8) {
            auto b = mcu::nrf52840::rtc_bases[id - 5];
            return kind == endpoint_kind::event
                       ? (a == b + 0x100 || a == b + 0x104 || indexed(a, b + 0x140, 4))
                       : kind == endpoint_kind::task &&
                             (a == b || a == b + 4 || a == b + 8 || a == b + 12);
        }
        if (id == 8)
            return kind == endpoint_kind::event && a == 0x40010100;
        if (id >= 9 && id < 15) {
            auto b = mcu::nrf52840::egu_bases[id - 9];
            return indexed(a, b + (kind == endpoint_kind::event ? 0x100 : 0), 16) &&
                   kind != endpoint_kind::resource;
        }
        if (id == 15)
            return kind == endpoint_kind::event && a == 0x4000617C;
        if (id >= 16 && id < 24) {
            auto n = id - 16;
            return kind == endpoint_kind::event
                       ? a == 0x40006100 + n * 4
                       : kind == endpoint_kind::task &&
                             (a == 0x40006000 + n * 4 || a == 0x40006030 + n * 4 ||
                              a == 0x40006060 + n * 4);
        }
        if (id >= 24 && id < 44)
            return kind == endpoint_kind::resource && a == 0x4001F510 + (id - 24) * 8;
        if (id >= 44 && id < 50)
            return kind == endpoint_kind::task &&
                   (a == 0x4001F000 + (id - 44) * 8 || a == 0x4001F004 + (id - 44) * 8);
        if (id >= 64 && id < 76)
            return kind == endpoint_kind::resource &&
                   a == mcu::nrf52840::ppi_resource_address(20 + id - 64);
        if (id == 76) {
            if (kind == endpoint_kind::task)
                return indexed(a, 0x40001000, 13);
            if (kind == endpoint_kind::event) {
                constexpr std::array<unsigned, 23> offsets{0x100, 0x104, 0x108, 0x10C, 0x110, 0x114,
                                                           0x118, 0x11C, 0x128, 0x130, 0x134, 0x138,
                                                           0x13C, 0x140, 0x144, 0x148, 0x14C, 0x150,
                                                           0x154, 0x158, 0x15C, 0x168, 0x16C};
                for (auto offset : offsets)
                    if (a == 0x40001000 + offset)
                        return true;
            }
        }
        if (id == 77) {
            if (kind == endpoint_kind::event)
                return indexed(a, 0x4000F100, 3);
            if (kind == endpoint_kind::task)
                return (a == 0x4000F000 || a == 0x4000F008 ||
                        (e.domain == endpoint_domain::ccm && (a == 0x4000F004 || a == 0x4000F00C)));
        }
        if (id == 78) {
            return kind == endpoint_kind::event ? indexed(a, 0x40012100, 5)
                 : kind == endpoint_kind::task && indexed(a, 0x40012000, 5);
        }
        if (id == 61 || id == 62) {
            if (kind != endpoint_kind::event || !indexed(a, 0x40000100, 12))
                return false;
            auto n = static_cast<unsigned>((a - 0x40000100) / 4);
            return ((id == 61 ? 0xC1BU : 0x3E4U) >> n) & 1U;
        }
        return false;
    }
    [[nodiscard]] system_result retain(system_endpoint e, endpoint_kind kind) noexcept {
        return execution.critical([&]() -> system_result {
            if (!valid(e, kind) || references_[e.resource] == 255)
                return std::unexpected(system_error::invalid);
            ++references_[e.resource];
            return {};
        });
    }
    void drop(system_endpoint e) noexcept {
        execution.critical([&] {
            if (e.context == this && e.resource < resource_count && owned_[e.resource] &&
                e.generation == generation_[e.resource] && references_[e.resource])
                --references_[e.resource];
        });
    }
    [[nodiscard]] system_value<std::uint32_t> read(std::uintptr_t a) noexcept {
        auto r = mmio.template read<std::uint32_t>(a);
        if (!r)
            return std::unexpected(system_error::mmio);
        return *r;
    }
    [[nodiscard]] system_result write(std::uintptr_t a, std::uint32_t v) noexcept {
        if (!mmio.template write<std::uint32_t>(a, v))
            return std::unexpected(system_error::mmio);
        return {};
    }
    [[nodiscard]] system_result clear_event(std::uintptr_t a) noexcept {
        if (!write(a, 0))
            return std::unexpected(system_error::mmio);
        auto r = read(a);
        if (!r)
            return std::unexpected(r.error());
        arch::data_synchronization_barrier();
        return {};
    }
    [[nodiscard]] system_result wait_bits(std::uintptr_t a, std::uint32_t mask, std::uint32_t value,
                                          unsigned budget) noexcept {
        for (unsigned i = 0; i < budget; ++i) {
            auto r = read(a);
            if (!r)
                return std::unexpected(r.error());
            if ((*r & mask) == value)
                return {};
        }
        return std::unexpected(system_error::timeout);
    }
    [[nodiscard]] system_value<mcu::nrf52840::silicon_identity> identity() noexcept {
        auto p = read(0x10000100), v = read(0x10000104), k = read(0x10000108), r = read(0x1000010C),
             f = read(0x10000110);
        if (!p || !v || !k || !r || !f)
            return std::unexpected(system_error::mmio);
        if (*p != 0x52840)
            return mcu::nrf52840::silicon_identity{*p, *v, *k, *r, *f};
        auto rom_part = read(0xF0000FE0), rom_revision = read(0xF0000FE8);
        if (!rom_part || !rom_revision)
            return std::unexpected(system_error::mmio);
        return mcu::nrf52840::silicon_identity{
            *p, *v, *k, *r, *f, *rom_part & 255U, (*rom_revision >> 4) & 15U};
    }
    void work_started() noexcept {
        execution.critical([&] { ++active_work_; });
    }
    void work_stopped() noexcept {
        execution.critical([&] {
            if (active_work_)
                --active_work_;
        });
    }
    [[nodiscard]] bool active_work() noexcept {
        return execution.critical([&] { return active_work_ != 0; });
    }

  private:
    static constexpr std::optional<peripheral> coordinated_resource(unsigned id, endpoint_domain domain) noexcept {
        if (id<5) return static_cast<peripheral>(static_cast<unsigned>(peripheral::timer0)+id);
        if (id<8) return static_cast<peripheral>(static_cast<unsigned>(peripheral::rtc0)+id-5);
        if (id==8) return peripheral::wdt;
        if (id<15) return static_cast<peripheral>(static_cast<unsigned>(peripheral::egu0)+id-9);
        if (id==61) return peripheral::clock;
        if (id==62) return peripheral::power;
        if (id==76) return peripheral::radio;
        if (id==77) return domain==endpoint_domain::aar ? peripheral::aar : peripheral::ccm;
        if (id==78) return peripheral::qdec;
        return std::nullopt; // channel/core reservations retain their existing finer-grained identities
    }
    std::array<bool, resource_count> owned_{}, published_{};
    std::array<std::uint8_t, resource_count> references_{};
    std::array<std::uint32_t, resource_count> generation_{};
    std::array<endpoint_domain, resource_count> domains_{};
    std::array<std::optional<resource_token>, resource_count> coordinated_{};
    std::array<const bool *,2> dma_endpoint_state_{};
    unsigned active_work_{};
};
} // namespace mcubsp::backend::nrf52840
