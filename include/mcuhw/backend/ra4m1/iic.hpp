#pragma once

#include <cstddef>
#include <cstdint>
#include <expected>
#include <optional>
#include <span>

#include <mcuhw/arch/barrier.hpp>
#include <mcuhw/core/metadata.hpp>
#include <mcuhw/core/transfer.hpp>
#include <mcuhw/driver/i2c.hpp>
#include <mcuhw/mcu/ra4m1/description.hpp>

namespace mcuhw::backend::ra4m1 {

enum class iic_recovery_policy : std::uint8_t {
    none,
    exclusive_peripheral_reset,
};

namespace iic_detail {

template<std::uint8_t Instance>
struct owner {
    static inline bool claimed{};
};

[[nodiscard]] constexpr mcu::ra4m1::signal scl_signal(
    std::uint8_t instance) noexcept {
    return instance == 0U ? mcu::ra4m1::signal::iic0_scl
                          : mcu::ra4m1::signal::iic1_scl;
}

[[nodiscard]] constexpr mcu::ra4m1::signal sda_signal(
    std::uint8_t instance) noexcept {
    return instance == 0U ? mcu::ra4m1::signal::iic0_sda
                          : mcu::ra4m1::signal::iic1_sda;
}

[[nodiscard]] constexpr mcu::ra4m1::instance_record metadata(
    std::uint8_t instance) noexcept {
    for (const auto& candidate : mcu::ra4m1::instances) {
        if (candidate.id.kind
                == static_cast<std::uint16_t>(
                    mcu::ra4m1::peripheral_kind::iic)
            && candidate.id.instance == instance) {
            return candidate;
        }
    }
    return {};
}

} // namespace iic_detail

template<std::uint8_t Instance, core::pin_id SclPin, core::pin_id SdaPin>
struct iic_binding {
    static_assert(Instance < 2U);
    static_assert(core::route_exists(
        mcu::ra4m1::pin_routes, SclPin,
        mcu::ra4m1::signal_id(iic_detail::scl_signal(Instance))));
    static_assert(core::route_exists(
        mcu::ra4m1::pin_routes, SdaPin,
        mcu::ra4m1::signal_id(iic_detail::sda_signal(Instance))));

    static constexpr std::uint8_t instance = Instance;
    static constexpr core::pin_id scl_pin = SclPin;
    static constexpr core::pin_id sda_pin = SdaPin;
    static constexpr auto metadata = iic_detail::metadata(Instance);
};

// IRQ-driven RA IIC controller. A combined operation stays in one state and
// one operation-pool slot from initial START through repeated START and STOP.
template<typename Binding, typename MmioPolicy, typename ExecutionPolicy>
// @req REQ-0014 REQ-0017 REQ-0019 REQ-0024 REQ-0025
class iic_controller_backend {
public:
    using pool_type = core::operation_pool<driver::i2c_transfer_result,
                                           driver::i2c_error, 1,
                                           ExecutionPolicy>;
    using transfer_type = typename pool_type::transfer_type;
    using start_result = std::expected<transfer_type,
                                       typename pool_type::error_type>;

    explicit iic_controller_backend(MmioPolicy& mmio,
                                    ExecutionPolicy& execution) noexcept
        : mmio_(mmio), operations_(execution) {}

    [[nodiscard]] std::expected<void, driver::i2c_error>
    initialize(driver::i2c_timing timing) noexcept {
        if (initialized_ || iic_detail::owner<Binding::instance>::claimed) {
            return std::unexpected(driver::i2c_error::already_owned);
        }
        iic_detail::owner<Binding::instance>::claimed = true;
        owned_ = true;
        if (!release_module_stop() || !configure_pins()) {
            release_owner();
            return mmio_failure();
        }
        timing_ = timing;
        if (!program_controller()) {
            static_cast<void>(restore_pins());
            release_owner();
            return mmio_failure();
        }
        initialized_ = true;
        return {};
    }

    [[nodiscard]] start_result write(
        driver::i2c_address address,
        std::span<const std::uint8_t> bytes) noexcept {
        if (bytes.empty()) {
            return start_failure(driver::i2c_error::empty_transfer);
        }
        return begin(address, bytes, {}, transaction_kind::write);
    }

