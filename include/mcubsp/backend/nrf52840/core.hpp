#pragma once
// @req-file REQ-0073 REQ-0057
#include <array>
#include <bit>
#include <cstdint>
#include <mcubsp/backend/nrf52840/system_context.hpp>

extern "C" {
using mcubsp_promicro_irq_callback = void (*)(void *, unsigned) noexcept;
inline mcubsp_promicro_irq_callback mcubsp_promicro_irq_callback_target{};
inline void *mcubsp_promicro_irq_callback_context{};
}
namespace mcubsp::backend::nrf52840 {
struct fault_status {
    std::uint32_t cfsr{}, hfsr{}, dfsr{}, mmfar{}, bfar{}, afsr{};
};
enum class mpu_access : std::uint32_t {
    none = 0,
    privileged_rw = 1,
    privileged_rw_user_ro = 2,
    read_write = 3,
    privileged_ro = 5,
    read_only = 6
};
template <class Context> class core_controller {
  public:
    explicit core_controller(Context &c) noexcept : c_(c) {}
    core_controller(const core_controller &) = delete;
    core_controller &operator=(const core_controller &) = delete;
    [[nodiscard]] system_result initialize() noexcept {
        if (owned_)
            return std::unexpected(system_error::busy);
        auto r = c_.claim(60);
        if (r)
            owned_ = true;
        return r;
    }
    [[nodiscard]] system_result vectors(std::uintptr_t base) noexcept {
        if (!owned_ || c_.reserved.vectors || base > 0xFFFFFFFFULL || base % 256)
            return std::unexpected(system_error::invalid);
        if (!c_.write(0xE000ED08, static_cast<std::uint32_t>(base)))
            return std::unexpected(system_error::mmio);
        arch::data_synchronization_barrier();
        arch::instruction_barrier();
        return {};
    }
    [[nodiscard]] system_result systick(std::uint32_t reload, bool cpu_clock = true,
                                        bool interrupt = true) noexcept {
        if (!owned_ || c_.reserved.systick || reload == 0 || reload > 0xFFFFFF)
            return std::unexpected(system_error::invalid);
        if (!owns_systick_) {
            auto status = c_.read(0xE000E010);
            if (!status)
                return std::unexpected(status.error());
            if (*status & 1)
                return std::unexpected(system_error::busy);
        }
        owns_systick_ = true;
        if (!c_.write(0xE000E010, 0) || !c_.write(0xE000E014, reload) || !c_.write(0xE000E018, 0) ||
            !c_.write(0xE000E010, 1U | (interrupt ? 2U : 0U) | (cpu_clock ? 4U : 0U)))
            return std::unexpected(system_error::mmio);
        return {};
    }
    [[nodiscard]] system_result stop_systick() noexcept {
        if (!owned_)
            return std::unexpected(system_error::not_initialized);
        if (!owns_systick_)
            return {};
        if (!c_.write(0xE000E010, 0) || !c_.write(0xE000ED04, 1U << 25))
            return std::unexpected(system_error::mmio);
        arch::data_synchronization_barrier();
        owns_systick_ = false;
        return {};
    }
    [[nodiscard]] system_result fpu(bool enabled, bool lazy = true) noexcept {
        if (!owned_)
            return std::unexpected(system_error::not_initialized);
        if (!enabled && c_.reserved.fpu)
            return std::unexpected(system_error::busy);
        auto r = c_.read(0xE000ED88), fp = c_.read(0xE000EF34);
        if (!r || !fp)
            return std::unexpected(system_error::mmio);
        if (!enabled && (*fp & 1))
            return std::unexpected(system_error::busy);
        if (!c_.write(0xE000ED88, (*r & ~0xF00000U) | (enabled ? 0xF00000U : 0U)) ||
            !c_.write(0xE000EF34,
                      (*fp & ~0xC0000000U) | (enabled ? (lazy ? 0xC0000000U : 0x80000000U) : 0U)))
            return std::unexpected(system_error::mmio);
        arch::data_synchronization_barrier();
        arch::instruction_barrier();
        return {};
    }
    [[nodiscard]] system_value<fault_status> faults() noexcept {
        fault_status f{};
        auto a = c_.read(0xE000ED28), b = c_.read(0xE000ED2C), d = c_.read(0xE000ED30),
             m = c_.read(0xE000ED34), z = c_.read(0xE000ED38), x = c_.read(0xE000ED3C);
        if (!a || !b || !d || !m || !z || !x)
            return std::unexpected(system_error::mmio);
        f = {*a, *b, *d, *m, *z, *x};
        return f;
    }
    // TEX/S/C/B are explicit native memory attributes (six bits). Executability
    // and access permissions are validated separately; callers own memory policy.
    [[nodiscard]] system_result mpu_region(unsigned region, std::uintptr_t base,
                                           std::uint32_t bytes, mpu_access access, bool executable,
                                           unsigned tex_s_c_b = 0) noexcept {
        auto ap = static_cast<unsigned>(access);
        if (!owned_ || c_.reserved.mpu || region >= 8 || bytes < 32 ||
            !std::has_single_bit(bytes) || base > 0xFFFFFFFFULL || base % bytes || tex_s_c_b > 63 ||
            ap == 4 || ap > 6)
            return std::unexpected(system_error::invalid);
        auto type = c_.read(0xE000ED90), ctrl = c_.read(0xE000ED94);
        if (!type || !ctrl)
            return std::unexpected(system_error::mmio);
        if (region >= ((*type >> 8) & 255) || (*ctrl & 1))
            return std::unexpected(system_error::busy);
        auto size = static_cast<unsigned>(std::countr_zero(bytes)) - 1;
        if (!c_.write(0xE000ED98, region) ||
            !c_.write(0xE000ED9C, static_cast<std::uint32_t>(base)) ||
            !c_.write(0xE000EDA0, 1U | (size << 1) | (tex_s_c_b << 16) | (ap << 24) |
                                      (executable ? 0U : 1U << 28)))
            return std::unexpected(system_error::mmio);
        return {};
    }
    [[nodiscard]] system_result mpu_enable(bool on, bool privileged_background = true,
                                           bool hardfault_nmi = false) noexcept {
        if (!owned_ || c_.reserved.mpu || (!on && hardfault_nmi))
            return std::unexpected(system_error::invalid);
        auto current = c_.read(0xE000ED94);
        if (!current)
            return std::unexpected(current.error());
        if ((*current & 1) && !owns_mpu_)
            return std::unexpected(system_error::busy);
        if (on)
            owns_mpu_ = true;
        arch::data_memory_barrier();
        auto r = c_.write(0xE000ED94, (on ? 1U : 0U) | (hardfault_nmi ? 2U : 0U) |
                                          (privileged_background ? 4U : 0U));
        arch::data_synchronization_barrier();
        arch::instruction_barrier();
        if (r && !on)
            owns_mpu_ = false;
        return r;
    }
    [[nodiscard]] system_result trace(bool on, bool cycle_counter = true) noexcept {
        if (!owned_)
            return std::unexpected(system_error::not_initialized);
        if (c_.reserved.cycle_counter && (!on || !cycle_counter))
            return std::unexpected(system_error::busy);
        auto dem = c_.read(0xE000EDFC), dwt = c_.read(0xE0001000);
        if (!dem || !dwt)
            return std::unexpected(system_error::mmio);
        if (!c_.write(0xE000EDFC, (*dem & ~(1U << 24)) | (on ? 1U << 24 : 0U)) ||
            !c_.write(0xE0001000, (*dwt & ~1U) | (on && cycle_counter ? 1U : 0U)))
            return std::unexpected(system_error::mmio);
        return {};
    }
    [[nodiscard]] system_result itm_port(unsigned port, bool on) noexcept {
        if (!owned_ || port >= 32)
            return std::unexpected(system_error::invalid);
        auto r = c_.read(0xE0000E00);
        if (!r)
            return std::unexpected(r.error());
        if (!c_.write(0xE0000FB0, 0xC5ACCE55) ||
            !c_.write(0xE0000E00, on ? (*r | (1U << port)) : (*r & ~(1U << port))))
            return std::unexpected(system_error::mmio);
        return {};
    }
    [[nodiscard]] system_result shutdown() noexcept {
        if (!owned_)
            return {};
        if (owns_systick_ || owns_mpu_)
            return std::unexpected(system_error::busy);
        auto r = c_.release(60);
        if (r)
            owned_ = false;
        return r;
    }

  private:
    Context &c_;
    bool owned_{}, owns_systick_{}, owns_mpu_{};
};

template <class Context, unsigned Capacity = 16>
    requires(Capacity > 0 && Capacity <= 192)
class interrupt_dispatcher {
  public:
    using callback = void (*)(void *) noexcept;
    struct token {
        unsigned slot{};
        std::uint32_t generation{};
        const void *owner{};
    };
    explicit interrupt_dispatcher(Context &c) noexcept : c_(c) {}
    interrupt_dispatcher(const interrupt_dispatcher &) = delete;
    interrupt_dispatcher &operator=(const interrupt_dispatcher &) = delete;
    [[nodiscard]] system_result initialize() noexcept {
        if (owned_)
            return std::unexpected(system_error::busy);
        auto r = c_.claim(63);
        if (r)
            owned_ = true;
        return r;
    }
    [[nodiscard]] system_value<token> subscribe(unsigned irq, callback fn, void *data,
                                                unsigned priority = 6) noexcept {
        if (!owned_ || !mcu::nrf52840::valid_irq(irq) || !fn || priority > 7 ||
            c_.execution.in_isr())
            return std::unexpected(system_error::invalid);
        return c_.execution.critical([&]() -> system_value<token> {
            for (auto &e : slots_)
                if (e.fn && e.irq == irq && e.priority != priority)
                    return std::unexpected(system_error::busy);
            for (unsigned n = 0; n < Capacity; ++n)
                if (!slots_[n].fn && slots_[n].generation != 0xFFFFFFFFU) {
                    auto &s = slots_[n];
                    s.fn = fn;
                    s.data = data;
                    s.irq = irq;
                    s.priority = priority;
                    ++s.generation;
                    // Publish the callback before enabling delivery. Shared priorities
                    // use byte-sized NVIC IPR writes, preserving adjacent vectors.
                    auto wr = c_.mmio.template write<std::uint8_t>(
                        0xE000E400 + irq, static_cast<std::uint8_t>(priority << 5));
                    if (!wr) {
                        s.fn = nullptr;
                        return std::unexpected(system_error::mmio);
                    }
                    if (!c_.write(0xE000E100 + (irq / 32) * 4, 1U << (irq % 32))) {
                        // ENABLE may have committed. Keep the handler live until clear()
                        // proves delivery is disabled, even though no token was returned.
                        return std::unexpected(system_error::mmio);
                    }
                    return token{n, s.generation, this};
                }
            return std::unexpected(system_error::busy);
        });
    }
    [[nodiscard]] system_result unsubscribe(token t) noexcept {
        if (t.owner != this || t.slot >= Capacity || c_.execution.in_isr())
            return std::unexpected(system_error::invalid);
        return c_.execution.critical([&]() -> system_result {
            auto &s = slots_[t.slot];
            if (!s.fn || s.generation != t.generation || dispatch_depth_)
                return std::unexpected(system_error::busy);
            bool other = false;
            for (unsigned n = 0; n < Capacity; ++n)
                if (n != t.slot && slots_[n].fn && slots_[n].irq == s.irq)
                    other = true;
            if (!other && (!c_.write(0xE000E180 + (s.irq / 32) * 4, 1U << (s.irq % 32)) ||
                           !c_.write(0xE000E280 + (s.irq / 32) * 4, 1U << (s.irq % 32))))
                return std::unexpected(system_error::mmio);
            arch::data_synchronization_barrier();
            s.fn = nullptr;
            return {};
        });
    }
    void dispatch(unsigned irq) noexcept {
        if (!owned_ || !mcu::nrf52840::valid_irq(irq))
            return;
        ++dispatch_depth_;
        for (auto &s : slots_)
            if (s.fn && s.irq == irq)
                s.fn(s.data);
        --dispatch_depth_;
    }
    [[nodiscard]] system_result pend(unsigned irq) noexcept {
        if (!owned_ || !mcu::nrf52840::valid_irq(irq))
            return std::unexpected(system_error::invalid);
        return c_.write(0xE000E200 + irq / 32 * 4, 1U << (irq % 32));
    }
    [[nodiscard]] system_result bind_target() noexcept {
        if (!owned_)
            return std::unexpected(system_error::not_initialized);
        return c_.execution.critical([&]() -> system_result {
            if (mcubsp_promicro_irq_callback_target && mcubsp_promicro_irq_callback_context != this)
                return std::unexpected(system_error::busy);
            mcubsp_promicro_irq_callback_context = this;
            mcubsp_promicro_irq_callback_target = &target_dispatch;
            bound_ = true;
            return {};
        });
    }
    [[nodiscard]] system_result clear() noexcept {
        if (!owned_ || c_.execution.in_isr())
            return std::unexpected(system_error::invalid);
        return c_.execution.critical([&]() -> system_result {
            if (dispatch_depth_)
                return std::unexpected(system_error::busy);
            for (auto &s : slots_)
                if (s.fn) {
                    if (!c_.write(0xE000E180 + s.irq / 32 * 4, 1U << (s.irq % 32)) ||
                        !c_.write(0xE000E280 + s.irq / 32 * 4, 1U << (s.irq % 32)))
                        return std::unexpected(system_error::mmio);
                }
            arch::data_synchronization_barrier();
            for (auto &s : slots_)
                s.fn = nullptr;
            return {};
        });
    }
    [[nodiscard]] system_result shutdown() noexcept {
        if (!owned_)
            return {};
        for (auto &s : slots_)
            if (s.fn)
                return std::unexpected(system_error::busy);
        return c_.execution.critical([&]() -> system_result {
            if (bound_) {
                mcubsp_promicro_irq_callback_target = nullptr;
                mcubsp_promicro_irq_callback_context = nullptr;
                bound_ = false;
            }
            auto r = c_.release(63);
            if (r)
                owned_ = false;
            return r;
        });
    }

  private:
    static void target_dispatch(void *p, unsigned irq) noexcept {
        static_cast<interrupt_dispatcher *>(p)->dispatch(irq);
    }
    struct entry {
        callback fn{};
        void *data{};
        unsigned irq{}, priority{};
        std::uint32_t generation{};
    };
    Context &c_;
    std::array<entry, Capacity> slots_{};
    unsigned dispatch_depth_{};
    bool owned_{}, bound_{};
};
} // namespace mcubsp::backend::nrf52840
