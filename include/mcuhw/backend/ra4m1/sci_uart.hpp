#pragma once

#include <cstddef>
#include <cstdint>
#include <expected>
#include <optional>
#include <span>

#include <mcuhw/arch/barrier.hpp>
#include <mcuhw/core/metadata.hpp>
#include <mcuhw/core/transfer.hpp>
#include <mcuhw/driver/uart.hpp>
#include <mcuhw/mcu/ra4m1/description.hpp>

namespace mcuhw::backend::ra4m1 {

namespace sci_detail {

template<std::uint8_t Instance>
struct owner {
    static inline bool claimed{};
};

[[nodiscard]] constexpr mcu::ra4m1::signal rx_signal(
    std::uint8_t instance) noexcept {
    using signal = mcu::ra4m1::signal;
    switch (instance) {
    case 0: return signal::sci0_rxd;
    case 1: return signal::sci1_rxd;
    case 2: return signal::sci2_rxd;
    default: return signal::sci9_rxd;
    }
}

[[nodiscard]] constexpr mcu::ra4m1::signal tx_signal(
    std::uint8_t instance) noexcept {
    using signal = mcu::ra4m1::signal;
    switch (instance) {
    case 0: return signal::sci0_txd;
    case 1: return signal::sci1_txd;
    case 2: return signal::sci2_txd;
    default: return signal::sci9_txd;
    }
}

[[nodiscard]] constexpr mcu::ra4m1::instance_record metadata(
    std::uint8_t instance) noexcept {
    for (const auto& candidate : mcu::ra4m1::instances) {
        if (candidate.id.kind
                == static_cast<std::uint16_t>(
                    mcu::ra4m1::peripheral_kind::sci)
            && candidate.id.instance == instance) {
            return candidate;
        }
    }
    return {};
}

} // namespace sci_detail

template<std::uint8_t Instance, core::pin_id RxPin, core::pin_id TxPin>
struct sci_uart_binding {
    static_assert(Instance == 0U || Instance == 1U || Instance == 2U
                  || Instance == 9U);
    static_assert(core::route_exists(
        mcu::ra4m1::pin_routes, RxPin,
        mcu::ra4m1::signal_id(sci_detail::rx_signal(Instance))));
    static_assert(core::route_exists(
        mcu::ra4m1::pin_routes, TxPin,
        mcu::ra4m1::signal_id(sci_detail::tx_signal(Instance))));

    static constexpr std::uint8_t instance = Instance;
    static constexpr core::pin_id rx_pin = RxPin;
    static constexpr core::pin_id tx_pin = TxPin;
    static constexpr auto metadata = sci_detail::metadata(Instance);
};

// Non-FIFO SCI UART backend. TX publishes completion only from TEI after
// TEND, while RX publishes after the requested finite buffer is full.
template<typename Binding, typename MmioPolicy, typename ExecutionPolicy>
// @req REQ-0014 REQ-0017 REQ-0019 REQ-0027
class sci_uart_backend {
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

    explicit sci_uart_backend(MmioPolicy& mmio,
                              ExecutionPolicy& execution) noexcept
        : mmio_(mmio), tx_operations_(execution), rx_operations_(execution) {}

