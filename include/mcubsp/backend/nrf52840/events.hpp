#pragma once
// @req-file REQ-0057
#include <mcubsp/backend/nrf52840/system_context.hpp>
#include <optional>
namespace mcubsp::backend::nrf52840 {
enum class edge : std::uint32_t { rising = 1, falling = 2, toggle = 3 };
enum class pin_sense : std::uint32_t { disabled = 0, high = 2, low = 3 };

template <unsigned Index, class Context>
    requires(Index < 8)
class gpiote_channel {
  public:
    explicit gpiote_channel(Context &c) noexcept : c_(c) {}
    gpiote_channel(const gpiote_channel &) = delete;
    gpiote_channel &operator=(const gpiote_channel &) = delete;
    [[nodiscard]] system_result initialize(core::pin_id pin, edge polarity, bool task = false,
                                           bool initial = false,
                                           driver::gpio_pull pull = driver::gpio_pull::none,
                                           bool fast_edges = true) noexcept {
        if (owned_)
            return std::unexpected(system_error::busy);
        auto pol = static_cast<std::uint32_t>(polarity);
        if (pol < 1 || pol > 3 || !mcu::nrf52840::gpio_supported(pin))
            return std::unexpected(system_error::invalid);
        auto r = c_.claim(resource);
        if (!r)
            return r;
        owned_ = true;
        auto p = c_.gpio.claim(pin, task ? driver::gpio_direction::output
                                         : driver::gpio_direction::input);
        if (!p) {
            (void)c_.release(resource);
            owned_ = false;
            return std::unexpected(system_error::busy);
        }
        pin_ = pin;
        pin_owned_ = true;
        auto cfg = driver::gpio_configuration{
            .direction = task ? driver::gpio_direction::output : driver::gpio_direction::input,
            .initial_level = initial ? driver::gpio_level::high : driver::gpio_level::low,
            .pull = pull};
        if (!c_.gpio.configure(pin, cfg) || !c_.write(base + 0x308, 1U << Index) ||
            !c_.clear_event(base + 0x100 + Index * 4) ||
            !c_.write(base + 0x510 + Index * 4, (task ? 3U : 1U) | (pin.index << 8) |
                                                    (pin.port << 13) | (pol << 16) |
                                                    (initial ? 1U << 20 : 0)))
            return fail();
        fast_input_ = !task && fast_edges;
        // Nordic anomaly 155: retain the high-frequency channel clock for fast edges.
        if (fast_input_ && !c_.write(base + 0x600 + Index * 4, 1))
            return fail();
        task_ = task;
        c_.publish(resource);
        return {};
    }
    [[nodiscard]] system_result interrupt(bool on) noexcept {
        if (!owned_ || fault_ || task_)
            return std::unexpected(system_error::invalid);
        return c_.write(base + (on ? 0x304 : 0x308), 1U << Index);
    }
    [[nodiscard]] system_value<bool> consume() noexcept {
        if (!owned_ || fault_ || task_)
            return std::unexpected(system_error::invalid);
        auto r = c_.read(base + 0x100 + Index * 4);
        if (!r)
            return std::unexpected(r.error());
        if (*r && !c_.clear_event(base + 0x100 + Index * 4))
            return std::unexpected(system_error::mmio);
        return *r != 0;
    }
    [[nodiscard]] system_endpoint event() const noexcept {
        return owned_ && !task_
                   ? c_.endpoint(resource, base + 0x100 + Index * 4, endpoint_kind::event)
                   : system_endpoint{};
    }
    [[nodiscard]] system_endpoint task() const noexcept {
        return owned_ && task_ ? c_.endpoint(resource, base + Index * 4, endpoint_kind::task)
                               : system_endpoint{};
    }
    [[nodiscard]] system_endpoint set_task() const noexcept {
        return owned_ && task_ ? c_.endpoint(resource, base + 0x30 + Index * 4, endpoint_kind::task)
                               : system_endpoint{};
    }
    [[nodiscard]] system_endpoint clear_task() const noexcept {
        return owned_ && task_ ? c_.endpoint(resource, base + 0x60 + Index * 4, endpoint_kind::task)
                               : system_endpoint{};
    }
    [[nodiscard]] system_result trigger() noexcept {
        if (!owned_ || fault_ || !task_)
            return std::unexpected(system_error::invalid);
        return c_.write(base + Index * 4, 1);
    }
    [[nodiscard]] system_result shutdown() noexcept {
        if (!owned_)
            return {};
        if (c_.referenced(resource))
            return std::unexpected(system_error::busy);
        if (!c_.write(base + 0x308, 1U << Index) || !c_.write(base + 0x510 + Index * 4, 0) ||
            !c_.wait_bits(base + 0x510 + Index * 4, 3, 0, 8) ||
            !c_.clear_event(base + 0x100 + Index * 4))
            return fail();
        if (fast_input_ && !c_.write(base + 0x600 + Index * 4, 0))
            return fail();
        fast_input_ = false;
        if (pin_owned_) {
            if (!c_.gpio.configure(pin_, {}))
                return fail();
            c_.gpio.release(pin_);
            pin_owned_ = false;
        }
        auto r = c_.release(resource);
        if (r) {
            owned_ = false;
            fault_ = false;
        }
        return r;
    }

