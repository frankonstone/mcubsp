#pragma once

#include <cstddef>
#include <cstdint>
#include <expected>
#include <optional>
#include <span>

#include <mcuhw/core/transfer.hpp>
#include <mcuhw/driver/uart.hpp>
#include <mcuhw/mcu/esp32p4/description.hpp>

namespace mcuhw::backend::esp32p4 {

namespace detail {

template<std::uint8_t Instance>
struct uart_owner { static inline bool claimed{}; };

[[nodiscard]] constexpr mcu::esp32p4::signal uart_rx(
    std::uint8_t instance) noexcept {
    return instance == 0U ? mcu::esp32p4::signal::uart0_rx
                          : mcu::esp32p4::signal::uart1_rx;
}

[[nodiscard]] constexpr mcu::esp32p4::signal uart_tx(
    std::uint8_t instance) noexcept {
    return instance == 0U ? mcu::esp32p4::signal::uart0_tx
                          : mcu::esp32p4::signal::uart1_tx;
}

[[nodiscard]] constexpr std::uintptr_t uart_base(
    std::uint8_t instance) noexcept {
    return mcu::esp32p4::uart0_base
        + static_cast<std::uintptr_t>(instance) * 0x1000U;
}

} // namespace detail

// HP UART uses an 8-bit integer source pre-divider followed by a 12.4-bit
// UART divider. The fixed-point UART divider is retained in divisor and the
// source pre-divider in backend_encoding.
// @req REQ-0011 REQ-0027 REQ-0034
[[nodiscard]] constexpr std::expected<driver::uart_baud_timing,
                                      driver::uart_error>
solve_uart_baud(driver::uart_baud_request request) noexcept {
    if (request.clock_hz != 40'000'000U
        && request.clock_hz != 80'000'000U) {
        return std::unexpected(driver::uart_error::invalid_clock);
    }
    if (request.target_baud == 0U) {
        return std::unexpected(driver::uart_error::invalid_baud);
    }

    bool found{};
    driver::uart_baud_timing best{};
    for (std::uint32_t source_divider = 1U; source_divider <= 256U;
         ++source_divider) {
        const auto fixed_divider = driver::uart_detail::rounded_divide(
            static_cast<std::uint64_t>(request.clock_hz) * 16U,
            static_cast<std::uint64_t>(request.target_baud)
                * source_divider);
        if (fixed_divider < 16U || fixed_divider > 65'535U) {
            continue;
        }
        const auto source_ticks = static_cast<std::uint64_t>(request.clock_hz)
            * 16U;
        const auto target_ticks = static_cast<std::uint64_t>(
            request.target_baud) * source_divider * fixed_divider;
        const auto difference = source_ticks > target_ticks
            ? source_ticks - target_ticks : target_ticks - source_ticks;
        const auto error = static_cast<std::uint32_t>(
            driver::uart_detail::rounded_divide(
                difference * 1'000'000ULL, source_ticks));
        const auto candidate = driver::uart_baud_timing{
            .clock_hz = request.clock_hz,
            .target_baud = request.target_baud,
            .achieved_baud = static_cast<std::uint32_t>(
                driver::uart_detail::rounded_divide(
                    source_ticks, source_divider * fixed_divider)),
            .error_ppm = error,
            .divisor = static_cast<std::uint16_t>(fixed_divider),
            .clock_select = static_cast<std::uint8_t>(
                request.clock_hz == 80'000'000U ? 2U : 0U),
            .brr = 0U,
            .double_speed = false,
            .asynchronous_base_clock_select = false,
            .extended_base_clock_select = false,
            .backend_encoding = source_divider,
        };
        if (!found || candidate.error_ppm < best.error_ppm
            || (candidate.error_ppm == best.error_ppm
                && candidate.backend_encoding < best.backend_encoding)) {
            best = candidate;
            found = true;
        }
    }
    if (!found || best.error_ppm > request.maximum_error_ppm) {
        return std::unexpected(driver::uart_error::unattainable_tolerance);
    }
    return best;
}

template<std::uint8_t Instance, core::pin_id RxPin, core::pin_id TxPin>
struct uart_binding {
    static_assert(Instance <= 1U,
                  "source-backed direct routes cover HP UART0 and UART1");
    static constexpr auto rx_signal = detail::uart_rx(Instance);
    static constexpr auto tx_signal = detail::uart_tx(Instance);
    static_assert(core::route_exists(mcu::esp32p4::pin_routes, RxPin,
                                     mcu::esp32p4::signal_id(rx_signal)));
    static_assert(core::route_exists(mcu::esp32p4::pin_routes, TxPin,
                                     mcu::esp32p4::signal_id(tx_signal)));
    static_assert(core::package_contains(mcu::esp32p4::package_pins, RxPin,
                                         core::electrical_mode::input));
    static_assert(core::package_contains(mcu::esp32p4::package_pins, TxPin,
                                         core::electrical_mode::push_pull));
    static constexpr std::uint8_t instance = Instance;
    static constexpr core::pin_id rx_pin = RxPin;
    static constexpr core::pin_id tx_pin = TxPin;
    static constexpr std::uintptr_t base = detail::uart_base(Instance);
    static constexpr auto clocks = mcu::esp32p4::uart_clock_controls[Instance];
};

// Fixed-storage finite transfers. handle_interrupt() can be called by an HP
// interrupt-matrix handler or by a bounded bare-metal polling hook. TX is not
// complete until TX_DONE proves that the final stop bit left the pin.
template<typename Binding, typename MmioPolicy, typename ExecutionPolicy>
// @req REQ-0003 REQ-0009 REQ-0011 REQ-0014 REQ-0016 REQ-0017 REQ-0019 REQ-0027 REQ-0034
class uart_backend {
public:
    using tx_pool_type = core::operation_pool<void, driver::uart_error, 1,
                                               ExecutionPolicy>;
    using rx_pool_type = core::operation_pool<std::size_t, driver::uart_error,
                                               1, ExecutionPolicy>;
    using tx_transfer = typename tx_pool_type::transfer_type;
    using rx_transfer = typename rx_pool_type::transfer_type;
    using tx_start_result = std::expected<tx_transfer,
        typename tx_pool_type::error_type>;
    using rx_start_result = std::expected<rx_transfer,
        typename rx_pool_type::error_type>;