    [[nodiscard]] start_result read(driver::i2c_address address,
                                    std::span<std::uint8_t> bytes) noexcept {
        if (bytes.empty()) {
            return start_failure(driver::i2c_error::empty_transfer);
        }
        return begin(address, {}, bytes, transaction_kind::read);
    }

    [[nodiscard]] start_result write_read(
        driver::i2c_address address,
        std::span<const std::uint8_t> write_bytes,
        std::span<std::uint8_t> read_bytes) noexcept {
        if (write_bytes.empty() || read_bytes.empty()) {
            return start_failure(driver::i2c_error::empty_transfer);
        }
        return begin(address, write_bytes, read_bytes,
                     transaction_kind::write_read);
    }

    [[nodiscard]] bool handle_transmit_empty_interrupt() noexcept {
        if (!operation_id_) {
            return false;
        }
        if (!address_written_) {
            const bool reading = kind_ == transaction_kind::read
                || phase_ == phase::read_address;
            if (!write<std::uint8_t>(base() + 0x12U,
                                     static_cast<std::uint8_t>(
                                         address_.value << 1U)
                                         | (reading ? 1U : 0U))) {
                return finish_immediately(driver::i2c_error::mmio);
            }
            address_written_ = true;
            if (reading) {
                phase_ = phase::receive_dummy;
                return write_interrupt_enable(interrupt_errors | irq_rie);
            }
            phase_ = phase::write_data;
            return true;
        }

        address_phase_ = false;
        if (write_index_ < write_bytes_.size()) {
            if (!write<std::uint8_t>(base() + 0x12U,
                                     write_bytes_[write_index_++])) {
                return finish_immediately(driver::i2c_error::mmio);
            }
            if (write_index_ == write_bytes_.size()) {
                phase_ = phase::await_write_end;
                return write_interrupt_enable(interrupt_errors | irq_teie);
            }
        }
        return true;
    }

    [[nodiscard]] bool handle_transmit_end_interrupt() noexcept {
        if (!operation_id_ || phase_ != phase::await_write_end) {
            return false;
        }
        if (kind_ == transaction_kind::write_read) {
            phase_ = phase::read_address;
            address_written_ = false;
            address_phase_ = true;
            // Enable TXI before requesting repeated START. The RA IIC can
            // assert TDRE as the condition is generated; enabling TIE after
            // the request can lose the event on real hardware.
            if (!write_interrupt_enable(interrupt_errors | irq_tie)
                || !request_condition(1U << 2U)) { // repeated START
                return finish_immediately(driver::i2c_error::mmio);
            }
            return true;
        }
        return request_stop();
    }

    [[nodiscard]] bool handle_receive_interrupt() noexcept {
        if (!operation_id_) {
            return false;
        }
        if (phase_ == phase::receive_dummy) {
            if (read_bytes_.size() <= 2U && !set_receive_wait(true)) {
                return finish_immediately(driver::i2c_error::mmio);
            }
            if (read_bytes_.size() == 1U && !set_receive_nack()) {
                return finish_immediately(driver::i2c_error::mmio);
            }
            address_phase_ = false;
            const auto dummy = mmio_.template read<std::uint8_t>(
                base() + 0x13U);
            if (!dummy) {
                return finish_immediately(driver::i2c_error::mmio);
            }
            phase_ = phase::receive_data;
            return true;
        }
        if (phase_ != phase::receive_data
            || read_index_ >= read_bytes_.size()) {
            return false;
        }
        const auto remaining = read_bytes_.size() - read_index_;
        if (remaining == 3U && !set_receive_wait(true)) {
            return finish_immediately(driver::i2c_error::mmio);
        }
        if (remaining == 2U && !set_receive_nack()) {
            return finish_immediately(driver::i2c_error::mmio);
        }
        if (remaining == 1U && !request_stop()) {
            return finish_immediately(driver::i2c_error::mmio);
        }
        const auto value = mmio_.template read<std::uint8_t>(base() + 0x13U);
        if (!value) {
            return finish_immediately(driver::i2c_error::mmio);
        }
        read_bytes_[read_index_++] = *value;
        if (remaining == 1U && !set_receive_wait(false)) {
            return finish_immediately(driver::i2c_error::mmio);
        }
        return true;
    }

