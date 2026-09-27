#pragma once

// @req-file REQ-0002 REQ-0003 REQ-0009 REQ-0010 REQ-0011 REQ-0013 REQ-0034 REQ-0043

#include <array>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <optional>
#include <span>

#include <mcuhw/arch/barrier.hpp>
#include <mcuhw/mcu/esp32p4/ethernet.hpp>

namespace mcuhw::backend::esp32p4 {

enum class ethernet_error : std::uint8_t {
    invalid_argument,
    mmio,
    timeout,
};

struct ethernet_mac_address {
    std::array<std::uint8_t, 6> bytes{};
};

struct ip101_identity {
    std::uint16_t id1{};
    std::uint16_t id2{};

    [[nodiscard]] constexpr std::uint32_t oui() const noexcept {
        return (static_cast<std::uint32_t>(id1) << 6U)
            | (static_cast<std::uint32_t>(id2) >> 10U);
    }
    [[nodiscard]] constexpr std::uint8_t model() const noexcept {
        return static_cast<std::uint8_t>((id2 >> 4U) & 0x3FU);
    }
    [[nodiscard]] constexpr bool valid() const noexcept {
        return oui() == 0x90C3U && model() == 0x05U;
    }
};

// ESP32-P4 native EMAC using an external 50 MHz RMII reference clock. This
// binding selects the board's IOMUX data pins and GPIO-matrix SMI pins.
// PHY reset remains a board concern because its polarity and timing are not an
// EMAC property.
template<typename MmioPolicy, typename Binding>
class ethernet_rmii_mac {
public:
    explicit ethernet_rmii_mac(MmioPolicy& mmio) noexcept : mmio_(mmio) {}

    [[nodiscard]] std::expected<void, ethernet_error> initialize(
        ethernet_mac_address address,
        std::uint32_t timeout = 100'000U) noexcept {
        if ((address.bytes[0] & 1U) != 0U) {
            return std::unexpected(ethernet_error::invalid_argument);
        }
        if (!enable_clocks() || !configure_rmii_pins()
            || !configure_smi_pins() || !select_rmii_input_clock()
            || !pulse_reset()) {
            return std::unexpected(ethernet_error::mmio);
        }
        if (!write(eth(0x1000U), 1U)) {
            return std::unexpected(ethernet_error::mmio);
        }
        if (!wait_clear(eth(0x1000U), 1U, timeout)) {
            return std::unexpected(ethernet_error::timeout);
        }

        const auto low = static_cast<std::uint32_t>(address.bytes[0])
            | (static_cast<std::uint32_t>(address.bytes[1]) << 8U)
            | (static_cast<std::uint32_t>(address.bytes[2]) << 16U)
            | (static_cast<std::uint32_t>(address.bytes[3]) << 24U);
        const auto high = static_cast<std::uint32_t>(address.bytes[4])
            | (static_cast<std::uint32_t>(address.bytes[5]) << 8U);

        // Enhanced descriptors, aligned/mixed eight-beat bursts. MAC starts
        // disabled at 100 Mbit/s full duplex until descriptor storage is armed.
        constexpr std::uint32_t bus_mode = (1U << 7U) | (8U << 8U)
            | (1U << 25U) | (1U << 26U);
        constexpr std::uint32_t mac_configuration = (1U << 7U)
            | (1U << 11U) | (1U << 14U) | (1U << 15U);
        if (!write(eth(0x1000U), bus_mode)
            || !write(eth(0x0044U), low)
            || !write(eth(0x0040U), high)
            || !write(eth(0x0004U), 0U)
            || !write(eth(0x0000U), mac_configuration)
            || !write(eth(0x0010U), 4U << 2U)) {
            return std::unexpected(ethernet_error::mmio);
        }
        initialized_ = true;
        return {};
    }

    [[nodiscard]] bool enable() noexcept {
        return initialized_ && modify(eth(0x0000U), (1U << 2U) | (1U << 3U), 0U);
    }

    [[nodiscard]] bool disable() noexcept {
        return modify(eth(0x0000U), 0U, (1U << 2U) | (1U << 3U));
    }