    [[nodiscard]] std::expected<void, driver::uart_error>
    initialize(driver::uart_baud_timing timing,
               driver::uart_format format) noexcept {
        if (initialized_) {
            return std::unexpected(driver::uart_error::already_owned);
        }
        if (sci_detail::owner<Binding::instance>::claimed) {
            return std::unexpected(driver::uart_error::already_owned);
        }
        if ((format.data_bits != driver::uart_data_bits::seven
             && format.data_bits != driver::uart_data_bits::eight)
            || (format.stop_bits != driver::uart_stop_bits::one
                && format.stop_bits != driver::uart_stop_bits::two)) {
            return std::unexpected(driver::uart_error::invalid_format);
        }
        sci_detail::owner<Binding::instance>::claimed = true;
        owned_ = true;
        if (!release_module_stop()) {
            release_owner();
            return mmio_failure();
        }

        std::uint8_t smr = timing.clock_select;
        if (format.stop_bits == driver::uart_stop_bits::two) {
            smr |= 1U << 3U;
        }
        if (format.parity != driver::uart_parity::none) {
            smr |= 1U << 5U;
            if (format.parity == driver::uart_parity::odd) {
                smr |= 1U << 4U;
            }
        }
        if (format.data_bits == driver::uart_data_bits::seven) {
            smr |= 1U << 6U;
        }
        std::uint8_t semr{};
        if (timing.double_speed) {
            semr |= 1U << 7U;
        }
        if (timing.asynchronous_base_clock_select) {
            semr |= 1U << 4U;
        }
        if (timing.extended_base_clock_select) {
            semr |= 1U << 3U;
        }
        // SCR.RE=0 does not clear RDRF, receive errors, or RDR. Disable the
        // peripheral before routing its pins, then discard state left by a
        // bootloader or an earlier owner while communication is aborted.
        if (!write<std::uint8_t>(base() + 0x02U, 0U)) {
            release_owner();
            return mmio_failure();
        }
        const auto status = mmio_.template read<std::uint8_t>(base() + 0x04U);
        if (!status) {
            release_owner();
            return mmio_failure();
        }
        if ((*status & (1U << 6U)) != 0U
            && !mmio_.template read<std::uint8_t>(base() + 0x05U)) {
            release_owner();
            return mmio_failure();
        }
        const bool configured =
            write<std::uint8_t>(base() + 0x06U, 0xF2U)
            && write<std::uint8_t>(base() + 0x09U, 0U)
            && write<std::uint8_t>(base() + 0x0DU, 0U)
            // Hold TXD in the asynchronous mark state while TE is off. A
            // high-impedance TXD can otherwise look like a start bit when a
            // looped-back receiver is enabled before the first transmission.
            && write<std::uint8_t>(base() + 0x1CU, 0x06U)
            && write<std::uint8_t>(base() + 0x00U, smr)
            && write<std::uint8_t>(base() + 0x01U, timing.brr)
            && write<std::uint8_t>(base() + 0x07U, semr)
            && write<std::uint8_t>(base() + 0x04U,
                                   static_cast<std::uint8_t>(*status & ~0x78U))
            && configure_pins();
        if (!configured) {
            static_cast<void>(restore_pins());
            release_owner();
            return mmio_failure();
        }
        initialized_ = true;
        return {};
    }

    [[nodiscard]] tx_start_result
    transmit(std::span<const std::uint8_t> bytes) noexcept {
        if (!initialized_) {
            return tx_failure(driver::uart_error::not_initialized);
        }
        if (bytes.empty()) {
            return tx_failure(driver::uart_error::empty_transfer);
        }
        if (tx_id_) {
            return tx_failure(driver::uart_error::busy);
        }
        auto started = tx_operations_.try_start(
            {.quiesce = &cancel_tx, .context = this});
        if (!started) {
            return std::unexpected(started.error());
        }
        tx_bytes_ = bytes;
        tx_index_ = 0U;
        tx_id_ = started->id();
        if (!update_scr(scr_te | scr_tie, scr_teie)) {
            const auto id = *tx_id_;
            tx_id_.reset();
            static_cast<void>(tx_operations_.fail(id,
                                                  driver::uart_error::mmio));
        }
        return started;
    }