    // EEI multiplexes STOP, NACK, arbitration loss and hardware timeout.
    [[nodiscard]] bool handle_error_interrupt() noexcept {
        if (!operation_id_) {
            return false;
        }
        const auto status = mmio_.template read<std::uint8_t>(base() + 0x09U);
        if (!status) {
            return finish_immediately(driver::i2c_error::mmio);
        }
        if ((*status & status_arbitration) != 0U) {
            // Arbitration loss already releases mastership. Never perform a
            // disruptive automatic reset or clock recovery here.
            static_cast<void>(clear_status(status_arbitration));
            return finish_immediately(driver::i2c_error::arbitration_lost);
        }
        if ((*status & status_timeout) != 0U) {
            static_cast<void>(clear_status(status_timeout));
            pending_error_ = driver::i2c_error::timeout;
            return request_stop();
        }
        if ((*status & status_nack) != 0U) {
            pending_error_ = address_phase_
                ? driver::i2c_error::address_nack
                : driver::i2c_error::data_nack;
            static_cast<void>(clear_status(status_nack));
            return request_stop();
        }
        if ((*status & status_stop) != 0U) {
            return finish_stop();
        }
        return false;
    }

    // @req REQ-0025
    [[nodiscard]] std::expected<void, driver::i2c_error>
    recover_bus(iic_recovery_policy policy) noexcept {
        if (!initialized_) {
            return std::unexpected(driver::i2c_error::not_initialized);
        }
        if (operation_id_ || policy == iic_recovery_policy::none) {
            return std::unexpected(driver::i2c_error::busy);
        }
        // This bounded reset only releases this controller's low drive. It is
        // opt-in because it is safe only when composition owns the topology.
        if (!write<std::uint8_t>(base() + 0x00U, 0xC0U)
            || !program_controller()) {
            return mmio_failure();
        }
        const auto control = mmio_.template read<std::uint8_t>(base() + 0x01U);
        if (!control) {
            return mmio_failure();
        }
        if ((*control & 0x80U) != 0U) {
            return std::unexpected(driver::i2c_error::bus_stuck);
        }
        return {};
    }

    [[nodiscard]] std::expected<void, driver::i2c_error> shutdown() noexcept {
        if (!initialized_) {
            return std::unexpected(driver::i2c_error::not_initialized);
        }
        if (operation_id_) {
            return std::unexpected(driver::i2c_error::busy);
        }
        if (!write<std::uint8_t>(base() + 0x07U, 0U)
            || !write<std::uint8_t>(base() + 0x00U, 0U)
            || !restore_pins()) {
            return mmio_failure();
        }
        initialized_ = false;
        release_owner();
        return {};
    }

private:
    enum class transaction_kind : std::uint8_t { write, read, write_read };
    enum class phase : std::uint8_t {
        idle,
        write_address,
        write_data,
        await_write_end,
        read_address,
        receive_dummy,
        receive_data,
        stopping,
    };

    static constexpr std::uint8_t irq_tie = 1U << 7U;
    static constexpr std::uint8_t irq_teie = 1U << 6U;
    static constexpr std::uint8_t irq_rie = 1U << 5U;
    static constexpr std::uint8_t interrupt_errors = 0x1BU;
    static constexpr std::uint8_t status_timeout = 1U << 0U;
    static constexpr std::uint8_t status_arbitration = 1U << 1U;
    static constexpr std::uint8_t status_stop = 1U << 3U;
    static constexpr std::uint8_t status_nack = 1U << 4U;

    [[nodiscard]] static constexpr std::uintptr_t base() noexcept {
        return Binding::metadata.base;
    }