    // IP101 page-16 CSSR op_mode: 1=10H, 2=100H, 5=10F, 6=100F.
    [[nodiscard]] bool configure_ip101_link(std::uint16_t cssr) noexcept {
        const auto mode = static_cast<std::uint8_t>(cssr & 0x7U);
        if (mode != 1U && mode != 2U && mode != 5U && mode != 6U) {
            return false;
        }
        const bool fast = mode == 2U || mode == 6U;
        const bool full = mode == 5U || mode == 6U;
        return modify(eth(0x0000U),
                      (fast ? 1U << 14U : 0U)
                          | (full ? 1U << 11U : 0U) | (1U << 15U),
                      (fast ? 0U : 1U << 14U)
                          | (full ? 0U : 1U << 11U));
    }

    [[nodiscard]] std::optional<std::uint16_t> read_phy(
        std::uint8_t phy, std::uint8_t reg,
        std::uint32_t timeout = 100'000U) noexcept {
        if (phy > 31U || reg > 31U || !wait_clear(eth(0x0010U), 1U, timeout)) {
            return std::nullopt;
        }
        const auto command = (static_cast<std::uint32_t>(phy) << 11U)
            | (static_cast<std::uint32_t>(reg) << 6U) | (4U << 2U) | 1U;
        if (!write(eth(0x0010U), command)
            || !wait_clear(eth(0x0010U), 1U, timeout)) {
            return std::nullopt;
        }
        const auto value = read(eth(0x0014U));
        return value ? std::optional<std::uint16_t>{
                           static_cast<std::uint16_t>(*value)}
                     : std::nullopt;
    }

    [[nodiscard]] bool write_phy(std::uint8_t phy, std::uint8_t reg,
                                 std::uint16_t data,
                                 std::uint32_t timeout = 100'000U) noexcept {
        if (phy > 31U || reg > 31U
            || !wait_clear(eth(0x0010U), 1U, timeout)
            || !write(eth(0x0014U), data)) {
            return false;
        }
        const auto command = (static_cast<std::uint32_t>(phy) << 11U)
            | (static_cast<std::uint32_t>(reg) << 6U)
            | (4U << 2U) | (1U << 1U) | 1U;
        return write(eth(0x0010U), command)
            && wait_clear(eth(0x0010U), 1U, timeout);
    }

    [[nodiscard]] std::optional<ip101_identity> identify_ip101(
        std::uint8_t phy = Binding::phy_address,
        std::uint32_t timeout = 100'000U) noexcept {
        const auto id1 = read_phy(phy, 2U, timeout);
        const auto id2 = read_phy(phy, 3U, timeout);
        if (!id1 || !id2) return std::nullopt;
        return ip101_identity{*id1, *id2};
    }

private:
    static constexpr std::uintptr_t hp_clock_base = 0x500E'6000U;
    static constexpr std::uintptr_t lp_clock_base = 0x5011'1000U;

    static constexpr std::uintptr_t eth(std::uintptr_t offset) noexcept {
        return mcu::esp32p4::ethernet_base + offset;
    }
    static constexpr std::uintptr_t pad(std::uint8_t pin) noexcept {
        return mcu::esp32p4::io_mux_base + 0x04U
            + static_cast<std::uintptr_t>(pin) * 4U;
    }
    static constexpr std::uintptr_t output_select(std::uint8_t pin) noexcept {
        return mcu::esp32p4::gpio_matrix_base + 0x558U
            + static_cast<std::uintptr_t>(pin) * 4U;
    }
    static constexpr std::uintptr_t input_select(std::uint16_t signal) noexcept {
        return mcu::esp32p4::gpio_matrix_base + 0x158U
            + static_cast<std::uintptr_t>(signal) * 4U;
    }

    [[nodiscard]] bool enable_clocks() noexcept {
        return modify(hp_clock_base + 0x18U, 1U << 13U, 0U);
    }

    [[nodiscard]] bool select_rmii_input_clock() noexcept {
        constexpr auto gmac_control = mcu::esp32p4::hp_system_base + 0x14CU;
        constexpr auto clock0 = hp_clock_base + 0x30U;
        constexpr auto clock1 = hp_clock_base + 0x34U;
        constexpr auto lp_clock = lp_clock_base + 0x40U;
        return modify(gmac_control, 4U << 2U, 0x3U << 2U)
            && modify(clock0, (1U << 27U) | (1U << 29U),
                      (1U << 24U) | (0x3U << 25U) | (1U << 28U))
            && modify(clock1, 1U | (1U << 9U) | (1U << 10U),
                      0xFEU | (1U << 8U) | (0xFEU << 10U))
            && modify(lp_clock, 1U << 15U, (1U << 13U) | (1U << 14U));
    }

