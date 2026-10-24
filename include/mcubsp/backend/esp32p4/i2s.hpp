#pragma once

#include <cstddef>
#include <cstdint>
#include <expected>
#include <optional>
#include <span>

#include <mcubsp/core/transfer.hpp>
#include <mcubsp/driver/i2s.hpp>
#include <mcubsp/mcu/esp32p4/description.hpp>

namespace mcubsp::backend::esp32p4 {

namespace detail {

template<std::uint8_t Instance>
struct i2s_owner { static inline bool claimed{}; };

template<std::uint8_t Channel>
struct ahb_dma_owner { static inline bool claimed{}; };

[[nodiscard]] constexpr std::uint32_t absolute_difference(
    std::uint32_t left, std::uint32_t right) noexcept {
    return left > right ? left - right : right - left;
}

} // namespace detail

// ESP32-P4 I2S uses source/(N + numerator/denominator), with N in 1..255
// and a fractional denominator no larger than 512. The solver keeps the
// MCLK/BCLK relationship integral and minimizes sample-rate error.
// @req REQ-0011 REQ-0034
[[nodiscard]] constexpr std::expected<driver::i2s_timing, driver::i2s_error>
solve_i2s_timing(driver::i2s_timing_request request) noexcept {
    if (request.clock_hz == 0U) {
        return std::unexpected(driver::i2s_error::invalid_clock);
    }
    if (request.sample_rate_hz == 0U || request.mclk_multiple == 0U
        || request.slot_bits != 16U || request.slot_count != 2U) {
        return std::unexpected(driver::i2s_error::unsupported_format);
    }
    const auto frame_bits = static_cast<std::uint32_t>(request.slot_bits)
        * request.slot_count;
    if (request.mclk_multiple % frame_bits != 0U
        || request.mclk_multiple / frame_bits > 64U) {
        return std::unexpected(driver::i2s_error::invalid_rate);
    }
    const auto desired_mclk = static_cast<std::uint64_t>(
        request.sample_rate_hz) * request.mclk_multiple;
    if (desired_mclk == 0U || desired_mclk > request.clock_hz) {
        return std::unexpected(driver::i2s_error::invalid_rate);
    }

    bool found{};
    driver::i2s_timing best{};
    for (std::uint16_t denominator = 1U; denominator <= 512U;
         ++denominator) {
        const auto scaled = (static_cast<std::uint64_t>(request.clock_hz)
                             * denominator + desired_mclk / 2U)
            / desired_mclk;
        auto integer = scaled / denominator;
        auto numerator = scaled % denominator;
        if (numerator == denominator) {
            ++integer;
            numerator = 0U;
        }
        if (integer < 1U || integer > 255U) continue;
        const auto divisor = integer * denominator + numerator;
        const auto achieved_mclk = static_cast<std::uint32_t>(
            (static_cast<std::uint64_t>(request.clock_hz) * denominator
             + divisor / 2U) / divisor);
        const auto achieved_rate = static_cast<std::uint32_t>(
            (static_cast<std::uint64_t>(achieved_mclk)
             + request.mclk_multiple / 2U) / request.mclk_multiple);
        const auto delta = detail::absolute_difference(
            achieved_rate, request.sample_rate_hz);
        const auto error = static_cast<std::uint32_t>(
            (static_cast<std::uint64_t>(delta) * 1'000'000ULL
             + request.sample_rate_hz / 2U) / request.sample_rate_hz);
        const driver::i2s_timing candidate{
            .clock_hz = request.clock_hz,
            .sample_rate_hz = request.sample_rate_hz,
            .achieved_sample_rate_hz = achieved_rate,
            .mclk_hz = achieved_mclk,
            .bclk_hz = achieved_rate * frame_bits,
            .error_ppm = error,
            .divider_numerator = static_cast<std::uint16_t>(numerator),
            .divider_denominator = denominator,
            .divider_integer = static_cast<std::uint8_t>(integer),
            .bclk_divider = static_cast<std::uint8_t>(
                request.mclk_multiple / frame_bits),
            .slot_bits = request.slot_bits,
            .slot_count = request.slot_count,
        };
        if (!found || candidate.error_ppm < best.error_ppm
            || (candidate.error_ppm == best.error_ppm
                && candidate.divider_denominator
                       < best.divider_denominator)) {
            best = candidate;
            found = true;
        }
    }
    if (!found || best.error_ppm > request.maximum_error_ppm) {
        return std::unexpected(driver::i2s_error::invalid_rate);
    }
    return best;
}

template<std::uint8_t Instance, core::pin_id MasterClockPin,
         core::pin_id BitClockPin, core::pin_id WordSelectPin,
         core::pin_id DataOutPin, core::pin_id DataInPin,
         std::uint8_t DmaChannel = 0U>
struct i2s_binding {
    static_assert(Instance == 0U,
                  "source-backed initial ESP32-P4 backend covers I2S0");
    static_assert(DmaChannel < 3U, "ESP32-P4 AHB DMA has three pairs");
    static_assert(mcu::esp32p4::qfn104_pin_database::template valid<
        MasterClockPin, mcu::esp32p4::signal_id(
                            mcu::esp32p4::signal::i2s0_mclk),
        core::electrical_mode::push_pull>);
    static_assert(mcu::esp32p4::qfn104_pin_database::template valid<
        BitClockPin, mcu::esp32p4::signal_id(
                         mcu::esp32p4::signal::i2s0_bclk),
        core::electrical_mode::push_pull>);
    static_assert(mcu::esp32p4::qfn104_pin_database::template valid<
        WordSelectPin, mcu::esp32p4::signal_id(
                          mcu::esp32p4::signal::i2s0_ws),
        core::electrical_mode::push_pull>);
    static_assert(mcu::esp32p4::qfn104_pin_database::template valid<
        DataOutPin, mcu::esp32p4::signal_id(
                        mcu::esp32p4::signal::i2s0_data_out),
        core::electrical_mode::push_pull>);
    static_assert(mcu::esp32p4::qfn104_pin_database::template valid<
        DataInPin, mcu::esp32p4::signal_id(
                       mcu::esp32p4::signal::i2s0_data_in),
        core::electrical_mode::input>);