    [[nodiscard]] rx_start_result
    receive(std::span<std::uint8_t> bytes) noexcept {
        if (!initialized_) {
            return rx_failure(driver::uart_error::not_initialized);
        }
        if (bytes.empty()) {
            return rx_failure(driver::uart_error::empty_transfer);
        }
        if (rx_id_) {
            return rx_failure(driver::uart_error::busy);
        }
        auto started = rx_operations_.try_start(
            {.quiesce = &cancel_rx, .context = this});
        if (!started) {
            return std::unexpected(started.error());
        }
        rx_bytes_ = bytes;
        rx_index_ = 0U;
        rx_id_ = started->id();
        if (!update_scr(scr_re | scr_rie, 0U)) {
            const auto id = *rx_id_;
            rx_id_.reset();
            static_cast<void>(rx_operations_.fail(id,
                                                  driver::uart_error::mmio));
        }
        return started;
    }

    [[nodiscard]] bool handle_transmit_data_empty_interrupt() noexcept {
        if (!tx_id_) {
            return false;
        }
        if (tx_index_ < tx_bytes_.size()
            && !write<std::uint8_t>(base() + 0x03U,
                                    tx_bytes_[tx_index_++])) {
            return fail_tx(driver::uart_error::mmio);
        }
        if (tx_index_ == tx_bytes_.size()) {
            // Stop feeding TDR, then wait for the shift register to finish.
            if (!update_scr(scr_te | scr_teie, scr_tie)) {
                return fail_tx(driver::uart_error::mmio);
            }
        }
        return true;
    }

    [[nodiscard]] bool handle_transmit_end_interrupt() noexcept {
        if (!tx_id_) {
            return false;
        }
        const auto status = mmio_.template read<std::uint8_t>(base() + 0x04U);
        if (!status || (*status & (1U << 2U)) == 0U) {
            return false;
        }
        if (!update_scr(0U, scr_tie | scr_teie | scr_te)) {
            return fail_tx(driver::uart_error::mmio);
        }
        const auto id = *tx_id_;
        tx_id_.reset();
        tx_bytes_ = {};
        return tx_operations_.complete(id);
    }

    [[nodiscard]] bool handle_receive_interrupt() noexcept {
        if (!rx_id_) {
            return false;
        }
        const auto status = mmio_.template read<std::uint8_t>(base() + 0x04U);
        if (!status) {
            return fail_rx(driver::uart_error::mmio);
        }
        if ((*status & 0x38U) != 0U) {
            return handle_receive_error(*status);
        }
        // Selectable ICU lines may retain a pending edge while a peripheral is
        // rebound. Do not consume undefined RDR data unless SCI confirms that
        // a complete character is present.
        if ((*status & (1U << 6U)) == 0U) {
            return false;
        }
        const auto value = mmio_.template read<std::uint8_t>(base() + 0x05U);
        if (!value) {
            return fail_rx(driver::uart_error::mmio);
        }
        rx_bytes_[rx_index_++] = *value;
        if (rx_index_ != rx_bytes_.size()) {
            return true;
        }
        if (!update_scr(0U, scr_rie | scr_re)) {
            return fail_rx(driver::uart_error::mmio);
        }
        const auto id = *rx_id_;
        const auto count = rx_index_;
        rx_id_.reset();
        rx_bytes_ = {};
        return rx_operations_.complete(id, count);
    }

    [[nodiscard]] bool handle_receive_error_interrupt() noexcept {
        const auto status = mmio_.template read<std::uint8_t>(base() + 0x04U);
        return status && handle_receive_error(*status);
    }

    [[nodiscard]] std::expected<void, driver::uart_error> shutdown() noexcept {
        if (!initialized_) {
            return std::unexpected(driver::uart_error::not_initialized);
        }
        if (tx_id_ || rx_id_) {
            return std::unexpected(driver::uart_error::busy);
        }
        if (!write<std::uint8_t>(base() + 0x02U, 0U) || !restore_pins()) {
            return mmio_failure();
        }
        initialized_ = false;
        release_owner();
        return {};
    }

private:
    static constexpr std::uint8_t scr_tie = 1U << 7U;
    static constexpr std::uint8_t scr_rie = 1U << 6U;
    static constexpr std::uint8_t scr_te = 1U << 5U;
    static constexpr std::uint8_t scr_re = 1U << 4U;
    static constexpr std::uint8_t scr_teie = 1U << 2U;