  private:
    system_result fail() noexcept {
        fault_ = true;
        c_.invalidate(resource);
        return std::unexpected(system_error::mmio);
    }
    static constexpr std::uintptr_t base = 0x40006000;
    static constexpr unsigned resource = 16 + Index;
    Context &c_;
    core::pin_id pin_{};
    bool owned_{}, pin_owned_{}, task_{}, fault_{}, fast_input_{};
};

template <unsigned Index, class Context>
    requires(Index < 20)
class ppi_channel {
  public:
    explicit ppi_channel(Context &c) noexcept : c_(c) {}
    ppi_channel(const ppi_channel &) = delete;
    ppi_channel &operator=(const ppi_channel &) = delete;
    [[nodiscard]] system_result initialize(system_endpoint event, system_endpoint task,
                                           std::optional<system_endpoint> fork = {}) noexcept {
        if (owned_)
            return std::unexpected(system_error::busy);
        // Validate every endpoint before changing any hardware or reference count.
        if (!c_.valid(event, endpoint_kind::event) || !c_.valid(task, endpoint_kind::task) ||
            (fork && !c_.valid(*fork, endpoint_kind::task)))
            return std::unexpected(system_error::invalid);
        auto r = c_.claim(resource);
        if (!r)
            return r;
        owned_ = true;
        if (!c_.retain(event, endpoint_kind::event)) {
            (void)c_.release(resource);
            owned_ = false;
            return std::unexpected(system_error::invalid);
        }
        event_ = event;
        if (!c_.retain(task, endpoint_kind::task)) {
            c_.drop(event_);
            (void)c_.release(resource);
            owned_ = false;
            return std::unexpected(system_error::invalid);
        }
        task_ = task;
        c_.publish(resource);
        if (fork) {
            if (!c_.retain(*fork, endpoint_kind::task)) {
                c_.drop(event_);
                c_.drop(task_);
                (void)c_.release(resource);
                owned_ = false;
                return std::unexpected(system_error::invalid);
            }
            fork_ = fork;
        }
        if (!c_.write(base + 0x508, 1U << Index) ||
            !c_.write(base + 0x510 + Index * 8, static_cast<std::uint32_t>(event.address)) ||
            !c_.write(base + 0x514 + Index * 8, static_cast<std::uint32_t>(task.address)) ||
            !c_.write(base + 0x910 + Index * 4,
                      fork ? static_cast<std::uint32_t>(fork->address) : 0))
            return fail();
        c_.publish(resource);
        return {};
    }
    [[nodiscard]] system_result enable(bool on = true) noexcept {
        if (!owned_ || fault_)
            return std::unexpected(system_error::not_initialized);
        return c_.write(base + (on ? 0x504 : 0x508), 1U << Index);
    }
    [[nodiscard]] system_result shutdown() noexcept {
        if (!owned_)
            return {};
        if (c_.referenced(resource))
            return std::unexpected(system_error::busy);
        if (!c_.write(base + 0x508, 1U << Index) ||
            !c_.wait_bits(base + 0x500, 1U << Index, 0, 8) ||
            !c_.write(base + 0x510 + Index * 8, 0) || !c_.write(base + 0x514 + Index * 8, 0) ||
            !c_.write(base + 0x910 + Index * 4, 0))
            return fail();
        c_.drop(event_);
        c_.drop(task_);
        if (fork_)
            c_.drop(*fork_);
        auto r = c_.release(resource);
        if (r) {
            owned_ = false;
            fault_ = false;
            fork_.reset();
        }
        return r;
    }
    [[nodiscard]] system_endpoint enable_task() const noexcept {
        return {};
    } // groups, not channels, expose tasks
    [[nodiscard]] unsigned resource_id() const noexcept { return resource; }