    static constexpr std::uint8_t instance = Instance;
    static constexpr std::uint8_t dma_channel = DmaChannel;
    static constexpr core::pin_id mclk_pin = MasterClockPin;
    static constexpr core::pin_id bclk_pin = BitClockPin;
    static constexpr core::pin_id ws_pin = WordSelectPin;
    static constexpr core::pin_id data_out_pin = DataOutPin;
    static constexpr core::pin_id data_in_pin = DataInPin;
    static constexpr std::uintptr_t base = mcu::esp32p4::i2s0_base;
    static constexpr std::uintptr_t dma_base = mcu::esp32p4::gdma_ahb_base;
    static constexpr std::uint16_t tx_bclk_matrix_signal = 25U;
    static constexpr std::uint16_t mclk_matrix_signal = 26U;
    static constexpr std::uint16_t tx_ws_matrix_signal = 27U;
    static constexpr std::uint16_t data_matrix_signal = 28U;
    static constexpr std::uint16_t rx_bclk_matrix_signal = 29U;
    static constexpr std::uint16_t rx_ws_matrix_signal = 30U;
    static constexpr std::uint8_t dma_peripheral = 3U;
};

// MemoryPolicy makes cache ownership explicit. It must supply address(ptr),
// prepare_for_device(ptr,size), and complete_from_device(ptr,size). No hidden
// allocation or cache operation occurs in this backend.
template<typename Binding, typename MmioPolicy, typename ExecutionPolicy,
         typename MemoryPolicy>
// @req REQ-0003 REQ-0009 REQ-0011 REQ-0013 REQ-0014 REQ-0017 REQ-0019 REQ-0034
class i2s_backend {
public:
    using pool_type = core::operation_pool<driver::i2s_transfer_result,
                                           driver::i2s_error, 1,
                                           ExecutionPolicy>;
    using transfer_type = typename pool_type::transfer_type;
    using start_result = std::expected<transfer_type,
                                       typename pool_type::error_type>;

    i2s_backend(MmioPolicy& mmio, ExecutionPolicy& execution,
                MemoryPolicy& memory) noexcept
        : mmio_(mmio), operations_(execution), memory_(memory) {}