    uart_backend(MmioPolicy& mmio, ExecutionPolicy& execution) noexcept
        : mmio_(mmio), tx_operations_(execution), rx_operations_(execution) {}

    [[nodiscard]] std::expected<void, driver::uart_error> initialize(
        driver::uart_baud_timing timing, driver::uart_format format) noexcept {
        if (initialized_ || detail::uart_owner<Binding::instance>::claimed) {
            return std::unexpected(driver::uart_error::already_owned);
        }
        if ((format.data_bits != driver::uart_data_bits::seven
             && format.data_bits != driver::uart_data_bits::eight)
            || (format.stop_bits != driver::uart_stop_bits::one
                && format.stop_bits != driver::uart_stop_bits::two)) {
            return std::unexpected(driver::uart_error::invalid_format);
        }
        if (timing.divisor < 16U || timing.backend_encoding == 0U
            || timing.backend_encoding > 256U || timing.clock_select > 2U) {
            return std::unexpected(driver::uart_error::invalid_baud);
        }

        detail::uart_owner<Binding::instance>::claimed = true;
        owned_ = true;
        const auto fixed_divider = timing.divisor;
        const auto divider_register =
            static_cast<std::uint32_t>(fixed_divider / 16U)
            | (static_cast<std::uint32_t>(fixed_divider % 16U) << 20U);

        if (!modify(Binding::clocks.system_gate_register,
                    Binding::clocks.system_gate_mask, 0U)
            || !modify(Binding::clocks.apb_gate_register,
                       Binding::clocks.apb_gate_mask, 0U)
            || !pulse_reset()
            || !modify(Binding::clocks.source_register,
                       (1U << 26U)
                           | (static_cast<std::uint32_t>(timing.clock_select)
                              << 24U),
                       0x3U << 24U)
            || !modify(Binding::clocks.divider_register,
                       timing.backend_encoding - 1U, 0x00FF'FFFFU)
            || !modify(Binding::base + reg_clock_configuration,
                       (1U << 24U) | (1U << 25U),
                       (1U << 26U) | (1U << 27U))
            || !write(reg_interrupt_enable, 0U)
            || !write(reg_interrupt_clear, all_interrupts)
            || !write(reg_clock_divider, divider_register)
            || !configure_format(format)
            || !modify(Binding::base + reg_configuration1,
                       1U << 8U, 0xFFFFU)
            || !reset_fifos()
            || !configure_pin(Binding::tx_pin, false)
            || !configure_pin(Binding::rx_pin, true)) {
            release_owner();
            return mmio_failure();
        }
        initialized_ = true;
        return {};
    }

    [[nodiscard]] tx_start_result transmit(
        std::span<const std::uint8_t> bytes) noexcept {
        if (!initialized_) return tx_failure(driver::uart_error::not_initialized);
        if (bytes.empty()) return tx_failure(driver::uart_error::empty_transfer);
        if (tx_id_) return tx_failure(driver::uart_error::busy);
        auto started = tx_operations_.try_start(
            {.quiesce = &cancel_tx, .context = this});
        if (!started) return std::unexpected(started.error());
        tx_ = bytes;
        tx_index_ = 0U;
        tx_id_ = started->id();
        if (!write(reg_interrupt_clear, int_tx_empty | int_tx_done)
            || !fill_transmit_fifo()) {
            (void)fail_tx(driver::uart_error::mmio);
        }
        return started;
    }

    [[nodiscard]] rx_start_result receive(
        std::span<std::uint8_t> bytes) noexcept {
        if (!initialized_) return rx_failure(driver::uart_error::not_initialized);
        if (bytes.empty()) return rx_failure(driver::uart_error::empty_transfer);
        if (rx_id_) return rx_failure(driver::uart_error::busy);
        auto started = rx_operations_.try_start(
            {.quiesce = &cancel_rx, .context = this});
        if (!started) return std::unexpected(started.error());
        rx_ = bytes;
        rx_index_ = 0U;
        rx_id_ = started->id();
        if (!write(reg_interrupt_clear, rx_interrupts)
            || !set_interrupts(rx_interrupts, 0U)) {
            (void)fail_rx(driver::uart_error::mmio);
        }
        return started;
    }

    [[nodiscard]] bool handle_interrupt() noexcept {
        const auto interrupts = read(reg_interrupt_status);
        if (!interrupts) return false;
        if (rx_id_ && (*interrupts & rx_errors) != 0U) {
            auto error = driver::uart_error::transport;
            if ((*interrupts & int_rx_overflow) != 0U) {
                error = driver::uart_error::overrun;
            } else if ((*interrupts & int_frame_error) != 0U) {
                error = driver::uart_error::framing;
            } else if ((*interrupts & int_parity_error) != 0U) {
                error = driver::uart_error::parity;
            }
            (void)write(reg_interrupt_clear, *interrupts & rx_errors);
            return fail_rx(error);
        }
        if (rx_id_ && (*interrupts & int_rx_full) != 0U) {
            const auto status = read(reg_status);
            if (!status) return fail_rx(driver::uart_error::mmio);
            auto available = static_cast<std::size_t>(*status & 0xFFU);
            bool progressed{};
            while (available != 0U && rx_index_ < rx_.size()) {
                const auto value = read(reg_fifo);
                if (!value) return fail_rx(driver::uart_error::mmio);
                rx_[rx_index_++] = static_cast<std::uint8_t>(*value);
                --available;
                progressed = true;
            }
            if (!write(reg_interrupt_clear, int_rx_full)) {
                return fail_rx(driver::uart_error::mmio);
            }
            if (rx_index_ == rx_.size()) {
                if (!set_interrupts(0U, rx_interrupts)) {
                    return fail_rx(driver::uart_error::mmio);
                }
                const auto id = *rx_id_;
                const auto count = rx_index_;
                rx_id_.reset();
                rx_ = {};
                return rx_operations_.complete(id, count);
            }
            return progressed;
        }
        if (tx_id_ && (*interrupts & int_tx_empty) != 0U) {
            if (!write(reg_interrupt_clear, int_tx_empty)) {
                return fail_tx(driver::uart_error::mmio);
            }
            return fill_transmit_fifo()
                || fail_tx(driver::uart_error::mmio);
        }
        if (tx_id_ && (*interrupts & int_tx_done) != 0U) {
            if (!write(reg_interrupt_clear, int_tx_done)
                || !set_interrupts(0U, int_tx_done | int_tx_empty)) {
                return fail_tx(driver::uart_error::mmio);
            }
            const auto id = *tx_id_;
            tx_id_.reset();
            tx_ = {};
            return tx_operations_.complete(id);
        }
        return false;
    }

    [[nodiscard]] bool handle_transmit_data_empty_interrupt() noexcept {
        return handle_interrupt();
    }
    [[nodiscard]] bool handle_transmit_end_interrupt() noexcept {
        return handle_interrupt();
    }
    [[nodiscard]] bool handle_receive_interrupt() noexcept {
        return handle_interrupt();
    }
    [[nodiscard]] bool handle_receive_error_interrupt() noexcept {
        return handle_interrupt();
    }

    [[nodiscard]] std::expected<void, driver::uart_error> shutdown() noexcept {
        if (!initialized_) {
            return std::unexpected(driver::uart_error::not_initialized);
        }
        if (tx_id_ || rx_id_) return std::unexpected(driver::uart_error::busy);
        if (!set_interrupts(0U, all_interrupts)
            || !restore_pin(Binding::rx_pin)
            || !restore_pin(Binding::tx_pin)
            || !modify(Binding::base + reg_clock_configuration, 0U,
                       (1U << 24U) | (1U << 25U))
            || !modify(Binding::clocks.source_register, 0U, 1U << 26U)
            || !modify(Binding::clocks.apb_gate_register, 0U,
                       Binding::clocks.apb_gate_mask)
            || !modify(Binding::clocks.system_gate_register, 0U,
                       Binding::clocks.system_gate_mask)) {
            return mmio_failure();
        }
        initialized_ = false;
        release_owner();
        return {};
    }

private:
    static constexpr std::uintptr_t reg_fifo = 0x00U;
    static constexpr std::uintptr_t reg_interrupt_status = 0x08U;
    static constexpr std::uintptr_t reg_interrupt_enable = 0x0CU;
    static constexpr std::uintptr_t reg_interrupt_clear = 0x10U;
    static constexpr std::uintptr_t reg_clock_divider = 0x14U;
    static constexpr std::uintptr_t reg_status = 0x1CU;
    static constexpr std::uintptr_t reg_configuration0 = 0x20U;
    static constexpr std::uintptr_t reg_configuration1 = 0x24U;
    static constexpr std::uintptr_t reg_clock_configuration = 0x88U;
    static constexpr std::uintptr_t reg_update = 0x98U;
    static constexpr std::uint32_t int_rx_full = 1U << 0U;
    static constexpr std::uint32_t int_tx_empty = 1U << 1U;
    static constexpr std::uint32_t int_parity_error = 1U << 2U;
    static constexpr std::uint32_t int_frame_error = 1U << 3U;
    static constexpr std::uint32_t int_rx_overflow = 1U << 4U;
    static constexpr std::uint32_t int_tx_done = 1U << 14U;
    static constexpr std::uint32_t rx_errors = int_parity_error
        | int_frame_error | int_rx_overflow;
    static constexpr std::uint32_t rx_interrupts = int_rx_full | rx_errors;
    static constexpr std::uint32_t all_interrupts = 0x000F'FFFFU;
    static constexpr std::uint32_t fifo_capacity = 128U;
    static constexpr std::size_t update_wait_limit = 1024U;

    [[nodiscard]] bool configure_format(driver::uart_format format) noexcept {
        const auto current = read(reg_configuration0);
        if (!current) return false;
        constexpr std::uint32_t controlled = 0x3FU;
        auto next = static_cast<std::uint32_t>(*current & ~controlled);
        next |= (format.data_bits == driver::uart_data_bits::eight ? 3U : 2U)
            << 2U;
        next |= (format.stop_bits == driver::uart_stop_bits::two ? 3U : 1U)
            << 4U;
        if (format.parity != driver::uart_parity::none) {
            next |= 1U << 1U;
            if (format.parity == driver::uart_parity::odd) next |= 1U;
        }
        return write(reg_configuration0, next) && synchronize();
    }

    [[nodiscard]] bool reset_fifos() noexcept {
        const auto current = read(reg_configuration0);
        if (!current) return false;
        constexpr std::uint32_t reset = (1U << 22U) | (1U << 23U);
        return write(reg_configuration0, *current | reset) && synchronize()
            && write(reg_configuration0, *current & ~reset) && synchronize();
    }

    [[nodiscard]] bool synchronize() noexcept {
        if (!write(reg_update, 1U)) return false;
        for (std::size_t attempt = 0U; attempt < update_wait_limit; ++attempt) {
            const auto value = read(reg_update);
            if (!value) return false;
            if ((*value & 1U) == 0U) return true;
        }
        return false;
    }

    [[nodiscard]] bool pulse_reset() noexcept {
        const auto reset_mask = Binding::clocks.apb_reset_mask;
        return modify(Binding::clocks.reset_register,
                      reset_mask, 0U)
            && modify(Binding::clocks.reset_register, 0U,
                      reset_mask);
    }

    [[nodiscard]] bool configure_pin(core::pin_id pin, bool input) noexcept {
        const auto address = mcu::esp32p4::io_mux_base + 0x04U
            + static_cast<std::uintptr_t>(pin.index) * 4U;
        const auto current = read_absolute(address);
        if (!current) return false;
        constexpr std::uint32_t function_mask = 0x7U << 12U;
        auto next = static_cast<std::uint32_t>(*current & ~function_mask);
        if (input) next |= 1U << 9U;
        return write_absolute(address, next);
    }

    [[nodiscard]] bool restore_pin(core::pin_id pin) noexcept {
        const auto address = mcu::esp32p4::io_mux_base + 0x04U
            + static_cast<std::uintptr_t>(pin.index) * 4U;
        const auto current = read_absolute(address);
        if (!current) return false;
        constexpr std::uint32_t function_mask = 0x7U << 12U;
        const auto next = static_cast<std::uint32_t>(
            (*current & ~function_mask) | (1U << 12U) | (1U << 9U));
        return write_absolute(address, next);
    }

    [[nodiscard]] bool set_interrupts(std::uint32_t set,
                                      std::uint32_t clear) noexcept {
        const auto current = read(reg_interrupt_enable);
        return current && write(reg_interrupt_enable,
                                (*current | set) & ~clear);
    }

    [[nodiscard]] bool fill_transmit_fifo() noexcept {
        const auto status = read(reg_status);
        if (!status) return false;
        const auto queued = static_cast<std::size_t>(
            (*status >> 16U) & 0xFFU);
        auto writable = queued < fifo_capacity
            ? fifo_capacity - queued : 0U;
        while (writable != 0U && tx_index_ < tx_.size()) {
            if (!write(reg_fifo, tx_[tx_index_++])) return false;
            --writable;
        }
        return tx_index_ == tx_.size()
            ? set_interrupts(int_tx_done, int_tx_empty)
            : set_interrupts(int_tx_empty, int_tx_done);
    }

    [[nodiscard]] static core::quiesce_result cancel_tx(
        void* context, core::operation_id id) noexcept {
        auto& self = *static_cast<uart_backend*>(context);
        if (!self.tx_id_ || *self.tx_id_ != id) {
            return core::quiesce_result::reusable;
        }
        const bool stopped = self.set_interrupts(
            0U, int_tx_empty | int_tx_done);
        self.tx_id_.reset();
        self.tx_ = {};
        return stopped ? core::quiesce_result::reusable
                       : core::quiesce_result::faulted;
    }

    [[nodiscard]] static core::quiesce_result cancel_rx(
        void* context, core::operation_id id) noexcept {
        auto& self = *static_cast<uart_backend*>(context);
        if (!self.rx_id_ || *self.rx_id_ != id) {
            return core::quiesce_result::reusable;
        }
        const bool stopped = self.set_interrupts(0U, rx_interrupts);
        self.rx_id_.reset();
        self.rx_ = {};
        return stopped ? core::quiesce_result::reusable
                       : core::quiesce_result::faulted;
    }

    [[nodiscard]] bool fail_tx(driver::uart_error error) noexcept {
        if (!tx_id_) return false;
        (void)set_interrupts(0U, int_tx_empty | int_tx_done);
        const auto id = *tx_id_;
        tx_id_.reset();
        tx_ = {};
        return tx_operations_.fail(id, error);
    }

    [[nodiscard]] bool fail_rx(driver::uart_error error) noexcept {
        if (!rx_id_) return false;
        (void)set_interrupts(0U, rx_interrupts);
        const auto id = *rx_id_;
        rx_id_.reset();
        rx_ = {};
        return rx_operations_.fail(id, error);
    }

    [[nodiscard]] auto read(std::uintptr_t offset) noexcept {
        return read_absolute(Binding::base + offset);
    }

    [[nodiscard]] bool write(std::uintptr_t offset,
                             std::uint32_t value) noexcept {
        return write_absolute(Binding::base + offset, value);
    }

    [[nodiscard]] auto read_absolute(std::uintptr_t address) noexcept {
        return mmio_.template read<std::uint32_t>(address);
    }

    [[nodiscard]] bool write_absolute(std::uintptr_t address,
                                      std::uint32_t value) noexcept {
        return mmio_.template write<std::uint32_t>(address, value).has_value();
    }

    [[nodiscard]] bool modify(std::uintptr_t address, std::uint32_t set,
                              std::uint32_t clear) noexcept {
        const auto current = read_absolute(address);
        return current && write_absolute(address, (*current & ~clear) | set);
    }

    void release_owner() noexcept {
        if (owned_) {
            detail::uart_owner<Binding::instance>::claimed = false;
            owned_ = false;
        }
    }

    [[nodiscard]] static tx_start_result tx_failure(
        driver::uart_error error) noexcept {
        return std::unexpected(typename tx_pool_type::error_type{
            .kind = core::transfer_error_kind::invalid_operation,
            .cause = error});
    }

    [[nodiscard]] static rx_start_result rx_failure(
        driver::uart_error error) noexcept {
        return std::unexpected(typename rx_pool_type::error_type{
            .kind = core::transfer_error_kind::invalid_operation,
            .cause = error});
    }

    [[nodiscard]] static std::expected<void, driver::uart_error>
    mmio_failure() noexcept {
        return std::unexpected(driver::uart_error::mmio);
    }

    MmioPolicy& mmio_;
    tx_pool_type tx_operations_;
    rx_pool_type rx_operations_;
    std::span<const std::uint8_t> tx_{};
    std::span<std::uint8_t> rx_{};
    std::optional<core::operation_id> tx_id_{};
    std::optional<core::operation_id> rx_id_{};
    std::size_t tx_index_{};
    std::size_t rx_index_{};
    bool owned_{};
    bool initialized_{};
};

} // namespace mcuhw::backend::esp32p4