    [[nodiscard]] start_result begin(
        driver::i2c_address address,
        std::span<const std::uint8_t> write_bytes,
        std::span<std::uint8_t> read_bytes,
        transaction_kind kind) noexcept {
        if (!initialized_) {
            return start_failure(driver::i2c_error::not_initialized);
        }
        if (operation_id_) {
            return start_failure(driver::i2c_error::busy);
        }
        if (write_bytes.size() > 65'535U || read_bytes.size() > 65'535U) {
            return start_failure(driver::i2c_error::excessive_length);
        }
        const auto control = mmio_.template read<std::uint8_t>(base() + 0x01U);
        if (!control) {
            return start_failure(driver::i2c_error::mmio);
        }
        if ((*control & 0x80U) != 0U) {
            return start_failure(driver::i2c_error::bus_stuck);
        }
        auto started = operations_.try_start(
            {.quiesce = &cancel_operation, .context = this});
        if (!started) {
            return std::unexpected(started.error());
        }
        operation_id_ = started->id();
        address_ = address;
        write_bytes_ = write_bytes;
        read_bytes_ = read_bytes;
        write_index_ = 0U;
        read_index_ = 0U;
        kind_ = kind;
        phase_ = kind == transaction_kind::read ? phase::read_address
                                                : phase::write_address;
        address_written_ = false;
        address_phase_ = true;
        pending_error_.reset();
        if (!clear_status(0x7FU)
            || !write_interrupt_enable(interrupt_errors | irq_tie)
            || !request_condition(1U << 1U)) { // START
            static_cast<void>(finish_immediately(driver::i2c_error::mmio));
        }
        return started;
    }

    [[nodiscard]] static core::quiesce_result cancel_operation(
        void* context, core::operation_id id) noexcept {
        auto& self = *static_cast<iic_controller_backend*>(context);
        if (!self.operation_id_ || *self.operation_id_ != id) {
            return core::quiesce_result::reusable;
        }
        // operation_pool cannot release the caller's buffers until quiescence
        // is synchronous. Disable IRQs and reset this controller's internal
        // state, which immediately releases its low drive, then restore the
        // validated timing for reuse.
        const bool stopped = self.write_interrupt_enable(0U)
            && self.template write<std::uint8_t>(base() + 0x00U, 0xC0U)
            && self.program_controller();
        self.reset_transaction();
        return stopped ? core::quiesce_result::reusable
                       : core::quiesce_result::faulted;
    }

    [[nodiscard]] bool request_stop() noexcept {
        phase_ = phase::stopping;
        if (!write_interrupt_enable(interrupt_errors)) {
            return false;
        }
        return request_condition(1U << 3U);
    }

    [[nodiscard]] bool request_condition(std::uint8_t bit) noexcept {
        const auto control = mmio_.template read<std::uint8_t>(base() + 0x01U);
        return control && write<std::uint8_t>(
            base() + 0x01U, static_cast<std::uint8_t>(*control | bit));
    }

    [[nodiscard]] bool set_receive_wait(bool enabled) noexcept {
        const auto mode = mmio_.template read<std::uint8_t>(base() + 0x04U);
        return mode && write<std::uint8_t>(
            base() + 0x04U,
            enabled ? static_cast<std::uint8_t>(*mode | (1U << 6U))
                    : static_cast<std::uint8_t>(*mode & ~(1U << 6U)));
    }

    [[nodiscard]] bool set_receive_nack() noexcept {
        const auto mode = mmio_.template read<std::uint8_t>(base() + 0x04U);
        if (!mode) {
            return false;
        }
        // ACKBT is write-protected. ACKWP must already be set before the
        // write that changes ACKBT; setting both bits in one write is ignored.
        const auto write_enabled =
            static_cast<std::uint8_t>(*mode | (1U << 4U));
        const auto nack =
            static_cast<std::uint8_t>(write_enabled | (1U << 3U));
        return write<std::uint8_t>(base() + 0x04U, write_enabled)
            && write<std::uint8_t>(base() + 0x04U, nack)
            && write<std::uint8_t>(base() + 0x04U,
                                   static_cast<std::uint8_t>(nack
                                                             & ~(1U << 4U)));
    }

    [[nodiscard]] bool finish_stop() noexcept {
        if (!clear_status(static_cast<std::uint8_t>(status_stop | status_nack))
            || !write_interrupt_enable(0U)) {
            return finish_immediately(driver::i2c_error::mmio);
        }
        if (pending_error_) {
            const auto error = *pending_error_;
            return finish_immediately(error);
        }
        const auto id = *operation_id_;
        const driver::i2c_transfer_result result{
            .bytes_written = write_index_, .bytes_read = read_index_};
        reset_transaction();
        return operations_.complete(id, result);
    }