    [[nodiscard]] std::expected<void, driver::i2s_error> initialize(
        driver::i2s_timing timing) noexcept {
        if (initialized_ || detail::i2s_owner<Binding::instance>::claimed
            || detail::ahb_dma_owner<Binding::dma_channel>::claimed) {
            return std::unexpected(driver::i2s_error::already_owned);
        }
        if (timing.clock_hz != 40'000'000U || timing.slot_bits != 16U
            || timing.slot_count != 2U || timing.bclk_divider == 0U
            || timing.divider_integer == 0U
            || timing.divider_denominator == 0U) {
            return std::unexpected(driver::i2s_error::unsupported_format);
        }
        detail::i2s_owner<Binding::instance>::claimed = true;
        detail::ahb_dma_owner<Binding::dma_channel>::claimed = true;
        owned_ = true;
        timing_ = timing;

        if (!enable_and_reset_dma()
            || !modify(hp_soc_clock_ctrl2, i2s0_apb_gate, 0U)
            || !modify(lp_hp_clock_control, i2s0_mclk_pad_gate, 0U)
            || !pulse_reset() || !configure_clocks(timing)
            || !configure_output_pin(Binding::mclk_pin,
                                     Binding::mclk_matrix_signal)
            || !configure_output_pin(Binding::bclk_pin,
                                     Binding::tx_bclk_matrix_signal)
            || !configure_output_pin(Binding::ws_pin,
                                     Binding::tx_ws_matrix_signal)
            || !configure_output_pin(Binding::data_out_pin,
                                     Binding::data_matrix_signal)
            || !configure_input_pin(Binding::data_in_pin,
                                    Binding::data_matrix_signal)
            || !configure_i2s(timing) || !configure_dma()) {
            return initialization_failed();
        }
        initialized_ = true;
        return {};
    }

    [[nodiscard]] start_result transmit(
        std::span<const std::int16_t> samples) noexcept {
        return start(samples.data(), samples.size_bytes(),
                     driver::i2s_direction::transmit);
    }

    [[nodiscard]] start_result receive(
        std::span<std::int16_t> samples) noexcept {
        return start(samples.data(), samples.size_bytes(),
                     driver::i2s_direction::receive);
    }

    [[nodiscard]] bool handle_interrupt() noexcept {
        if (!id_) return false;
        const auto raw = read_absolute(direction_ == driver::i2s_direction::receive
                                           ? dma_rx_interrupt_raw()
                                           : dma_tx_interrupt_raw());
        if (!raw) return finish(driver::i2s_error::mmio);
        const auto errors = direction_ == driver::i2s_direction::receive
            ? rx_error_interrupts : tx_error_interrupts;
        if ((*raw & errors) != 0U) {
            return finish(driver::i2s_error::transport);
        }
        const auto complete = direction_ == driver::i2s_direction::receive
            ? rx_complete_interrupt : tx_complete_interrupt;
        if ((*raw & complete) == 0U) return false;

        const auto current = *id_;
        const auto direction = direction_;
        const auto bytes = bytes_;
        bool stopped = quiesce(direction);
        if (stopped && direction == driver::i2s_direction::receive) {
            stopped = memory_.complete_from_device(buffer_, bytes);
        }
        reset_state();
        return stopped
            ? operations_.complete(current, {direction, bytes})
            : operations_.fail(current, driver::i2s_error::mmio);
    }

    [[nodiscard]] std::expected<void, driver::i2s_error> shutdown() noexcept {
        if (!initialized_) {
            return std::unexpected(driver::i2s_error::not_initialized);
        }
        if (id_) return std::unexpected(driver::i2s_error::busy);
        const bool restored = restore_input_pin(Binding::data_in_pin,
                                                Binding::data_matrix_signal)
            && restore_output_pin(Binding::data_out_pin)
            && restore_output_pin(Binding::ws_pin)
            && restore_output_pin(Binding::bclk_pin)
            && restore_output_pin(Binding::mclk_pin)
            && write_absolute(dma_rx_peripheral(), disconnected_peripheral)
            && write_absolute(dma_tx_peripheral(), disconnected_peripheral)
            && modify(lp_hp_clock_control, 0U, i2s0_mclk_pad_gate)
            && modify(hp_soc_clock_ctrl2, 0U, i2s0_apb_gate);
        initialized_ = false;
        release_owner();
        return restored ? std::expected<void, driver::i2s_error>{}
                        : std::unexpected(driver::i2s_error::mmio);
    }

private:
    struct alignas(4) dma_descriptor {
        std::uint32_t control{};
        std::uint32_t buffer{};
        std::uint32_t next{};
    };
    static_assert(sizeof(dma_descriptor) == 12U);