    [[nodiscard]] bool pulse_reset() noexcept {
        constexpr auto reset = lp_clock_base + 0x4CU;
        return modify(reset, 1U << 30U, 0U)
            && modify(reset, 0U, 1U << 30U);
    }

    [[nodiscard]] bool configure_rmii_pins() noexcept {
        for (const auto pin : Binding::rmii_inputs) {
            if (!configure_iomux(pin, true)) return false;
        }
        for (const auto pin : Binding::rmii_outputs) {
            if (!configure_iomux(pin, false)) return false;
        }
        return configure_iomux(Binding::reference_clock, true);
    }

    [[nodiscard]] bool configure_iomux(std::uint8_t pin,
                                       bool input) noexcept {
        const auto current = read(pad(pin));
        if (!current) return false;
        constexpr std::uint32_t controlled = (1U << 7U) | (1U << 8U)
            | (1U << 9U) | (0x3U << 10U) | (0x7U << 12U);
        auto next = static_cast<std::uint32_t>((*current & ~controlled)
            | (0x2U << 10U) | (3U << 12U));
        if (input) next |= 1U << 9U;
        return write(pad(pin), next);
    }

    [[nodiscard]] bool configure_smi_pins() noexcept {
        // MDC uses matrix output 108. MDIO uses matrix output 109
        // plus matrix input 107; peripheral-controlled output enable is kept.
        return configure_matrix_pad(Binding::mdc, false)
            && route_output(Binding::mdc, 108U)
            && configure_matrix_pad(Binding::mdio, true)
            && route_output(Binding::mdio, 109U)
            && route_input(Binding::mdio, 107U);
    }

    [[nodiscard]] bool configure_matrix_pad(std::uint8_t pin,
                                            bool input) noexcept {
        const auto current = read(pad(pin));
        if (!current) return false;
        constexpr std::uint32_t controlled = (1U << 7U) | (1U << 8U)
            | (1U << 9U) | (0x3U << 10U) | (0x7U << 12U);
        auto next = static_cast<std::uint32_t>((*current & ~controlled)
            | (0x2U << 10U) | (1U << 12U));
        if (input) next |= 1U << 9U;
        return write(pad(pin), next);
    }

    [[nodiscard]] bool route_output(std::uint8_t pin,
                                    std::uint16_t signal) noexcept {
        const auto address = output_select(pin);
        const auto current = read(address);
        return current && write(address, (*current & ~0xFFFU) | signal);
    }

    [[nodiscard]] bool route_input(std::uint8_t pin,
                                   std::uint16_t signal) noexcept {
        const auto address = input_select(signal);
        const auto current = read(address);
        return current && write(address, (*current & ~0x1FFU) | pin);
    }

    [[nodiscard]] bool wait_clear(std::uintptr_t address, std::uint32_t mask,
                                  std::uint32_t timeout) noexcept {
        while (timeout-- != 0U) {
            const auto value = read(address);
            if (!value) return false;
            if ((*value & mask) == 0U) return true;
        }
        return false;
    }

    [[nodiscard]] bool modify(std::uintptr_t address, std::uint32_t set,
                              std::uint32_t clear) noexcept {
        const auto value = read(address);
        return value && write(address, (*value | set) & ~clear);
    }

    [[nodiscard]] std::optional<std::uint32_t>
    read(std::uintptr_t address) noexcept {
        const auto value = mmio_.template read<std::uint32_t>(address);
        return value ? std::optional<std::uint32_t>{*value} : std::nullopt;
    }

    [[nodiscard]] bool write(std::uintptr_t address,
                             std::uint32_t value) noexcept {
        return mmio_.template write<std::uint32_t>(address, value).has_value();
    }

    MmioPolicy& mmio_;
    bool initialized_{};
};

struct alignas(64) ethernet_descriptor {
    std::uint32_t word0{};
    std::uint32_t word1{};
    std::uint32_t word2{};
    std::uint32_t word3{};
    std::array<std::uint32_t, 12> padding{};
};
static_assert(sizeof(ethernet_descriptor) == 64U);

struct ethernet_receive_completion {
    std::size_t size{};
    bool error{};
};

// Fixed-storage enhanced-descriptor rings. Protection owns cache maintenance
// and synchronization so the hardware layer remains independent of ESP-IDF.
template<typename MmioPolicy, typename Protection,
         std::size_t RxDescriptors = 4U, std::size_t TxDescriptors = 4U>
class ethernet_dma_device {
public:
    static_assert(RxDescriptors != 0U && TxDescriptors > 1U);
    static constexpr std::size_t rx_descriptor_count = RxDescriptors;
    static constexpr std::size_t tx_descriptor_count = TxDescriptors;
    static constexpr std::size_t buffer_size = 1536U;