    [[nodiscard]] bool finish_immediately(driver::i2c_error error) noexcept {
        if (!operation_id_) {
            return false;
        }
        static_cast<void>(write_interrupt_enable(0U));
        const auto id = *operation_id_;
        reset_transaction();
        return operations_.fail(id, error);
    }

    void reset_transaction() noexcept {
        operation_id_.reset();
        write_bytes_ = {};
        read_bytes_ = {};
        write_index_ = 0U;
        read_index_ = 0U;
        phase_ = phase::idle;
        address_written_ = false;
        address_phase_ = false;
        pending_error_.reset();
    }

    [[nodiscard]] bool program_controller() noexcept {
        // Internal reset remains asserted while timing and features are set.
        return write<std::uint8_t>(base() + 0x00U, 0xC0U)
            && write<std::uint8_t>(base() + 0x02U,
                                   static_cast<std::uint8_t>(
                                       (timing_.clock_select << 4U) | 0x08U))
            && write<std::uint8_t>(base() + 0x03U, 0x06U)
            && write<std::uint8_t>(base() + 0x04U, 0U)
            // SCLE, NACKE, MALE and TMOE.
            && write<std::uint8_t>(base() + 0x05U, 0x53U)
            && write<std::uint8_t>(base() + 0x06U, 0U)
            && write<std::uint8_t>(base() + 0x07U, 0U)
            && write<std::uint8_t>(base() + 0x10U,
                                   static_cast<std::uint8_t>(0xE0U
                                                               | timing_.brl))
            && write<std::uint8_t>(base() + 0x11U,
                                   static_cast<std::uint8_t>(0xE0U
                                                               | timing_.brh))
            && write<std::uint8_t>(base() + 0x00U, 0x80U);
    }

    [[nodiscard]] bool clear_status(std::uint8_t mask) noexcept {
        const auto status = mmio_.template read<std::uint8_t>(base() + 0x09U);
        return status && write<std::uint8_t>(
            base() + 0x09U, static_cast<std::uint8_t>(*status & ~mask));
    }

    [[nodiscard]] bool write_interrupt_enable(std::uint8_t value) noexcept {
        return write<std::uint8_t>(base() + 0x07U, value);
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
        return configure_pin(Binding::scl_pin,
                             iic_detail::scl_signal(Binding::instance))
            && configure_pin(Binding::sda_pin,
                             iic_detail::sda_signal(Binding::instance));
    }

    [[nodiscard]] bool restore_pins() noexcept {
        return restore_pin(Binding::scl_pin) && restore_pin(Binding::sda_pin);
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
            (0x1FU << 24U) | (1U << 16U) | (1U << 6U);
        const auto next = (*current & ~controlled)
            | (psel_for(pin, signal) << 24U) | (1U << 16U)
            | (1U << 6U);
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
            (0x1FU << 24U) | (1U << 16U) | (1U << 6U);
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

    [[nodiscard]] static start_result
    start_failure(driver::i2c_error error) noexcept {
        return std::unexpected(typename pool_type::error_type{
            .kind = core::transfer_error_kind::failure, .cause = error});
    }

    [[nodiscard]] static std::expected<void, driver::i2c_error>
    mmio_failure() noexcept {
        return std::unexpected(driver::i2c_error::mmio);
    }

    void release_owner() noexcept {
        if (owned_) {
            iic_detail::owner<Binding::instance>::claimed = false;
            owned_ = false;
        }
    }

    MmioPolicy& mmio_;
    pool_type operations_;
    driver::i2c_timing timing_{};
    driver::i2c_address address_{};
    std::span<const std::uint8_t> write_bytes_{};
    std::span<std::uint8_t> read_bytes_{};
    std::optional<core::operation_id> operation_id_{};
    std::optional<driver::i2c_error> pending_error_{};
    std::size_t write_index_{};
    std::size_t read_index_{};
    transaction_kind kind_{};
    phase phase_{phase::idle};
    bool address_written_{};
    bool address_phase_{};
    bool owned_{};
    bool initialized_{};
};

} // namespace mcuhw::backend::ra4m1