  private:
    system_result fail() noexcept {
        fault_ = true;
        c_.invalidate(resource);
        return std::unexpected(system_error::mmio);
    }
    static constexpr std::uintptr_t base = 0x4001F000;
    static constexpr unsigned resource = 24 + Index;
    Context &c_;
    system_endpoint event_{}, task_{};
    std::optional<system_endpoint> fork_;
    bool owned_{}, fault_{};
};

template <unsigned Index, class Context>
    requires(Index >= 20 && Index < 32)
class fixed_ppi_channel {
  public:
    explicit fixed_ppi_channel(Context &c) noexcept : c_(c) {}
    fixed_ppi_channel(const fixed_ppi_channel &) = delete;
    fixed_ppi_channel &operator=(const fixed_ppi_channel &) = delete;
    [[nodiscard]] system_result initialize(system_endpoint event, system_endpoint task,
                                           std::optional<system_endpoint> fork = {}) noexcept {
        constexpr auto binding = mcu::nrf52840::fixed_ppi_bindings[Index - 20];
        if (owned_)
            return std::unexpected(system_error::busy);
        if (event.address != binding.event || task.address != binding.task ||
            event.domain != binding.event_owner || task.domain != binding.task_owner ||
            !c_.valid(event, endpoint_kind::event) || !c_.valid(task, endpoint_kind::task) ||
            (fork && !c_.valid(*fork, endpoint_kind::task)))
            return std::unexpected(system_error::invalid);
        auto r = c_.claim(resource);
        if (!r)
            return r;
        owned_ = true;
        if (!c_.retain(event, endpoint_kind::event)) {
            (void)c_.release(resource);
            owned_ = false;
            return std::unexpected(system_error::invalid);
        }
        event_ = event;
        if (!c_.retain(task, endpoint_kind::task)) {
            c_.drop(event_);
            (void)c_.release(resource);
            owned_ = false;
            return std::unexpected(system_error::invalid);
        }
        task_ = task;
        if (fork) {
            if (!c_.retain(*fork, endpoint_kind::task)) {
                c_.drop(event_);
                c_.drop(task_);
                (void)c_.release(resource);
                owned_ = false;
                return std::unexpected(system_error::invalid);
            }
            fork_ = fork;
        }
        // Never write EEP/TEP: these registers do not exist for channels 20..31.
        if (!c_.write(base + 0x508, 1U << Index) ||
            !c_.write(base + 0x910 + Index * 4,
                      fork ? static_cast<std::uint32_t>(fork->address) : 0))
            return fail();
        c_.publish(resource);
        return {};
    }
    [[nodiscard]] system_result enable(bool on = true) noexcept {
        if (!owned_ || fault_)
            return std::unexpected(system_error::not_initialized);
        auto r = c_.write(base + (on ? 0x504 : 0x508), 1U << Index);
        if (!r)
            return fail();
        return {};
    }
    [[nodiscard]] system_result shutdown() noexcept {
        if (!owned_)
            return {};
        if (c_.referenced(resource))
            return std::unexpected(system_error::busy);
        if (!c_.write(base + 0x508, 1U << Index) ||
            !c_.wait_bits(base + 0x500, 1U << Index, 0, 8) ||
            !c_.write(base + 0x910 + Index * 4, 0))
            return fail();
        c_.drop(event_);
        c_.drop(task_);
        if (fork_)
            c_.drop(*fork_);
        auto r = c_.release(resource);
        if (r) {
            owned_ = fault_ = false;
            fork_.reset();
        }
        return r;
    }