    [[nodiscard]] static constexpr std::uintptr_t base() noexcept {
        return Binding::metadata.base;
    }

    [[nodiscard]] static core::quiesce_result cancel_tx(
        void* context, core::operation_id id) noexcept {
        auto& self = *static_cast<sci_uart_backend*>(context);
        if (!self.tx_id_ || *self.tx_id_ != id) {
            return core::quiesce_result::reusable;
        }
        const bool stopped = self.update_scr(0U, scr_tie | scr_teie | scr_te);
        self.tx_id_.reset();
        self.tx_bytes_ = {};
        return stopped ? core::quiesce_result::reusable
                       : core::quiesce_result::faulted;
    }

    [[nodiscard]] static core::quiesce_result cancel_rx(
        void* context, core::operation_id id) noexcept {
        auto& self = *static_cast<sci_uart_backend*>(context);
        if (!self.rx_id_ || *self.rx_id_ != id) {
            return core::quiesce_result::reusable;
        }
        const bool stopped = self.update_scr(0U, scr_rie | scr_re);
        self.rx_id_.reset();
        self.rx_bytes_ = {};
        return stopped ? core::quiesce_result::reusable
                       : core::quiesce_result::faulted;
    }

    [[nodiscard]] bool handle_receive_error(std::uint8_t status) noexcept {
        driver::uart_error error = driver::uart_error::transport;
        if ((status & (1U << 5U)) != 0U) {
            error = driver::uart_error::overrun;
        } else if ((status & (1U << 4U)) != 0U) {
            error = driver::uart_error::framing;
        } else if ((status & (1U << 3U)) != 0U) {
            error = driver::uart_error::parity;
        }
        if (!write<std::uint8_t>(base() + 0x04U,
                                 static_cast<std::uint8_t>(status & ~0x38U))) {
            error = driver::uart_error::mmio;
        }
        return fail_rx(error);
    }

    [[nodiscard]] bool fail_tx(driver::uart_error error) noexcept {
        if (!tx_id_) {
            return false;
        }
        static_cast<void>(update_scr(0U, scr_tie | scr_teie | scr_te));
        const auto id = *tx_id_;
        tx_id_.reset();
        tx_bytes_ = {};
        return tx_operations_.fail(id, error);
    }

    [[nodiscard]] bool fail_rx(driver::uart_error error) noexcept {
        if (!rx_id_) {
            return false;
        }
        static_cast<void>(update_scr(0U, scr_rie | scr_re));
        const auto id = *rx_id_;
        rx_id_.reset();
        rx_bytes_ = {};
        return rx_operations_.fail(id, error);
    }

    [[nodiscard]] bool update_scr(std::uint8_t set,
                                  std::uint8_t clear) noexcept {
        const auto current = mmio_.template read<std::uint8_t>(base() + 0x02U);
        return current && write<std::uint8_t>(
            base() + 0x02U,
            static_cast<std::uint8_t>((*current | set) & ~clear));
    }

    [[nodiscard]] bool release_module_stop() noexcept {
        constexpr std::uintptr_t prcr = 0x4001'E3FEU;
        const auto current = mmio_.template read<std::uint32_t>(
            Binding::metadata.module_stop_register);
        if (!current || !write<std::uint16_t>(prcr, 0xA502U)) {
            return false;
        }
        const bool enabled = write<std::uint32_t>(
            Binding::metadata.module_stop_register,
            *current & ~Binding::metadata.module_stop_mask);
        const bool locked = write<std::uint16_t>(prcr, 0xA500U);
        arch::data_memory_barrier();
        return enabled && locked;
    }

    [[nodiscard]] bool configure_pins() noexcept {
        return configure_pin(Binding::rx_pin, sci_detail::rx_signal(
            Binding::instance))
            && configure_pin(Binding::tx_pin, sci_detail::tx_signal(
                Binding::instance));
    }