    ethernet_dma_device(MmioPolicy& mmio, Protection& protection) noexcept
        : mmio_(mmio), protection_(protection) {}

    [[nodiscard]] bool initialize() noexcept {
        clear_descriptors();
        link_rings();
        clean_descriptor_rings();
        return write(0x100CU, address_of(rx_descriptors_.data()))
            && write(0x1010U, address_of(tx_descriptors_.data()))
            && write(0x101CU, 0U)
            && write(0x1014U, 0xFFFF'FFFFU);
    }

    [[nodiscard]] bool arm_receive(std::size_t index,
                                   std::span<std::byte> buffer) noexcept {
        if (index >= RxDescriptors || buffer.size() < buffer_size
            || (reinterpret_cast<std::uintptr_t>(buffer.data()) % 64U) != 0U
            || rx_active_[index]) return false;
        protection_.enter();
        protection_.invalidate_from_device(buffer.first(buffer_size));
        auto& descriptor = rx_descriptors_[index];
        descriptor.word1 = rx_chained | static_cast<std::uint32_t>(buffer_size);
        descriptor.word2 = address_of(buffer.data());
        descriptor.word3 = address_of(
            &rx_descriptors_[(index + 1U) % RxDescriptors]);
        descriptor.word0 = descriptor_owned;
        rx_buffers_[index] = buffer.first(buffer_size);
        rx_completions_[index].reset();
        rx_active_[index] = true;
        protection_.clean_for_device(std::as_bytes(std::span{&descriptor, 1U}));
        protection_.publish_barrier();
        (void)write(0x1008U, 1U);
        protection_.leave();
        return true;
    }

    [[nodiscard]] bool start_transmit(
        std::size_t index, std::span<const std::byte> frame) noexcept {
        if (index >= TxDescriptors || frame.empty() || frame.size() > buffer_size
            || (reinterpret_cast<std::uintptr_t>(frame.data()) % 64U) != 0U
            || tx_active_[index]) return false;
        protection_.enter();
        protection_.clean_for_device(frame);
        auto& descriptor = tx_descriptors_[index];
        descriptor.word1 = static_cast<std::uint32_t>(frame.size());
        descriptor.word2 = address_of(frame.data());
        descriptor.word3 = address_of(
            &tx_descriptors_[(index + 1U) % TxDescriptors]);
        descriptor.word0 = descriptor_owned | tx_interrupt
            | descriptor_first | descriptor_last | tx_chained;
        tx_active_[index] = true;
        tx_completions_[index] = false;
        protection_.clean_for_device(std::as_bytes(std::span{&descriptor, 1U}));
        protection_.publish_barrier();
        (void)write(0x1004U, 1U);
        protection_.leave();
        return true;
    }

    void start() noexcept { (void)update(0x1018U, (1U << 1U) | (1U << 13U)); }
    void stop() noexcept { (void)clear(0x1018U, (1U << 1U) | (1U << 13U)); }

    void poll() noexcept {
        protection_.enter();
        invalidate_descriptor_rings();
        protection_.acquire_barrier();
        for (std::size_t index = 0U; index < RxDescriptors; ++index) {
            const auto status = rx_descriptors_[index].word0;
            if (rx_active_[index] && (status & descriptor_owned) == 0U) {
                const auto size = static_cast<std::size_t>(
                    (status >> 16U) & 0x3FFFU);
                const bool whole = (status & (rx_first | rx_last))
                    == (rx_first | rx_last);
                rx_completions_[index] = {
                    size, !whole || (status & descriptor_error) != 0U};
                protection_.invalidate_from_device(rx_buffers_[index]);
                rx_active_[index] = false;
            }
        }
        for (std::size_t index = 0U; index < TxDescriptors; ++index) {
            if (tx_active_[index]
                && (tx_descriptors_[index].word0 & descriptor_owned) == 0U) {
                tx_completions_[index] = true;
                tx_active_[index] = false;
            }
        }
        protection_.leave();
    }

    [[nodiscard]] std::optional<ethernet_receive_completion>
    take_receive(std::size_t index) noexcept {
        if (index >= RxDescriptors) return std::nullopt;
        auto result = rx_completions_[index];
        rx_completions_[index].reset();
        return result;
    }

    [[nodiscard]] bool take_transmit(std::size_t index) noexcept {
        if (index >= TxDescriptors) return false;
        const bool result = tx_completions_[index];
        tx_completions_[index] = false;
        return result;
    }

    [[nodiscard]] std::span<ethernet_descriptor, RxDescriptors>
    receive_descriptors() noexcept { return rx_descriptors_; }
    [[nodiscard]] std::span<ethernet_descriptor, TxDescriptors>
    transmit_descriptors() noexcept { return tx_descriptors_; }

private:
    static constexpr std::uint32_t descriptor_owned = 1U << 31U;
    static constexpr std::uint32_t tx_interrupt = 1U << 30U;
    static constexpr std::uint32_t descriptor_last = 1U << 29U;
    static constexpr std::uint32_t descriptor_first = 1U << 28U;
    static constexpr std::uint32_t tx_chained = 1U << 20U;
    static constexpr std::uint32_t rx_chained = 1U << 14U;
    static constexpr std::uint32_t descriptor_error = 1U << 15U;
    static constexpr std::uint32_t rx_first = 1U << 9U;
    static constexpr std::uint32_t rx_last = 1U << 8U;

    static std::uint32_t address_of(const void* pointer) noexcept {
        return static_cast<std::uint32_t>(
            reinterpret_cast<std::uintptr_t>(pointer));
    }
    [[nodiscard]] bool write(std::uintptr_t offset,
                             std::uint32_t value) noexcept {
        return mmio_.template write<std::uint32_t>(
            mcu::esp32p4::ethernet_base + offset, value).has_value();
    }
    [[nodiscard]] bool update(std::uintptr_t offset,
                              std::uint32_t mask) noexcept {
        const auto value = mmio_.template read<std::uint32_t>(
            mcu::esp32p4::ethernet_base + offset);
        return value && write(offset, *value | mask);
    }
    [[nodiscard]] bool clear(std::uintptr_t offset,
                             std::uint32_t mask) noexcept {
        const auto value = mmio_.template read<std::uint32_t>(
            mcu::esp32p4::ethernet_base + offset);
        return value && write(offset, *value & ~mask);
    }
    void clear_descriptors() noexcept {
        for (auto& descriptor : rx_descriptors_) descriptor = {};
        for (auto& descriptor : tx_descriptors_) descriptor = {};
    }
    void link_rings() noexcept {
        for (std::size_t index = 0U; index < RxDescriptors; ++index) {
            rx_descriptors_[index].word1 = rx_chained;
            rx_descriptors_[index].word3 = address_of(
                &rx_descriptors_[(index + 1U) % RxDescriptors]);
        }
        for (std::size_t index = 0U; index < TxDescriptors; ++index) {
            tx_descriptors_[index].word0 = tx_chained;
            tx_descriptors_[index].word3 = address_of(
                &tx_descriptors_[(index + 1U) % TxDescriptors]);
        }
    }
    void clean_descriptor_rings() noexcept {
        protection_.clean_for_device(std::as_bytes(std::span{rx_descriptors_}));
        protection_.clean_for_device(std::as_bytes(std::span{tx_descriptors_}));
    }
    void invalidate_descriptor_rings() noexcept {
        protection_.invalidate_from_device(
            std::as_writable_bytes(std::span{rx_descriptors_}));
        protection_.invalidate_from_device(
            std::as_writable_bytes(std::span{tx_descriptors_}));
    }

    MmioPolicy& mmio_;
    Protection& protection_;
    alignas(64) std::array<ethernet_descriptor, RxDescriptors> rx_descriptors_{};
    alignas(64) std::array<ethernet_descriptor, TxDescriptors> tx_descriptors_{};
    std::array<std::span<std::byte>, RxDescriptors> rx_buffers_{};
    std::array<std::optional<ethernet_receive_completion>, RxDescriptors>
        rx_completions_{};
    std::array<bool, RxDescriptors> rx_active_{};
    std::array<bool, TxDescriptors> tx_active_{};
    std::array<bool, TxDescriptors> tx_completions_{};
};

} // namespace mcuhw::backend::esp32p4
