#pragma once

#include <cstddef>
#include <cstdint>
#include <expected>
#include <optional>
#include <span>

#include <mcuhw/backend/stm32h753/detail.hpp>
#include <mcuhw/core/transfer.hpp>
#include <mcuhw/driver/uart.hpp>

namespace mcuhw::backend::stm32h753 {

namespace detail {

template<std::uint8_t Instance>
struct usart_owner { static inline bool claimed{}; };

[[nodiscard]] constexpr mcu::stm32h753::signal usart_rx(
    std::uint8_t instance) noexcept {
    return instance == 3U ? mcu::stm32h753::signal::usart3_rx
                          : mcu::stm32h753::signal::usart1_rx;
}
[[nodiscard]] constexpr mcu::stm32h753::signal usart_tx(
    std::uint8_t instance) noexcept {
    return instance == 3U ? mcu::stm32h753::signal::usart3_tx
                          : mcu::stm32h753::signal::usart1_tx;
}

} // namespace detail

// OVER8=0: USARTDIV is encoded directly in BRR and baud = kernel/BRR.
// @req REQ-0011 REQ-0027 REQ-0033
[[nodiscard]] constexpr std::expected<driver::uart_baud_timing,
                                      driver::uart_error>
solve_usart_baud(driver::uart_baud_request request) noexcept {
    if (request.clock_hz == 0U) {
        return std::unexpected(driver::uart_error::invalid_clock);
    }
    if (request.target_baud == 0U) {
        return std::unexpected(driver::uart_error::invalid_baud);
    }
    const auto brr = driver::uart_detail::rounded_divide(
        request.clock_hz, request.target_baud);
    if (brr < 16U || brr > 65'535U) {
        return std::unexpected(driver::uart_error::unattainable_tolerance);
    }
    const auto error = driver::uart_detail::baud_error_ppm(
        request.clock_hz, request.target_baud, brr);
    if (error > request.maximum_error_ppm) {
        return std::unexpected(driver::uart_error::unattainable_tolerance);
    }
    return driver::uart_baud_timing{
        .clock_hz = request.clock_hz,
        .target_baud = request.target_baud,
        .achieved_baud = static_cast<std::uint32_t>(
            driver::uart_detail::rounded_divide(request.clock_hz, brr)),
        .error_ppm = error,
        .divisor = static_cast<std::uint16_t>(brr),
        .clock_select = 0U,
        .brr = 0U,
        .double_speed = false,
        .asynchronous_base_clock_select = false,
        .extended_base_clock_select = false,
        .backend_encoding = static_cast<std::uint32_t>(brr),
    };
}

template<std::uint8_t Instance, core::pin_id RxPin, core::pin_id TxPin>
struct usart_binding {
    static_assert(Instance == 1U || Instance == 3U,
                  "source-backed initial routes cover USART1 and USART3");
    static constexpr auto rx_signal = detail::usart_rx(Instance);
    static constexpr auto tx_signal = detail::usart_tx(Instance);
    static_assert(core::route_exists(mcu::stm32h753::pin_routes, RxPin,
                                     mcu::stm32h753::signal_id(rx_signal)));
    static_assert(core::route_exists(mcu::stm32h753::pin_routes, TxPin,
                                     mcu::stm32h753::signal_id(tx_signal)));
    static_assert(mcu::stm32h753::package_contains(
        mcu::stm32h753::package_variant::lqfp144, RxPin,
        mcu::stm32h753::package_memberships));
    static_assert(mcu::stm32h753::package_contains(
        mcu::stm32h753::package_variant::lqfp144, TxPin,
        mcu::stm32h753::package_memberships));
    static constexpr std::uint8_t instance = Instance;
    static constexpr core::pin_id rx_pin = RxPin;
    static constexpr core::pin_id tx_pin = TxPin;
    static constexpr auto metadata = detail::instance(
        mcu::stm32h753::peripheral_kind::usart, Instance);
    static constexpr auto rx_af = detail::alternate_function(RxPin, rx_signal);
    static constexpr auto tx_af = detail::alternate_function(TxPin, tx_signal);
};

// Interrupt-driven finite transfers. TX completes only after TC (the final
// stop bit has left the pin); RX publishes the exact requested byte count.
template<typename Binding, typename MmioPolicy, typename ExecutionPolicy>
// @req REQ-0003 REQ-0009 REQ-0014 REQ-0017 REQ-0019 REQ-0027 REQ-0033
class usart_backend {
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

    usart_backend(MmioPolicy& mmio, ExecutionPolicy& execution) noexcept
        : mmio_(mmio), tx_operations_(execution), rx_operations_(execution) {}