  private:
    system_result fail() noexcept {
        fault_ = true;
        c_.invalidate(resource);
        return std::unexpected(system_error::mmio);
    }
    static constexpr std::uintptr_t base = mcu::nrf52840::ppi_base;
    static constexpr unsigned resource = mcu::nrf52840::ppi_resource(Index);
    Context &c_;
    system_endpoint event_{}, task_{};
    std::optional<system_endpoint> fork_;
    bool owned_{}, fault_{};
};

template <unsigned Group, class Context>
    requires(Group < 6)
class ppi_group {
  public:
    explicit ppi_group(Context &c) noexcept : c_(c) {}
    ppi_group(const ppi_group &) = delete;
    ppi_group &operator=(const ppi_group &) = delete;
    // Only channels owned in this context can join; retaining them
    // prevents a group task from enabling a released or rebound channel.
    [[nodiscard]] system_result initialize(std::uint32_t mask) noexcept {
        if (owned_)
            return std::unexpected(system_error::invalid);
        auto r = c_.claim(resource);
        if (!r)
            return r;
        owned_ = true;
        for (unsigned n = 0; n < 32; ++n)
            if (mask & (1U << n)) {
                auto e =
                    c_.endpoint(mcu::nrf52840::ppi_resource(n),
                                mcu::nrf52840::ppi_resource_address(n), endpoint_kind::resource);
                if (!c_.retain(e, endpoint_kind::resource)) {
                    for (unsigned k = 0; k < n; ++k)
                        if (mask_ & (1U << k))
                            c_.drop(channels_[k]);
                    (void)c_.release(resource);
                    owned_ = false;
                    mask_ = 0;
                    return std::unexpected(system_error::invalid);
                }
                channels_[n] = e;
                mask_ |= 1U << n;
            }
        if (!c_.write(base + Group * 8 + 4, 1) || !c_.write(base + 0x800 + Group * 4, mask))
            return std::unexpected(system_error::mmio);
        c_.publish(resource);
        return {};
    }
    [[nodiscard]] system_endpoint enable_task() const noexcept {
        return owned_ ? c_.endpoint(resource, base + Group * 8, endpoint_kind::task)
                      : system_endpoint{};
    }
    [[nodiscard]] system_endpoint disable_task() const noexcept {
        return owned_ ? c_.endpoint(resource, base + Group * 8 + 4, endpoint_kind::task)
                      : system_endpoint{};
    }
    [[nodiscard]] system_result enable(bool on) noexcept {
        if (!owned_ || !c_.published(resource))
            return std::unexpected(system_error::not_initialized);
        return c_.write(base + Group * 8 + (on ? 0 : 4), 1);
    }
    [[nodiscard]] system_result set_members(std::uint32_t mask) noexcept {
        if (!owned_)
            return std::unexpected(system_error::invalid);
        // Static graph construction may include a route to this group's own
        // disable task. Reconfigure only while every PPI route is disabled.
        auto enabled = c_.read(base + 0x500);
        if (!enabled)
            return std::unexpected(enabled.error());
        if (*enabled)
            return std::unexpected(system_error::busy);
        for (unsigned n = 0; n < 32; ++n)
            if (mask & (1U << n)) {
                auto token =
                    c_.endpoint(mcu::nrf52840::ppi_resource(n),
                                mcu::nrf52840::ppi_resource_address(n), endpoint_kind::resource);
                if (!c_.valid(token, endpoint_kind::resource))
                    return std::unexpected(system_error::invalid);
            }
        auto cleared = clear_members();
        if (!cleared)
            return cleared;
        for (unsigned n = 0; n < 32; ++n)
            if (mask & (1U << n)) {
                auto token =
                    c_.endpoint(mcu::nrf52840::ppi_resource(n),
                                mcu::nrf52840::ppi_resource_address(n), endpoint_kind::resource);
                auto held = c_.retain(token, endpoint_kind::resource);
                if (!held)
                    return held;
                channels_[n] = token;
                mask_ |= 1U << n;
            }
        return c_.write(base + 0x800 + Group * 4, mask_);
    }
    [[nodiscard]] system_result clear_members() noexcept {
        if (!owned_)
            return std::unexpected(system_error::not_initialized);
        if (!c_.write(base + 0x800 + Group * 4, 0) || !c_.write(base + 0x508, mask_) ||
            !c_.wait_bits(base + 0x500, mask_, 0, 8))
            return std::unexpected(system_error::mmio);
        for (unsigned n = 0; n < 32; ++n)
            if (mask_ & (1U << n))
                c_.drop(channels_[n]);
        mask_ = 0;
        return {};
    }
    [[nodiscard]] system_result shutdown() noexcept {
        if (!owned_)
            return {};
        if (c_.referenced(resource))
            return std::unexpected(system_error::busy);
        if (!c_.write(base + Group * 8 + 4, 1) || !c_.wait_bits(base + 0x500, mask_, 0, 8) ||
            !c_.write(base + 0x800 + Group * 4, 0))
            return std::unexpected(system_error::mmio);
        for (unsigned n = 0; n < 32; ++n)
            if (mask_ & (1U << n))
                c_.drop(channels_[n]);
        auto r = c_.release(resource);
        if (r) {
            owned_ = false;
            mask_ = 0;
        }
        return r;
    }