    [[nodiscard]] start_result start(const void* buffer, std::size_t bytes,
                                     driver::i2s_direction direction) noexcept {
        if (!initialized_) return failure(driver::i2s_error::not_initialized);
        if (id_) return failure(driver::i2s_error::busy);
        if (bytes == 0U) return failure(driver::i2s_error::empty_transfer);
        if (bytes > descriptor_maximum_bytes || (bytes % 4U) != 0U) {
            return failure(bytes > descriptor_maximum_bytes
                               ? driver::i2s_error::excessive_length
                               : driver::i2s_error::invalid_buffer);
        }
        const auto buffer_address = memory_.address(buffer);
        const auto descriptor_address = memory_.address(&descriptor_);
        if (!buffer_address || !descriptor_address) {
            return failure(driver::i2s_error::invalid_buffer);
        }

        auto started = operations_.try_start({.quiesce = &cancel,
                                              .context = this});
        if (!started) return std::unexpected(started.error());
        id_ = started->id();
        direction_ = direction;
        buffer_ = const_cast<void*>(buffer);
        bytes_ = bytes;
        descriptor_ = {
            .control = static_cast<std::uint32_t>(bytes)
                | (static_cast<std::uint32_t>(bytes) << 12U)
                | (1U << 30U) | (1U << 31U),
            .buffer = *buffer_address,
            .next = 0U,
        };

        // RX needs cache preparation too: dirty lines must not be written
        // back over samples after the device has filled the buffer.
        bool prepared = memory_.prepare_for_device(buffer, bytes);
        prepared = prepared
            && memory_.prepare_for_device(&descriptor_, sizeof(descriptor_))
            && route_serial_clocks(direction);
        if (direction == driver::i2s_direction::receive) {
            prepared = prepared && write(reg_rx_eof_num,
                                         static_cast<std::uint32_t>(bytes));
        }
        const auto raw = direction == driver::i2s_direction::receive
            ? dma_rx_interrupt_raw() : dma_tx_interrupt_raw();
        const auto clear = direction == driver::i2s_direction::receive
            ? dma_rx_interrupt_clear() : dma_tx_interrupt_clear();
        const auto configuration = direction == driver::i2s_direction::receive
            ? dma_rx_configuration() : dma_tx_configuration();
        const auto link = direction == driver::i2s_direction::receive
            ? dma_rx_link() : dma_tx_link();
        const auto link_address = direction == driver::i2s_direction::receive
            ? dma_rx_link_address() : dma_tx_link_address();
        prepared = prepared && write_absolute(clear, all_dma_interrupts)
            && write_absolute(configuration, 1U)
            && write_absolute(configuration, 0U)
            && write_absolute(link_address, *descriptor_address)
            && write_absolute(link, direction == driver::i2s_direction::receive
                                        ? 1U << 2U : 1U << 1U);
        static_cast<void>(raw);
        if (prepared) {
            const auto conf = direction == driver::i2s_direction::receive
                ? reg_rx_configuration : reg_tx_configuration;
            prepared = reset_fifo(conf) && synchronize(conf)
                && modify(Binding::base + conf, start_bit, 0U);
        }
        if (!prepared) static_cast<void>(finish(driver::i2s_error::mmio));
        return started;
    }