    [[nodiscard]] bool restore_pins() noexcept {
        return restore_pin(Binding::rx_pin) && restore_pin(Binding::tx_pin);
    }

    [[nodiscard]] std::uint32_t psel_for(core::pin_id pin,
                                         mcu::ra4m1::signal signal) const noexcept {
        for (const auto& route : mcu::ra4m1::pin_routes) {
            if (route.pin == pin
                && route.signal == mcu::ra4m1::signal_id(signal)) {
                return route.vendor_encoding;
            }
        }
        return 0U;
    }

    [[nodiscard]] bool configure_pin(core::pin_id pin,
                                     mcu::ra4m1::signal signal) noexcept {
        constexpr std::uintptr_t pwpr = 0x4004'0D03U;
        const auto address = mcu::ra4m1::pfs_address(pin);
        const auto current = mmio_.template read<std::uint32_t>(address);
        if (!current || !write<std::uint8_t>(pwpr, 0U)
            || !write<std::uint8_t>(pwpr, 0x40U)) {
            return false;
        }
        constexpr std::uint32_t controlled =
            (0x1FU << 24U) | (1U << 16U) | (1U << 15U) | (1U << 14U);
        const auto next = (*current & ~controlled)
            | (psel_for(pin, signal) << 24U) | (1U << 16U);
        const bool configured = write<std::uint32_t>(address, next);
        const bool locked = write<std::uint8_t>(pwpr, 0U)
            && write<std::uint8_t>(pwpr, 0x80U);
        arch::data_memory_barrier();
        return configured && locked;
    }

    [[nodiscard]] bool restore_pin(core::pin_id pin) noexcept {
        constexpr std::uintptr_t pwpr = 0x4004'0D03U;
        const auto address = mcu::ra4m1::pfs_address(pin);
        const auto current = mmio_.template read<std::uint32_t>(address);
        if (!current || !write<std::uint8_t>(pwpr, 0U)
            || !write<std::uint8_t>(pwpr, 0x40U)) {
            return false;
        }
        constexpr std::uint32_t controlled =
            (0x1FU << 24U) | (1U << 16U);
        const bool restored = write<std::uint32_t>(address,
                                                   *current & ~controlled);
        const bool locked = write<std::uint8_t>(pwpr, 0U)
            && write<std::uint8_t>(pwpr, 0x80U);
        arch::data_memory_barrier();
        return restored && locked;
    }

    template<typename Value>
    [[nodiscard]] bool write(std::uintptr_t address, Value value) noexcept {
        return mmio_.template write<Value>(address, value).has_value();
    }

    [[nodiscard]] static tx_start_result
    tx_failure(driver::uart_error error) noexcept {
        return std::unexpected(typename tx_pool_type::error_type{
            .kind = core::transfer_error_kind::failure, .cause = error});
    }

    [[nodiscard]] static rx_start_result
    rx_failure(driver::uart_error error) noexcept {
        return std::unexpected(typename rx_pool_type::error_type{
            .kind = core::transfer_error_kind::failure, .cause = error});
    }

    [[nodiscard]] static std::expected<void, driver::uart_error>
    mmio_failure() noexcept {
        return std::unexpected(driver::uart_error::mmio);
    }

    void release_owner() noexcept {
        if (owned_) {
            sci_detail::owner<Binding::instance>::claimed = false;
            owned_ = false;
        }
    }

    MmioPolicy& mmio_;
    tx_pool_type tx_operations_;
    rx_pool_type rx_operations_;
    std::optional<core::operation_id> tx_id_{};
    std::optional<core::operation_id> rx_id_{};
    std::span<const std::uint8_t> tx_bytes_{};
    std::span<std::uint8_t> rx_bytes_{};
    std::size_t tx_index_{};
    std::size_t rx_index_{};
    bool owned_{};
    bool initialized_{};
};

} // namespace mcuhw::backend::ra4m1