  private:
    static constexpr std::uintptr_t base = 0x4001F000;
    static constexpr unsigned resource = 44 + Group;
    Context &c_;
    std::array<system_endpoint, 32> channels_{};
    std::uint32_t mask_{};
    bool owned_{};
};

template <unsigned Index, class Context>
    requires(Index < 6)
class event_generator {
  public:
    explicit event_generator(Context &c) noexcept : c_(c) {}
    event_generator(const event_generator &) = delete;
    event_generator &operator=(const event_generator &) = delete;
    [[nodiscard]] system_result initialize() noexcept {
        if (owned_)
            return std::unexpected(system_error::busy);
        auto r = c_.claim(resource);
        if (!r)
            return r;
        owned_ = true;
        if (!c_.write(base + 0x308, 0xFFFF))
            return std::unexpected(system_error::mmio);
        for (unsigned n = 0; n < 16; ++n)
            if (!c_.clear_event(base + 0x100 + n * 4))
                return std::unexpected(system_error::mmio);
        c_.publish(resource);
        return {};
    }
    [[nodiscard]] system_result trigger(unsigned n) noexcept {
        if (!owned_ || !c_.published(resource) || n >= 16)
            return std::unexpected(system_error::invalid);
        return c_.write(base + n * 4, 1);
    }
    [[nodiscard]] system_result interrupt(unsigned n, bool on) noexcept {
        if (!owned_ || !c_.published(resource) || n >= 16)
            return std::unexpected(system_error::invalid);
        return c_.write(base + (on ? 0x304 : 0x308), 1U << n);
    }
    [[nodiscard]] system_value<bool> consume(unsigned n) noexcept {
        if (!owned_ || !c_.published(resource) || n >= 16)
            return std::unexpected(system_error::invalid);
        auto r = c_.read(base + 0x100 + n * 4);
        if (!r)
            return std::unexpected(r.error());
        if (*r && !c_.clear_event(base + 0x100 + n * 4))
            return std::unexpected(system_error::mmio);
        return *r != 0;
    }
    [[nodiscard]] system_endpoint event(unsigned n) const noexcept {
        return owned_ && n < 16 ? c_.endpoint(resource, base + 0x100 + n * 4, endpoint_kind::event)
                                : system_endpoint{};
    }
    [[nodiscard]] system_endpoint task(unsigned n) const noexcept {
        return owned_ && n < 16 ? c_.endpoint(resource, base + n * 4, endpoint_kind::task)
                                : system_endpoint{};
    }
    [[nodiscard]] system_result shutdown() noexcept {
        if (!owned_)
            return {};
        if (c_.referenced(resource))
            return std::unexpected(system_error::busy);
        if (!c_.write(base + 0x308, 0xFFFF))
            return std::unexpected(system_error::mmio);
        for (unsigned n = 0; n < 16; ++n)
            if (!c_.clear_event(base + 0x100 + n * 4))
                return std::unexpected(system_error::mmio);
        auto r = c_.release(resource);
        if (r)
            owned_ = false;
        return r;
    }
    static constexpr unsigned irq = 20 + Index; // SWI and EGU are the same IRQ owner.
  private:
    static constexpr std::uintptr_t base = mcu::nrf52840::egu_bases[Index];
    static constexpr unsigned resource = 9 + Index;
    Context &c_;
    bool owned_{};
};

template <class Context> class gpio_port_events {
  public:
    explicit gpio_port_events(Context &c) noexcept : c_(c) {}
    gpio_port_events(const gpio_port_events &) = delete;
    gpio_port_events &operator=(const gpio_port_events &) = delete;
    [[nodiscard]] system_result initialize() noexcept {
        if (owned_)
            return std::unexpected(system_error::busy);
        auto r = c_.claim(15);
        if (!r)
            return r;
        owned_ = true;
        if (!c_.write(0x40006308, 1U << 31) || !c_.clear_event(0x4000617C))
            return std::unexpected(system_error::mmio);
        c_.publish(15);
        return {};
    }
    [[nodiscard]] system_endpoint event() const noexcept {
        return owned_ ? c_.endpoint(15, 0x4000617C, endpoint_kind::event) : system_endpoint{};
    }
    [[nodiscard]] system_result sense(core::pin_id pin, pin_sense sense,
                                      driver::gpio_pull pull = driver::gpio_pull::none) noexcept {
        if (!owned_ || !mcu::nrf52840::gpio_supported(pin))
            return std::unexpected(system_error::invalid);
        auto v = static_cast<std::uint32_t>(sense);
        if (v != 0 && v != 2 && v != 3)
            return std::unexpected(system_error::invalid);
        if (!detect_saved_[pin.port]) {
            auto mode = c_.read(0x50000524 + pin.port * 0x300);
            if (!mode)
                return std::unexpected(mode.error());
            detect_mode_[pin.port] = *mode;
            detect_saved_[pin.port] = true;
        }
        if (!c_.write(0x50000524 + pin.port * 0x300, 1))
            return std::unexpected(system_error::mmio);
        auto mask = 1U << pin.index;
        if (!(pins_[pin.port] & mask)) {
            if (!c_.gpio.claim(pin, driver::gpio_direction::input))
                return std::unexpected(system_error::busy);
            pins_[pin.port] |= mask;
        }
        if (!c_.gpio.configure(pin, {.pull = pull}) ||
            !c_.write(mcu::nrf52840::pin_configuration(pin),
                      (pull == driver::gpio_pull::up     ? 12U
                       : pull == driver::gpio_pull::down ? 4U
                                                         : 0U) |
                          (v << 16)))
            return std::unexpected(system_error::mmio);
        return {};
    }
    [[nodiscard]] system_result interrupt(bool on) noexcept {
        if (!owned_)
            return std::unexpected(system_error::not_initialized);
        return c_.write(0x40006000 + (on ? 0x304 : 0x308), 1U << 31);
    }
    [[nodiscard]] system_value<std::uint32_t> latch(unsigned port) noexcept {
        if (!owned_ || port > 1)
            return std::unexpected(system_error::invalid);
        return c_.read(0x50000520 + port * 0x300);
    }
    [[nodiscard]] system_result clear_latch(unsigned port, std::uint32_t bits) noexcept {
        if (!owned_ || port > 1 || (bits & ~pins_[port]))
            return std::unexpected(system_error::invalid);
        if (!c_.write(0x50000520 + port * 0x300, bits))
            return std::unexpected(system_error::mmio);
        auto r = c_.read(0x50000520 + port * 0x300);
        if (!r)
            return std::unexpected(r.error());
        return {}; // erratum 173: read after write
    }
    [[nodiscard]] system_result clear_port_event() noexcept {
        if (!owned_)
            return std::unexpected(system_error::not_initialized);
        return c_.clear_event(0x4000617C);
    }
    [[nodiscard]] system_result shutdown() noexcept {
        if (!owned_)
            return {};
        if (c_.referenced(15))
            return std::unexpected(system_error::busy);
        if (!c_.write(0x40006308, 1U << 31))
            return std::unexpected(system_error::mmio);
        for (unsigned p = 0; p < 2; ++p)
            for (unsigned n = 0; n < 32; ++n)
                if (pins_[p] & (1U << n)) {
                    core::pin_id pin{static_cast<std::uint16_t>(p), static_cast<std::uint16_t>(n)};
                    if (!c_.gpio.configure(pin, {}))
                        return std::unexpected(system_error::mmio);
                    c_.gpio.release(pin);
                    pins_[p] &= ~(1U << n);
                }
        for (unsigned p = 0; p < 2; ++p)
            if (detect_saved_[p]) {
                if (!c_.write(0x50000524 + p * 0x300, detect_mode_[p]))
                    return std::unexpected(system_error::mmio);
                detect_saved_[p] = false;
            }
        if (!c_.clear_event(0x4000617C))
            return std::unexpected(system_error::mmio);
        auto r = c_.release(15);
        if (r)
            owned_ = false;
        return r;
    }

  private:
    Context &c_;
    std::array<std::uint32_t, 2> pins_{}, detect_mode_{};
    std::array<bool, 2> detect_saved_{};
    bool owned_{};
};
} // namespace mcubsp::backend::nrf52840