    [[nodiscard]] bool configure_clocks(driver::i2s_timing timing) noexcept {
        std::uint32_t x{};
        std::uint32_t y{};
        std::uint32_t z{};
        std::uint32_t yn1{};
        if (timing.divider_numerator != 0U) {
            yn1 = timing.divider_numerator * 2U
                    > timing.divider_denominator;
            z = yn1 ? timing.divider_denominator
                          - timing.divider_numerator
                    : timing.divider_numerator;
            x = timing.divider_denominator / z - 1U;
            y = timing.divider_denominator % z;
        }
        // Preserve the documented sequencing workaround: select a small
        // integer divisor before updating the fractional coefficients.
        return modify(hp_peripheral_clock12, 2U, 0xFFU)
            && modify(hp_peripheral_clock13, 2U << 13U,
                      (0xFFU << 13U) | (0x1FFU << 21U)
                          | 0x3FFU)
            && modify(hp_peripheral_clock14, 1U,
                      0x000F'FFFFU)
            && modify(hp_peripheral_clock12,
                      timing.divider_integer | (x << 8U) | (y << 17U),
                      0x03FF'FFFFU)
            && modify(hp_peripheral_clock13,
                      (z & 0x1FFU) | (yn1 << 9U) | (1U << 10U)
                          | (static_cast<std::uint32_t>(
                                 timing.divider_integer)
                             << 13U)
                          | (x << 21U),
                      0x7FFF'FFFFU)
            && modify(hp_peripheral_clock14,
                      y | (z << 9U) | (yn1 << 18U) | (1U << 19U),
                      0x000F'FFFFU)
            && modify(hp_peripheral_clock11, 1U << 24U,
                      0x3U << 25U);
    }

    [[nodiscard]] bool configure_i2s(driver::i2s_timing timing) noexcept {
        const auto bits = static_cast<std::uint32_t>(timing.slot_bits - 1U);
        const auto conf1 = bits | (bits << 14U) | (bits << 19U)
            | (bits << 27U);
        const auto common = pcm_bypass | msb_shift | left_align | tdm_enable
            | (static_cast<std::uint32_t>(timing.bclk_divider - 1U) << 21U);
        return write(reg_interrupt_enable, 0U)
            && write(reg_interrupt_clear, 0xFU)
            && write(reg_clock_gate, 1U)
            && write(reg_rx_configuration, common)
            && write(reg_tx_configuration, common | tx_stop_when_empty)
            && write(reg_rx_configuration1, conf1)
            && write(reg_tx_configuration1, conf1)
            && write(reg_rx_pdm_configuration, 0U)
            && write(reg_tx_pdm_configuration, 0U)
            && write(reg_rx_tdm_control, 0x0001'0003U)
            && write(reg_tx_tdm_control, 0x0001'0003U)
            && reset_fifo(reg_rx_configuration)
            && reset_fifo(reg_tx_configuration)
            && synchronize(reg_rx_configuration)
            && synchronize(reg_tx_configuration);
    }

    [[nodiscard]] bool route_serial_clocks(
        driver::i2s_direction direction) noexcept {
        const auto bclk = direction == driver::i2s_direction::receive
            ? Binding::rx_bclk_matrix_signal
            : Binding::tx_bclk_matrix_signal;
        const auto ws = direction == driver::i2s_direction::receive
            ? Binding::rx_ws_matrix_signal
            : Binding::tx_ws_matrix_signal;
        return configure_output_pin(Binding::bclk_pin, bclk)
            && configure_output_pin(Binding::ws_pin, ws);
    }

    [[nodiscard]] bool configure_dma() noexcept {
        return modify(Binding::dma_base + dma_misc_configuration,
                      1U << 3U, 0U)
            && write_absolute(Binding::dma_base + dma_internal_memory_start,
                              0x4000'0000U)
            && write_absolute(Binding::dma_base + dma_internal_memory_end,
                              0x4FFC'0000U)
            && write_absolute(dma_rx_interrupt_clear(), all_dma_interrupts)
            && write_absolute(dma_tx_interrupt_clear(), all_dma_interrupts)
            && write_absolute(dma_rx_interrupt_enable(), 0U)
            && write_absolute(dma_tx_interrupt_enable(), 0U)
            && write_absolute(dma_rx_peripheral(), Binding::dma_peripheral)
            && write_absolute(dma_tx_peripheral(), Binding::dma_peripheral)
            && write_absolute(dma_rx_configuration1(), 1U << 12U)
            && write_absolute(dma_tx_configuration1(), 1U << 12U);
    }

    [[nodiscard]] bool enable_and_reset_dma() noexcept {
        // AHB PDMA has a system-bus gate in addition to the local register
        // clock bit. Match Espressif's GDMA group initialization order:
        // enable the bus clock, then pulse the module-wide reset before any
        // channel registers are programmed.
        return modify(hp_soc_clock_ctrl1, ahb_pdma_system_gate, 0U)
            && modify(hp_rst_en1, ahb_pdma_reset, 0U)
            && modify(hp_rst_en1, 0U, ahb_pdma_reset);
    }

    [[nodiscard]] bool synchronize(std::uintptr_t configuration) noexcept {
        if (!modify(Binding::base + configuration, update_bit, 0U)) return false;
        for (std::size_t attempt = 0U; attempt < synchronization_limit;
             ++attempt) {
            const auto value = read(configuration);
            if (!value) return false;
            if ((*value & update_bit) == 0U) return true;
        }
        return false;
    }

    [[nodiscard]] bool reset_fifo(std::uintptr_t configuration) noexcept {
        return modify(Binding::base + configuration, fifo_reset_bit, 0U)
            && modify(Binding::base + configuration, 0U, fifo_reset_bit);
    }

    [[nodiscard]] bool pulse_reset() noexcept {
        return modify(hp_rst_en2, i2s0_reset, 0U)
            && modify(hp_rst_en2, 0U, i2s0_reset);
    }

    [[nodiscard]] bool configure_output_pin(core::pin_id pin,
                                            std::uint16_t signal) noexcept {
        const auto pad = pad_address(pin);
        const auto output = output_select_address(pin);
        const auto current_pad = read_absolute(pad);
        const auto current_output = read_absolute(output);
        if (!current_pad || !current_output) return false;
        constexpr std::uint32_t pad_controlled = (1U << 9U)
            | (0x3U << 10U) | (0x7U << 12U);
        return write_absolute(output,
                              (*current_output & ~0x7FFU) | signal)
            && write_absolute(pad, (*current_pad & ~pad_controlled)
                                      | (2U << 10U) | (1U << 12U));
    }

    [[nodiscard]] bool configure_input_pin(core::pin_id pin,
                                           std::uint16_t signal) noexcept {
        const auto pad = pad_address(pin);
        const auto input = input_select_address(signal);
        const auto current_pad = read_absolute(pad);
        const auto current_input = read_absolute(input);
        if (!current_pad || !current_input) return false;
        constexpr std::uint32_t pad_controlled = (1U << 9U)
            | (0x3U << 10U) | (0x7U << 12U);
        return write_absolute(input,
                              (*current_input & ~0xFFU) | pin.index
                                  | (1U << 7U))
            && write_absolute(pad, (*current_pad & ~pad_controlled)
                                      | (1U << 9U) | (2U << 10U)
                                      | (1U << 12U));
    }

    [[nodiscard]] bool restore_output_pin(core::pin_id pin) noexcept {
        const auto pad = pad_address(pin);
        const auto output = output_select_address(pin);
        const auto current_pad = read_absolute(pad);
        const auto current_output = read_absolute(output);
        return current_pad && current_output
            && write_absolute(output,
                              (*current_output & ~0x7FFU) | 256U
                                  | (1U << 10U))
            && write_absolute(pad, (*current_pad & ~(0x7U << 12U))
                                      | (1U << 9U) | (1U << 12U));
    }

    [[nodiscard]] bool restore_input_pin(core::pin_id pin,
                                         std::uint16_t signal) noexcept {
        const auto pad = pad_address(pin);
        const auto input = input_select_address(signal);
        const auto current_pad = read_absolute(pad);
        const auto current_input = read_absolute(input);
        return current_pad && current_input
            && write_absolute(input,
                              (*current_input & ~0xFFU) | 0x3EU
                                  | (1U << 7U))
            && write_absolute(pad, (*current_pad & ~(0x7U << 12U))
                                      | (1U << 9U) | (1U << 12U));
    }

    [[nodiscard]] static core::quiesce_result cancel(
        void* context, core::operation_id id) noexcept {
        auto& self = *static_cast<i2s_backend*>(context);
        if (!self.id_ || *self.id_ != id) {
            return core::quiesce_result::reusable;
        }
        const bool stopped = self.quiesce(self.direction_);
        self.reset_state();
        return stopped ? core::quiesce_result::reusable
                       : core::quiesce_result::faulted;
    }

    [[nodiscard]] bool quiesce(driver::i2s_direction direction) noexcept {
        const auto conf = direction == driver::i2s_direction::receive
            ? reg_rx_configuration : reg_tx_configuration;
        const auto link = direction == driver::i2s_direction::receive
            ? dma_rx_link() : dma_tx_link();
        const auto clear = direction == driver::i2s_direction::receive
            ? dma_rx_interrupt_clear() : dma_tx_interrupt_clear();
        return modify(Binding::base + conf, 0U, start_bit)
            && write_absolute(link, direction == driver::i2s_direction::receive
                                       ? 1U << 1U : 1U)
            && write_absolute(clear, all_dma_interrupts)
            && reset_fifo(conf);
    }

    [[nodiscard]] bool finish(driver::i2s_error error) noexcept {
        if (!id_) return false;
        const bool stopped = quiesce(direction_);
        const auto current = *id_;
        reset_state();
        return operations_.fail(current, stopped ? error
                                                 : driver::i2s_error::mmio);
    }

    [[nodiscard]] std::expected<void, driver::i2s_error>
    initialization_failed() noexcept {
        static_cast<void>(restore_input_pin(Binding::data_in_pin,
                                            Binding::data_matrix_signal));
        static_cast<void>(restore_output_pin(Binding::data_out_pin));
        static_cast<void>(restore_output_pin(Binding::ws_pin));
        static_cast<void>(restore_output_pin(Binding::bclk_pin));
        static_cast<void>(restore_output_pin(Binding::mclk_pin));
        release_owner();
        return std::unexpected(driver::i2s_error::mmio);
    }

    [[nodiscard]] start_result failure(driver::i2s_error error) noexcept {
        return std::unexpected(typename pool_type::error_type{
            .kind = core::transfer_error_kind::invalid_operation,
            .cause = error});
    }

    void reset_state() noexcept {
        id_.reset();
        buffer_ = nullptr;
        bytes_ = 0U;
    }

    void release_owner() noexcept {
        if (owned_) {
            detail::i2s_owner<Binding::instance>::claimed = false;
            detail::ahb_dma_owner<Binding::dma_channel>::claimed = false;
            owned_ = false;
        }
    }

    [[nodiscard]] static constexpr std::uintptr_t dma_channel_base() noexcept {
        return Binding::dma_base + 0x70U
            + static_cast<std::uintptr_t>(Binding::dma_channel) * 0xC0U;
    }
    [[nodiscard]] static constexpr std::uintptr_t dma_rx_interrupt_raw() noexcept {
        return Binding::dma_base
            + static_cast<std::uintptr_t>(Binding::dma_channel) * 0x10U;
    }
    [[nodiscard]] static constexpr std::uintptr_t dma_tx_interrupt_raw() noexcept {
        return Binding::dma_base + 0x30U
            + static_cast<std::uintptr_t>(Binding::dma_channel) * 0x10U;
    }
    [[nodiscard]] static constexpr std::uintptr_t dma_rx_interrupt_enable() noexcept { return dma_rx_interrupt_raw() + 0x08U; }
    [[nodiscard]] static constexpr std::uintptr_t dma_tx_interrupt_enable() noexcept { return dma_tx_interrupt_raw() + 0x08U; }
    [[nodiscard]] static constexpr std::uintptr_t dma_rx_interrupt_clear() noexcept { return dma_rx_interrupt_raw() + 0x0CU; }
    [[nodiscard]] static constexpr std::uintptr_t dma_tx_interrupt_clear() noexcept { return dma_tx_interrupt_raw() + 0x0CU; }
    [[nodiscard]] static constexpr std::uintptr_t dma_rx_configuration() noexcept { return dma_channel_base(); }
    [[nodiscard]] static constexpr std::uintptr_t dma_rx_configuration1() noexcept { return dma_channel_base() + 0x04U; }
    [[nodiscard]] static constexpr std::uintptr_t dma_rx_link() noexcept { return dma_channel_base() + 0x10U; }
    [[nodiscard]] static constexpr std::uintptr_t dma_rx_peripheral() noexcept { return dma_channel_base() + 0x30U; }
    [[nodiscard]] static constexpr std::uintptr_t dma_tx_configuration() noexcept { return dma_channel_base() + 0x60U; }
    [[nodiscard]] static constexpr std::uintptr_t dma_tx_configuration1() noexcept { return dma_channel_base() + 0x64U; }
    [[nodiscard]] static constexpr std::uintptr_t dma_tx_link() noexcept { return dma_channel_base() + 0x70U; }
    [[nodiscard]] static constexpr std::uintptr_t dma_tx_peripheral() noexcept { return dma_channel_base() + 0x90U; }
    [[nodiscard]] static constexpr std::uintptr_t dma_rx_link_address() noexcept { return Binding::dma_base + 0x3ACU + static_cast<std::uintptr_t>(Binding::dma_channel) * 4U; }
    [[nodiscard]] static constexpr std::uintptr_t dma_tx_link_address() noexcept { return Binding::dma_base + 0x3B8U + static_cast<std::uintptr_t>(Binding::dma_channel) * 4U; }
    [[nodiscard]] static constexpr std::uintptr_t pad_address(core::pin_id pin) noexcept { return mcu::esp32p4::io_mux_base + 0x04U + static_cast<std::uintptr_t>(pin.index) * 4U; }
    [[nodiscard]] static constexpr std::uintptr_t output_select_address(core::pin_id pin) noexcept { return mcu::esp32p4::gpio_matrix_base + 0x558U + static_cast<std::uintptr_t>(pin.index) * 4U; }
    [[nodiscard]] static constexpr std::uintptr_t input_select_address(std::uint16_t signal) noexcept { return mcu::esp32p4::gpio_matrix_base + 0x158U + static_cast<std::uintptr_t>(signal) * 4U; }

    [[nodiscard]] auto read(std::uintptr_t offset) noexcept { return read_absolute(Binding::base + offset); }
    [[nodiscard]] bool write(std::uintptr_t offset, std::uint32_t value) noexcept { return write_absolute(Binding::base + offset, value); }
    [[nodiscard]] auto read_absolute(std::uintptr_t address) noexcept { return mmio_.template read<std::uint32_t>(address); }
    [[nodiscard]] bool write_absolute(std::uintptr_t address, std::uint32_t value) noexcept { return mmio_.template write<std::uint32_t>(address, value).has_value(); }
    [[nodiscard]] bool modify(std::uintptr_t address, std::uint32_t set, std::uint32_t clear) noexcept {
        const auto current = read_absolute(address);
        return current && write_absolute(address, (*current & ~clear) | set);
    }

    static constexpr std::uintptr_t hp_soc_clock_ctrl1 = mcu::esp32p4::hp_soc_clock_ctrl1;
    static constexpr std::uintptr_t hp_soc_clock_ctrl2 = mcu::esp32p4::hp_soc_clock_ctrl2;
    static constexpr std::uintptr_t hp_rst_en1 = mcu::esp32p4::hp_rst_en1;
    static constexpr std::uintptr_t hp_rst_en2 = mcu::esp32p4::hp_rst_en2;
    static constexpr std::uintptr_t hp_peripheral_clock11 = mcu::esp32p4::hp_clkrst_base + 0x44U;
    static constexpr std::uintptr_t hp_peripheral_clock12 = mcu::esp32p4::hp_clkrst_base + 0x48U;
    static constexpr std::uintptr_t hp_peripheral_clock13 = mcu::esp32p4::hp_clkrst_base + 0x4CU;
    static constexpr std::uintptr_t hp_peripheral_clock14 = mcu::esp32p4::hp_clkrst_base + 0x50U;
    static constexpr std::uintptr_t lp_hp_clock_control = 0x5011'1040U;
    static constexpr std::uint32_t i2s0_apb_gate = 1U << 14U;
    static constexpr std::uint32_t i2s0_mclk_pad_gate = 1U << 12U;
    static constexpr std::uint32_t i2s0_reset = 1U << 4U;
    static constexpr std::uint32_t ahb_pdma_system_gate = 1U << 3U;
    static constexpr std::uint32_t ahb_pdma_reset = 1U << 1U;
    static constexpr std::uintptr_t reg_interrupt_enable = 0x14U;
    static constexpr std::uintptr_t reg_interrupt_clear = 0x18U;
    static constexpr std::uintptr_t reg_rx_configuration = 0x20U;
    static constexpr std::uintptr_t reg_tx_configuration = 0x24U;
    static constexpr std::uintptr_t reg_rx_configuration1 = 0x28U;
    static constexpr std::uintptr_t reg_tx_configuration1 = 0x2CU;
    static constexpr std::uintptr_t reg_tx_pdm_configuration = 0x40U;
    static constexpr std::uintptr_t reg_rx_pdm_configuration = 0x48U;
    static constexpr std::uintptr_t reg_rx_tdm_control = 0x50U;
    static constexpr std::uintptr_t reg_tx_tdm_control = 0x54U;
    static constexpr std::uintptr_t reg_rx_eof_num = 0x64U;
    static constexpr std::uintptr_t reg_clock_gate = 0x7CU;
    static constexpr std::uintptr_t dma_misc_configuration = 0x64U;
    static constexpr std::uintptr_t dma_internal_memory_start = 0x3C4U;
    static constexpr std::uintptr_t dma_internal_memory_end = 0x3C8U;
    static constexpr std::uint32_t fifo_reset_bit = 1U << 1U;
    static constexpr std::uint32_t start_bit = 1U << 2U;
    static constexpr std::uint32_t update_bit = 1U << 8U;
    static constexpr std::uint32_t tx_stop_when_empty = 1U << 4U;
    static constexpr std::uint32_t pcm_bypass = 1U << 12U;
    static constexpr std::uint32_t msb_shift = 1U << 13U;
    static constexpr std::uint32_t left_align = 1U << 15U;
    static constexpr std::uint32_t tdm_enable = 1U << 19U;
    static constexpr std::uint32_t rx_complete_interrupt = 1U << 1U;
    static constexpr std::uint32_t tx_complete_interrupt = 1U << 3U;
    static constexpr std::uint32_t rx_error_interrupts = 0x7CU;
    static constexpr std::uint32_t tx_error_interrupts = 0x34U;
    static constexpr std::uint32_t all_dma_interrupts = 0x7FU;
    static constexpr std::uint32_t disconnected_peripheral = 0x3FU;
    static constexpr std::size_t descriptor_maximum_bytes = 4092U;
    static constexpr std::size_t synchronization_limit = 1'000U;

    MmioPolicy& mmio_;
    pool_type operations_;
    MemoryPolicy& memory_;
    alignas(64) dma_descriptor descriptor_{};
    driver::i2s_timing timing_{};
    std::optional<core::operation_id> id_{};
    driver::i2s_direction direction_{driver::i2s_direction::receive};
    void* buffer_{};
    std::size_t bytes_{};
    bool initialized_{};
    bool owned_{};
};

} // namespace mcubsp::backend::esp32p4
