#pragma once

// @req-file REQ-0002 REQ-0003 REQ-0009 REQ-0013 REQ-0033 REQ-0043

#include <array>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <optional>
#include <span>

#include <mcuhw/arch/barrier.hpp>
#include <mcuhw/mcu/stm32h753/ethernet.hpp>

namespace mcuhw::backend::stm32h753 {

enum class ethernet_error : std::uint8_t {
    invalid_argument,
    mmio,
    timeout,
};

struct ethernet_mac_address {
    std::array<std::uint8_t, 6> bytes{};
};

template<typename MmioPolicy, typename Binding>
class ethernet_rmii_mac {
public:
    explicit ethernet_rmii_mac(MmioPolicy& mmio) noexcept : mmio_(mmio) {}

    // Configure the binding's RMII pin set, select RMII before releasing the MAC
    // reset, and initialize store-and-forward operation at 100 Mbit/s full
    // duplex. The PHY-provided 50 MHz reference clock must already be present.
    [[nodiscard]] std::expected<void, ethernet_error> initialize(
        ethernet_mac_address address, std::uint32_t timeout = 100'000U)
        noexcept {
        if ((address.bytes[0] & 1U) != 0U) {
            return std::unexpected(ethernet_error::invalid_argument);
        }
        if (!enable_clocks() || !configure_rmii_pins() || !select_rmii()) {
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
        if (write(eth(0x0304U), low)
                && write(eth(0x0300U), high)
                && write(eth(0x0D00U), 1U << 1U)
                && write(eth(0x0D30U), (0x7U << 20U) | (1U << 5U))
                && write(eth(0x1004U), (1U << 12U) | 1U)
                && write(eth(0x0008U), 0U)
                && write(eth(0x0000U), (1U << 20U) | (1U << 14U)
                                             | (1U << 13U))) {
            return {};
        }
        return std::unexpected(ethernet_error::mmio);
    }

    [[nodiscard]] bool enable() noexcept {
        const auto value = read(eth(0x0000U));
        return value && write(eth(0x0000U), *value | 0x3U);
    }

    [[nodiscard]] bool disable() noexcept {
        const auto value = read(eth(0x0000U));
        return value && write(eth(0x0000U), *value & ~0x3U);
    }

    [[nodiscard]] bool configure_link(std::uint16_t phy_status) noexcept {
        const auto negotiated = static_cast<std::uint16_t>(phy_status & 0x1CU);
        const bool fast = negotiated == 0x08U || negotiated == 0x18U;
        const bool full_duplex = negotiated == 0x14U || negotiated == 0x18U;
        const auto value = read(eth(0x0000U));
        if (!value) return false;
        auto next = *value & ~((1U << 14U) | (1U << 13U));
        if (fast) next |= 1U << 14U;
        if (full_duplex) next |= 1U << 13U;
        return write(eth(0x0000U), next);
    }

    [[nodiscard]] std::optional<std::uint16_t> read_phy(
        std::uint8_t phy, std::uint8_t reg,
        std::uint32_t timeout = 100'000U) noexcept {
        if (phy > 31U || reg > 31U || !wait_clear(eth(0x0200U), 1U, timeout)) {
            return std::nullopt;
        }
        const auto command = (static_cast<std::uint32_t>(phy) << 21U)
            | (static_cast<std::uint32_t>(reg) << 16U) | 0xCU | 1U;
        if (!write(eth(0x0200U), command)
            || !wait_clear(eth(0x0200U), 1U, timeout)) return std::nullopt;
        const auto value = read(eth(0x0204U));
        return value ? std::optional<std::uint16_t>{
                           static_cast<std::uint16_t>(*value)}
                     : std::nullopt;
    }

    [[nodiscard]] bool write_phy(std::uint8_t phy, std::uint8_t reg,
                                 std::uint16_t data,
                                 std::uint32_t timeout = 100'000U) noexcept {
        if (phy > 31U || reg > 31U || !wait_clear(eth(0x0200U), 1U, timeout)
            || !write(eth(0x0204U), data)) return false;
        const auto command = (static_cast<std::uint32_t>(phy) << 21U)
            | (static_cast<std::uint32_t>(reg) << 16U) | 0x4U | 1U;
        return write(eth(0x0200U), command)
            && wait_clear(eth(0x0200U), 1U, timeout);
    }

private:
    static constexpr std::uintptr_t eth(std::uintptr_t offset) noexcept {
        return mcu::stm32h753::ethernet_base + offset;
    }
    [[nodiscard]] bool enable_clocks() noexcept {
        constexpr auto rcc = mcu::stm32h753::rcc_base;
        std::uint32_t ports{};
        for (const auto pin : Binding::rmii_pins) ports |= 1U << pin.port;
        const auto ahb4 = read(rcc + 0xE0U);
        const auto apb4 = read(rcc + 0xF4U);
        const auto ahb1 = read(rcc + 0xD8U);
        return ahb4 && apb4 && ahb1
            && write(rcc + 0xE0U, *ahb4 | ports)
            && write(rcc + 0xF4U, *apb4 | (1U << 1U))
            && write(rcc + 0xD8U, *ahb1 | (0x7U << 15U));
    }
    [[nodiscard]] bool configure_rmii_pins() noexcept {
        for (const auto pin : Binding::rmii_pins) {
            const auto base = mcu::stm32h753::gpio_base(pin.port);
            const auto pair_shift = static_cast<unsigned>(pin.index * 2U);
            const auto pair_mask = 0x3U << pair_shift;
            auto mode = read(base + 0x00U);
            auto type = read(base + 0x04U);
            auto speed = read(base + 0x08U);
            auto pull = read(base + 0x0CU);
            const auto af_offset = pin.index < 8U ? 0x20U : 0x24U;
            const auto af_shift = static_cast<unsigned>((pin.index % 8U) * 4U);
            auto af = read(base + af_offset);
            if (!mode || !type || !speed || !pull || !af) return false;
            if (!write(base + 0x04U, *type & ~(1U << pin.index))
                || !write(base + 0x08U, (*speed & ~pair_mask)
                                            | (0x3U << pair_shift))
                || !write(base + 0x0CU, *pull & ~pair_mask)
                || !write(base + af_offset,
                           (*af & ~(0xFU << af_shift)) | (11U << af_shift))
                || !write(base + 0x00U,
                           (*mode & ~pair_mask) | (0x2U << pair_shift))) {
                return false;
            }
        }
        arch::data_synchronization_barrier();
        return true;
    }
    [[nodiscard]] bool select_rmii() noexcept {
        const auto pmcr = read(mcu::stm32h753::syscfg_base + 0x04U);
        if (!pmcr || !write(mcu::stm32h753::syscfg_base + 0x04U,
                            (*pmcr & ~(0x7U << 21U)) | (0x4U << 21U))) {
            return false;
        }
        // ST requires a read-back/delay before ETH software reset so the
        // interface selection propagates into the MAC clock domain.
        (void)read(mcu::stm32h753::syscfg_base + 0x04U);
        arch::data_synchronization_barrier();
        return true;
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
};

// Hardware consumes the first four words. Each slot owns a complete M7 cache
// line so publishing one descriptor cannot overwrite a neighbor's DMA result.
struct alignas(32) ethernet_descriptor {
    std::uint32_t word0{};
    std::uint32_t word1{};
    std::uint32_t word2{};
    std::uint32_t word3{};
    std::array<std::uint32_t, 4> padding{};
};
static_assert(sizeof(ethernet_descriptor) == 32U);

struct ethernet_receive_completion {
    std::size_t size{};
    bool error{};
};

// Single-owner protection policy for the Cortex-M7 data cache. Buffer storage
// supplied to the device must own complete 32-byte cache lines.
template<typename MmioPolicy>
class ethernet_descriptor_protection {
public:
    explicit ethernet_descriptor_protection(MmioPolicy& mmio) noexcept
        : mmio_(mmio) {}

    void enter() noexcept { arch::data_memory_barrier(); }
    void leave() noexcept { arch::data_memory_barrier(); }
    void publish_barrier() noexcept { arch::data_synchronization_barrier(); }
    void acquire_barrier() noexcept { arch::data_synchronization_barrier(); }

    void clean_for_device(std::span<const std::byte> bytes) noexcept {
        maintain(0xE000'EF68U, bytes.data(), bytes.size());
    }
    void invalidate_from_device(std::span<std::byte> bytes) noexcept {
        maintain(0xE000'EF5CU, bytes.data(), bytes.size());
    }

private:
    void maintain(std::uintptr_t command, const void* pointer,
                  std::size_t size) noexcept {
        if (size == 0U) return;
        constexpr std::uintptr_t line_mask = 31U;
        const auto first = reinterpret_cast<std::uintptr_t>(pointer) & ~line_mask;
        const auto last = (reinterpret_cast<std::uintptr_t>(pointer) + size - 1U)
            & ~line_mask;
        for (auto address = first;; address += 32U) {
            (void)mmio_.template write<std::uint32_t>(
                command, static_cast<std::uint32_t>(address));
            if (address == last) break;
        }
        arch::data_synchronization_barrier();
    }

    MmioPolicy& mmio_;
};

// Fixed-storage descriptor device. Its public surface intentionally matches
// mcuip::NetworkDevice without importing the stack into the hardware layer.
template<typename MmioPolicy, typename Protection,
         std::size_t RxDescriptors = 4U, std::size_t TxDescriptors = 4U>
class ethernet_dma_device {
public:
    static_assert(RxDescriptors != 0U && RxDescriptors <= 1024U
                  && TxDescriptors > 1U && TxDescriptors <= 1024U);
    static constexpr std::size_t rx_descriptor_count = RxDescriptors;
    static constexpr std::size_t tx_descriptor_count = TxDescriptors;
    static constexpr std::size_t buffer_size = 1536U;

    ethernet_dma_device(MmioPolicy& mmio, Protection& protection) noexcept
        : mmio_(mmio), protection_(protection) {}

    [[nodiscard]] bool initialize() noexcept {
        clear_descriptors();
        // DMACCR.DSL counts skipped 32-bit words after the 16-byte hardware
        // descriptor: four words produce the 32-byte software slot stride.
        if (!write(0x1100U, 4U << 18U)
            || !write(0x1104U, 8U << 16U)
            || !write(0x1108U, (8U << 16U) | (buffer_size << 1U))
            || !write(0x1114U, address_of(tx_descriptors_.data()))
            || !write(0x111CU, address_of(rx_descriptors_.data()))
            || !write(0x112CU, static_cast<std::uint32_t>(TxDescriptors - 1U))
            || !write(0x1130U, static_cast<std::uint32_t>(RxDescriptors - 1U))) {
            return false;
        }
        clean_descriptor_rings();
        return true;
    }

    [[nodiscard]] bool arm_receive(std::size_t index,
                                   std::span<std::byte> buffer) noexcept {
        if (index >= RxDescriptors || buffer.size() < buffer_size
            || (reinterpret_cast<std::uintptr_t>(buffer.data()) % 32U) != 0U
            || rx_active_[index]) return false;
        protection_.enter();
        protection_.invalidate_from_device(buffer.first(buffer_size));
        auto& descriptor = rx_descriptors_[index];
        descriptor = {address_of(buffer.data()), 0U, 0U,
                      descriptor_owned | rx_descriptor_ioc | rx_buffer1_valid};
        rx_buffers_[index] = buffer.first(buffer_size);
        rx_completions_[index].reset();
        rx_active_[index] = true;
        protection_.clean_for_device(std::as_bytes(std::span{&descriptor, 1U}));
        protection_.publish_barrier();
        (void)write(0x1128U, address_of(&rx_descriptors_[index]));
        protection_.leave();
        return true;
    }

    [[nodiscard]] bool start_transmit(
        std::size_t index, std::span<const std::byte> frame) noexcept {
        if (index >= TxDescriptors || frame.empty() || frame.size() > buffer_size
            || (reinterpret_cast<std::uintptr_t>(frame.data()) % 32U) != 0U
            || tx_active_[index]) return false;
        protection_.enter();
        protection_.clean_for_device(frame);
        auto& descriptor = tx_descriptors_[index];
        descriptor = {address_of(frame.data()), 0U,
                      tx_descriptor_ioc
                          | static_cast<std::uint32_t>(frame.size()),
                      descriptor_owned | descriptor_first | descriptor_last};
        tx_active_[index] = true;
        tx_completions_[index] = false;
        protection_.clean_for_device(std::as_bytes(std::span{&descriptor, 1U}));
        protection_.publish_barrier();
        (void)write(0x1120U, address_of(&tx_descriptors_[index]));
        protection_.leave();
        return true;
    }

    void start() noexcept {
        (void)update(0x1104U, 1U);
        (void)update(0x1108U, 1U);
    }

    void stop() noexcept {
        (void)clear(0x1104U, 1U);
        (void)clear(0x1108U, 1U);
    }

    void poll() noexcept {
        protection_.enter();
        invalidate_descriptor_rings();
        protection_.acquire_barrier();
        for (std::size_t index = 0U; index < RxDescriptors; ++index) {
            if (rx_active_[index]
                && (rx_descriptors_[index].word3 & descriptor_owned) == 0U) {
                const auto status = rx_descriptors_[index].word3;
                const auto size = static_cast<std::size_t>(status & 0x7FFFU);
                const bool whole_frame = (status & (descriptor_first | descriptor_last))
                    == (descriptor_first | descriptor_last);
                rx_completions_[index] = ethernet_receive_completion{
                    size, !whole_frame || (status & descriptor_error) != 0U};
                protection_.invalidate_from_device(rx_buffers_[index]);
                rx_active_[index] = false;
            }
        }
        for (std::size_t index = 0U; index < TxDescriptors; ++index) {
            if (tx_active_[index]
                && (tx_descriptors_[index].word3 & descriptor_owned) == 0U) {
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
    static constexpr std::uint32_t descriptor_owned = 0x8000'0000U;
    static constexpr std::uint32_t rx_descriptor_ioc = 0x4000'0000U;
    static constexpr std::uint32_t tx_descriptor_ioc = 0x8000'0000U;
    static constexpr std::uint32_t descriptor_first = 0x2000'0000U;
    static constexpr std::uint32_t descriptor_last = 0x1000'0000U;
    static constexpr std::uint32_t rx_buffer1_valid = 0x0100'0000U;
    static constexpr std::uint32_t descriptor_error = 0x0000'8000U;

    static std::uint32_t address_of(const void* pointer) noexcept {
        return static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(pointer));
    }
    [[nodiscard]] bool write(std::uintptr_t offset,
                             std::uint32_t value) noexcept {
        return mmio_.template write<std::uint32_t>(
            mcu::stm32h753::ethernet_base + offset, value).has_value();
    }
    [[nodiscard]] bool update(std::uintptr_t offset,
                              std::uint32_t mask) noexcept {
        const auto value = mmio_.template read<std::uint32_t>(
            mcu::stm32h753::ethernet_base + offset);
        return value && write(offset, *value | mask);
    }
    [[nodiscard]] bool clear(std::uintptr_t offset,
                             std::uint32_t mask) noexcept {
        const auto value = mmio_.template read<std::uint32_t>(
            mcu::stm32h753::ethernet_base + offset);
        return value && write(offset, *value & ~mask);
    }
    void clear_descriptors() noexcept {
        for (auto& descriptor : rx_descriptors_) descriptor = {};
        for (auto& descriptor : tx_descriptors_) descriptor = {};
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
    alignas(32) std::array<ethernet_descriptor, RxDescriptors> rx_descriptors_{};
    alignas(32) std::array<ethernet_descriptor, TxDescriptors> tx_descriptors_{};
    std::array<std::span<std::byte>, RxDescriptors> rx_buffers_{};
    std::array<std::optional<ethernet_receive_completion>, RxDescriptors>
        rx_completions_{};
    std::array<bool, RxDescriptors> rx_active_{};
    std::array<bool, TxDescriptors> tx_active_{};
    std::array<bool, TxDescriptors> tx_completions_{};
};

} // namespace mcuhw::backend::stm32h753
