#pragma once

// @req-file REQ-0014 REQ-0045
#include <array>
#include <cstddef>
#include <cstdint>
#include <utility>
#include <mcubsp/io/analog.hpp>
#include <mcubsp/io/i2c.hpp>
#include <mcubsp/io/pwm.hpp>
#include <mcubsp/io/serial.hpp>
#include <mcubsp/io/spi.hpp>

namespace mcubsp::io {

// Bindings retain native compile-time instance/pin validation. These selectors
// supply wiring/backend aliases without asking applications to derive profiles.
template<class Binding> struct serial_selection {
    template<class B> struct profile : B {
        using serial_binding = Binding;
        static constexpr auto serial_pins=[] {
            if constexpr(requires{Binding::facade_pins;})return Binding::facade_pins;
            else return std::array{Binding::rx_pin, Binding::tx_pin};
        }();
        static constexpr auto solve_serial(std::uint32_t hz) noexcept {
            if constexpr(requires{B::template solve_serial_binding<Binding>(hz);})return B::template solve_serial_binding<Binding>(hz);
            else return B::solve_serial(hz);
        }
        template<class M, class E> using serial_backend = typename B::template serial_backend_for<Binding, M, E>;
    };
    template<class C> using feature = serial_port<C>;
};
template<class Binding> struct i2c_selection {
    template<class B> struct profile : B {
        using i2c_binding = Binding;
        static constexpr auto solve_i2c(std::uint32_t hz) noexcept {
            if constexpr(requires{B::template solve_i2c_binding<Binding>(hz);})return B::template solve_i2c_binding<Binding>(hz);
            else return B::solve_i2c(hz);
        }
        static constexpr std::array i2c_pins{Binding::scl_pin, Binding::sda_pin};
        template<class M, class E> using i2c_backend = typename B::template i2c_backend_for<Binding, M, E>;
    };
    template<class C> using feature = i2c_bus<C>;
};
template<class Binding> struct spi_selection {
    template<class B> struct profile : B {
        using spi_binding = Binding;
        static constexpr auto solve_spi(std::uint32_t hz) noexcept {
            if constexpr(requires{B::template solve_spi_binding<Binding>(hz);})return B::template solve_spi_binding<Binding>(hz);
            else return B::solve_spi(hz);
        }
        static constexpr std::array spi_pins{Binding::miso_pin, Binding::mosi_pin, Binding::clock_pin, Binding::select_pin};
        template<class M, class E> using spi_backend = typename B::template spi_backend_for<Binding, M, E>;
    };
    template<class C> using feature = spi_bus<C>;
};
template<class Binding> struct analog_selection {
    template<class B> struct profile : B {
        using adc_binding = Binding;
        static constexpr auto adc_pins = Binding::pins;
        static constexpr auto adc_channels = [] {
            std::array<driver::adc_channel, Binding::channels.size()> result{};
            for (std::size_t i = 0; i < result.size(); ++i) result[i] = driver::adc_channel{Binding::channels[i]};
            return result;
        }();
        template<class M, class E> using adc_backend = typename B::template adc_backend_for<Binding, M, E>;
    };
    template<class C> using feature = analog_input<C>;
};
template<class Binding> struct pwm_selection {
    template<class B> struct profile : B {
        using pwm_binding = Binding;
        static constexpr auto pwm_pin = Binding::pin;
        static constexpr std::array pwm_pins{Binding::pin};
        template<class M, class E> using pwm_backend = typename B::template pwm_backend_for<Binding, M, E>;
        static constexpr auto solve_pwm(std::uint32_t hz) noexcept { return B::template solve_pwm_binding<Binding>(hz); }
    };
    template<class C> using feature = pwm_output<C>;
};

template<class Board, class... Selections> struct select_profile;
template<class Board> struct select_profile<Board> { using type = Board; };
template<class Board, class First, class... Rest>
struct select_profile<Board, First, Rest...> : select_profile<typename First::template profile<Board>, Rest...> {};
template<class Board, class... Selections>
using select = typename select_profile<Board, Selections...>::type;

namespace detail {
// All selected features borrow the same pin registry and execution service.
template<class Context, class Profile> class selected_context {
public:
    using board_type = Profile;
    using mmio_type = typename Context::mmio_type;
    using execution_type = typename Context::execution_type;
    explicit selected_context(Context& parent) noexcept : parent_(parent) {}
    auto& mmio() noexcept { return parent_.mmio(); }
    auto& execution() noexcept { return parent_.execution(); }
    auto& native_services() noexcept requires requires(Context& c){c.native_services();} {return parent_.native_services();}
    [[nodiscard]] bool started() const noexcept { return parent_.started(); }
    auto claim(std::span<const core::pin_id> pins) noexcept { return parent_.claim(pins); }
    void release(std::span<const core::pin_id> pins) noexcept { parent_.release(pins); }
    void retain() noexcept { parent_.retain(); }
    void drop() noexcept { parent_.drop(); }
    template<class D> auto deadline_after(D timeout) noexcept { return parent_.deadline_after(timeout); }
private:
    Context& parent_;
};
template<class Context, class Selection> class selected_peripheral {
    using profile = typename Selection::template profile<typename Context::board_type>;
    selected_context<Context, profile> context_;
public:
    typename Selection::template feature<selected_context<Context, profile>> peripheral;
    explicit selected_peripheral(Context& parent) noexcept : context_(parent), peripheral(context_) {}
};
} // namespace detail
} // namespace mcubsp::io