    [[nodiscard]] std::expected<void, driver::uart_error> initialize(
        driver::uart_baud_timing timing, driver::uart_format format) noexcept {
        if (initialized_ || detail::usart_owner<Binding::instance>::claimed) {
            return std::unexpected(driver::uart_error::already_owned);
        }
        if ((format.data_bits != driver::uart_data_bits::seven
             && format.data_bits != driver::uart_data_bits::eight)
            || (format.stop_bits != driver::uart_stop_bits::one
                && format.stop_bits != driver::uart_stop_bits::two)
            || timing.backend_encoding < 16U
            || timing.backend_encoding > 65'535U) {
            return std::unexpected(driver::uart_error::invalid_format);
        }
        detail::usart_owner<Binding::instance>::claimed = true;
        owned_ = true;
        detail::pinmux<MmioPolicy> pins{mmio_};
        if (!detail::enable_peripheral_clock(mmio_, Binding::metadata)
            || !write(0x00U, 0U)
            || !pins.alternate(Binding::tx_pin, Binding::tx_af)
            || !pins.alternate(Binding::rx_pin, Binding::rx_af, false, 1U)) {
            release_owner();
            return mmio_failure();
        }
        std::uint32_t cr1{};
        if (format.data_bits == driver::uart_data_bits::seven
            && format.parity == driver::uart_parity::none) {
            cr1 |= 1U << 28U; // seven total bits
        } else if (format.data_bits == driver::uart_data_bits::eight
                   && format.parity != driver::uart_parity::none) {
            cr1 |= 1U << 12U; // nine total bits: eight data plus parity
        }
        if (format.parity != driver::uart_parity::none) {
            cr1 |= 1U << 10U;
            if (format.parity == driver::uart_parity::odd) {
                cr1 |= 1U << 9U;
            }
        }
        const auto stop = format.stop_bits == driver::uart_stop_bits::two
            ? 2U << 12U : 0U;
        if (!write(0x04U, stop) || !write(0x08U, 0U)
            || !write(0x0CU, timing.backend_encoding)
            || !write(0x20U, 0x0000'0C5FU)
            || !write(0x00U, cr1 | cr_re | cr_te | cr_ue)) {
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
        if (!update_cr1(cr_txeie, cr_tcie)) {
            (void)fail_tx(driver::uart_error::mmio);
        }
        return started;
    }

    [[nodiscard]] rx_start_result receive(std::span<std::uint8_t> bytes) noexcept {
        if (!initialized_) return rx_failure(driver::uart_error::not_initialized);
        if (bytes.empty()) return rx_failure(driver::uart_error::empty_transfer);
        if (rx_id_) return rx_failure(driver::uart_error::busy);
        auto started = rx_operations_.try_start(
            {.quiesce = &cancel_rx, .context = this});
        if (!started) return std::unexpected(started.error());
        rx_ = bytes;
        rx_index_ = 0U;
        rx_id_ = started->id();
        if (!update_cr1(cr_rxneie | cr_peie, 0U)
            || !update_cr3(1U, 0U)) { // EIE
            (void)fail_rx(driver::uart_error::mmio);
        }
        return started;
    }

    [[nodiscard]] bool handle_interrupt() noexcept {
        const auto status = read(0x1CU);
        if (!status) return false;
        if (rx_id_ && (*status & 0x0FU) != 0U) {
            auto error = driver::uart_error::transport;
            if ((*status & (1U << 3U)) != 0U) error = driver::uart_error::overrun;
            else if ((*status & (1U << 1U)) != 0U) error = driver::uart_error::framing;
            else if ((*status & 1U) != 0U) error = driver::uart_error::parity;
            (void)write(0x20U, *status & 0x0FU);
            return fail_rx(error);
        }
        if (rx_id_ && (*status & (1U << 5U)) != 0U) {
            const auto value = read(0x24U);
            if (!value) return fail_rx(driver::uart_error::mmio);
            rx_[rx_index_++] = static_cast<std::uint8_t>(*value);
            if (rx_index_ == rx_.size()) {
                if (!update_cr1(0U, cr_rxneie | cr_peie)
                    || !update_cr3(0U, 1U)) {
                    return fail_rx(driver::uart_error::mmio);
                }
                const auto id = *rx_id_;
                const auto count = rx_index_;
                rx_id_.reset(); rx_ = {};
                return rx_operations_.complete(id, count);
            }
            return true;
        }
        if (tx_id_ && (*status & (1U << 7U)) != 0U
            && tx_index_ < tx_.size()) {
            if (!write(0x28U, tx_[tx_index_++])) {
                return fail_tx(driver::uart_error::mmio);
            }
            if (tx_index_ == tx_.size()
                && !update_cr1(cr_tcie, cr_txeie)) {
                return fail_tx(driver::uart_error::mmio);
            }
            return true;
        }
        if (tx_id_ && (*status & (1U << 6U)) != 0U) {
            if (!write(0x20U, 1U << 6U)
                || !update_cr1(0U, cr_tcie | cr_txeie)) {
                return fail_tx(driver::uart_error::mmio);
            }
            const auto id = *tx_id_;
            tx_id_.reset(); tx_ = {};
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
        detail::pinmux<MmioPolicy> pins{mmio_};
        if (!write(0x00U, 0U) || !pins.restore(Binding::rx_pin)
            || !pins.restore(Binding::tx_pin)) return mmio_failure();
        initialized_ = false;
        release_owner();
        return {};
    }

private:
    static constexpr std::uint32_t cr_ue = 1U;
    static constexpr std::uint32_t cr_re = 1U << 2U;
    static constexpr std::uint32_t cr_te = 1U << 3U;
    static constexpr std::uint32_t cr_rxneie = 1U << 5U;
    static constexpr std::uint32_t cr_tcie = 1U << 6U;
    static constexpr std::uint32_t cr_txeie = 1U << 7U;
    static constexpr std::uint32_t cr_peie = 1U << 8U;

    [[nodiscard]] static core::quiesce_result cancel_tx(
        void* context, core::operation_id id) noexcept {
        auto& self = *static_cast<usart_backend*>(context);
        if (!self.tx_id_ || *self.tx_id_ != id) return core::quiesce_result::reusable;
        const bool ok = self.update_cr1(0U, cr_txeie | cr_tcie);
        self.tx_id_.reset(); self.tx_ = {};
        return ok ? core::quiesce_result::reusable : core::quiesce_result::faulted;
    }
    [[nodiscard]] static core::quiesce_result cancel_rx(
        void* context, core::operation_id id) noexcept {
        auto& self = *static_cast<usart_backend*>(context);
        if (!self.rx_id_ || *self.rx_id_ != id) return core::quiesce_result::reusable;
        const bool ok = self.update_cr1(0U, cr_rxneie | cr_peie)
            && self.update_cr3(0U, 1U);
        self.rx_id_.reset(); self.rx_ = {};
        return ok ? core::quiesce_result::reusable : core::quiesce_result::faulted;
    }
    [[nodiscard]] bool fail_tx(driver::uart_error error) noexcept {
        if (!tx_id_) return false;
        (void)update_cr1(0U, cr_txeie | cr_tcie);
        const auto id = *tx_id_; tx_id_.reset(); tx_ = {};
        return tx_operations_.fail(id, error);
    }
    [[nodiscard]] bool fail_rx(driver::uart_error error) noexcept {
        if (!rx_id_) return false;
        (void)update_cr1(0U, cr_rxneie | cr_peie); (void)update_cr3(0U, 1U);
        const auto id = *rx_id_; rx_id_.reset(); rx_ = {};
        return rx_operations_.fail(id, error);
    }
    [[nodiscard]] bool update_cr1(std::uint32_t set,
                                  std::uint32_t clear) noexcept {
        const auto value = read(0x00U);
        return value && write(0x00U, (*value | set) & ~clear);
    }
    [[nodiscard]] bool update_cr3(std::uint32_t set,
                                  std::uint32_t clear) noexcept {
        const auto value = read(0x08U);
        return value && write(0x08U, (*value | set) & ~clear);
    }
    [[nodiscard]] auto read(std::uintptr_t offset) noexcept {
        return mmio_.template read<std::uint32_t>(Binding::metadata.base + offset);
    }
    [[nodiscard]] bool write(std::uintptr_t offset, std::uint32_t value) noexcept {
        return mmio_.template write<std::uint32_t>(Binding::metadata.base + offset,
                                                   value).has_value();
    }
    void release_owner() noexcept {
        if (owned_) {
            detail::usart_owner<Binding::instance>::claimed = false;
            owned_ = false;
        }
    }
    [[nodiscard]] static tx_start_result tx_failure(driver::uart_error error) noexcept {
        return std::unexpected(typename tx_pool_type::error_type{
            .kind = core::transfer_error_kind::invalid_operation, .cause = error});
    }
    [[nodiscard]] static rx_start_result rx_failure(driver::uart_error error) noexcept {
        return std::unexpected(typename rx_pool_type::error_type{
            .kind = core::transfer_error_kind::invalid_operation, .cause = error});
    }
    [[nodiscard]] static std::expected<void, driver::uart_error>
    mmio_failure() noexcept { return std::unexpected(driver::uart_error::mmio); }

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

} // namespace mcuhw::backend::stm32h753
