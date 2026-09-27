#include <array>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <expected>
#include <span>

#include <mcutest/mcutest.hpp>

#include <mcuhw/backend/stm32h753/ethernet.hpp>
#include <mcuhw/board/nucleo_h753zi.hpp>
#include <mcuhw/integration/mcuip/stm32h753_ethernet.hpp>
#include <mcuhw/testing/recording_mmio.hpp>

namespace {

struct fake_protection {
    void enter() noexcept { ++enters; }
    void leave() noexcept { ++leaves; }
    void publish_barrier() noexcept { ++publishes; }
    void acquire_barrier() noexcept { ++acquires; }
    void clean_for_device(std::span<const std::byte>) noexcept { ++cleans; }
    void invalidate_from_device(std::span<std::byte>) noexcept { ++invalidates; }

    std::size_t enters{};
    std::size_t leaves{};
    std::size_t publishes{};
    std::size_t acquires{};
    std::size_t cleans{};
    std::size_t invalidates{};
};

class sparse_mmio {
public:
    template<std::unsigned_integral Value>
    [[nodiscard]] std::expected<Value, mcuhw::core::mmio_error>
    read(std::uintptr_t address) noexcept {
        return static_cast<Value>(value(address));
    }

    template<std::unsigned_integral Value>
    [[nodiscard]] std::expected<void, mcuhw::core::mmio_error>
    write(std::uintptr_t address, Value written) noexcept {
        auto stored = static_cast<std::uint32_t>(written);
        constexpr auto ethernet = mcuhw::mcu::stm32h753::ethernet_base;
        if ((address == ethernet + 0x1000U || address == ethernet + 0x0200U)
            && (stored & 1U) != 0U) stored &= ~1U;
        set(address, stored);
        return {};
    }

    void set(std::uintptr_t address, std::uint32_t stored) noexcept {
        for (std::size_t index = 0U; index < count_; ++index) {
            if (cells_[index].address == address) {
                cells_[index].value = stored;
                return;
            }
        }
        if (count_ < cells_.size()) cells_[count_++] = {address, stored};
    }

    [[nodiscard]] std::uint32_t value(std::uintptr_t address) const noexcept {
        for (std::size_t index = 0U; index < count_; ++index) {
            if (cells_[index].address == address) return cells_[index].value;
        }
        return 0U;
    }

private:
    struct cell { std::uintptr_t address{}; std::uint32_t value{}; };
    std::array<cell, 128> cells_{};
    std::size_t count_{};
};

using dma_mmio = mcuhw::testing::recording_mmio<0x1200, 128>;
using dma_device = mcuhw::backend::stm32h753::ethernet_dma_device<
    dma_mmio, fake_protection, 4U, 4U>;

// Separate CPU cache and DMA RAM views, with writeback only for dirty 32-byte
// lines. This catches lost completions that a coherent-memory fake cannot.
class split_cache_protection {
public:
    using descriptor = mcuhw::backend::stm32h753::ethernet_descriptor;
    static constexpr std::size_t ring_size = sizeof(descriptor) * 4U;

    void attach(std::span<descriptor> rx, std::span<descriptor> tx) noexcept {
        rings_[0].cpu = reinterpret_cast<std::byte*>(rx.data());
        rings_[1].cpu = reinterpret_cast<std::byte*>(tx.data());
    }
    void enter() noexcept {}
    void leave() noexcept {}
    void publish_barrier() noexcept {}
    void acquire_barrier() noexcept {}

    void clean_for_device(std::span<const std::byte> bytes) noexcept {
        maintain(bytes.data(), bytes.size(), true);
    }
    void invalidate_from_device(std::span<std::byte> bytes) noexcept {
        maintain(bytes.data(), bytes.size(), false);
    }
    void complete(std::size_t ring, std::size_t slot, std::uint32_t status) noexcept {
        std::memcpy(rings_[ring].ram.data() + slot * sizeof(descriptor)
                        + offsetof(descriptor, word3), &status, sizeof(status));
    }

private:
    void maintain(const std::byte* bytes, std::size_t size, bool clean) noexcept {
        const auto first = reinterpret_cast<std::uintptr_t>(bytes);
        const auto last = first + size;
        for (auto& ring : rings_) {
            const auto base = reinterpret_cast<std::uintptr_t>(ring.cpu);
            if (first < base || last > base + ring_size) continue;
            const auto offset = (first - base) & ~std::uintptr_t{31U};
            for (auto line = offset; base + line < last; line += 32U) {
                if (clean) {
                    if (std::memcmp(ring.cpu + line, ring.clean.data() + line, 32U) != 0) {
                        std::memcpy(ring.ram.data() + line, ring.cpu + line, 32U);
                    }
                } else {
                    std::memcpy(ring.cpu + line, ring.ram.data() + line, 32U);
                }
                std::memcpy(ring.clean.data() + line, ring.cpu + line, 32U);
            }
        }
    }
    struct ring {
        std::byte* cpu{};
        std::array<std::byte, ring_size> ram{};
        std::array<std::byte, ring_size> clean{};
    };
    std::array<ring, 2> rings_{};
};

TEST(stm32h753_ethernet, rearming_a_slot_preserves_neighbor_dma_completions,
     "REQ-0013", "REQ-0043") {
    dma_mmio mmio{mcuhw::mcu::stm32h753::ethernet_base};
    split_cache_protection cache;
    mcuhw::backend::stm32h753::ethernet_dma_device<dma_mmio, split_cache_protection> device{mmio, cache};
    cache.attach(device.receive_descriptors(), device.transmit_descriptors());
    ASSERT_TRUE(device.initialize());
    const auto control = mmio.read<std::uint32_t>(mcuhw::mcu::stm32h753::ethernet_base + 0x1100U);
    ASSERT_TRUE(control.has_value());
    const auto hardware_stride = 16U + ((*control >> 18U) & 7U) * 4U;
    EXPECT_EQ(hardware_stride, sizeof(split_cache_protection::descriptor));
    EXPECT_EQ(hardware_stride, 32U);

    alignas(32) std::array<std::array<std::byte, dma_device::buffer_size>, 2> rx{};
    alignas(32) std::array<std::array<std::byte, 64>, 2> tx{};
    for (std::size_t index = 0U; index < 2U; ++index) {
        ASSERT_TRUE(device.arm_receive(index, rx[index]));
        ASSERT_TRUE(device.start_transmit(index, tx[index]));
    }
    cache.complete(0U, 0U, 0x3000'003CU);
    cache.complete(1U, 0U, 0U);
    device.poll();
    ASSERT_TRUE(device.take_receive(0U).has_value());
    ASSERT_TRUE(device.take_transmit(0U));

    // DMA updates slot 1 after the CPU last invalidated its cached OWN bit.
    cache.complete(0U, 1U, 0x3000'0080U);
    cache.complete(1U, 1U, 0U);
    ASSERT_TRUE(device.arm_receive(0U, rx[0]));
    ASSERT_TRUE(device.start_transmit(0U, tx[0]));
    device.poll();
    const auto received = device.take_receive(1U);
    ASSERT_TRUE(received.has_value());
    EXPECT_EQ(received->size, 128U);
    EXPECT_FALSE(received->error);
    EXPECT_TRUE(device.take_transmit(1U));
}

#if defined(MCUHW_TEST_MCUIP_CONTRACTS)
static_assert(::mcuip::NetworkDevice<dma_device>);
static_assert(::mcuip::DescriptorProtection<fake_protection>);
static_assert(::mcuip::valid_hardware(
    mcuhw::integration::mcuip::nucleo_h753zi_hardware));
#endif

TEST(stm32h753_ethernet, fixed_rings_preserve_ownership_and_completion_order,
     "REQ-0002", "REQ-0009", "REQ-0013", "REQ-0043") {
    dma_mmio mmio{mcuhw::mcu::stm32h753::ethernet_base};
    fake_protection protection;
    dma_device device{mmio, protection};
    ASSERT_TRUE(device.initialize());

    alignas(32) std::array<
        std::array<std::byte, dma_device::buffer_size>,
        dma_device::rx_descriptor_count> rx{};
    alignas(32) std::array<std::array<std::byte, 64>,
                           dma_device::tx_descriptor_count> tx{};
    for (std::size_t index = 0U; index < rx.size(); ++index) {
        EXPECT_TRUE(device.arm_receive(index, rx[index]));
        EXPECT_TRUE(device.start_transmit(index, tx[index]));
    }
    EXPECT_TRUE(!device.arm_receive(0U, rx[0]));
    EXPECT_TRUE(!device.arm_receive(4U, rx[0]));
    EXPECT_TRUE(!device.start_transmit(1U, tx[1]));

    auto receive = device.receive_descriptors();
    auto transmit = device.transmit_descriptors();
    EXPECT_EQ(receive[0].word3 & 0x8000'0000U, 0x8000'0000U);
    EXPECT_EQ(transmit[1].word2 & 0x8000'0000U, 0x8000'0000U);
    EXPECT_EQ(transmit[1].word3 & 0xB000'0000U, 0xB000'0000U);

    // Hardware may complete slots out of index order; each terminal result
    // must remain attached to its original fixed slot.
    receive[2].word3 = 0x3000'0000U | 1514U;
    receive[0].word3 = 0x3000'0000U | 60U;
    transmit[3].word3 = 0U;
    transmit[1].word3 = 0U;
    device.poll();
    const auto received_later_slot = device.take_receive(2U);
    const auto received_first_slot = device.take_receive(0U);
    ASSERT_TRUE(received_later_slot.has_value());
    ASSERT_TRUE(received_first_slot.has_value());
    EXPECT_EQ(received_later_slot->size, 1514U);
    EXPECT_EQ(received_first_slot->size, 60U);
    EXPECT_TRUE(!received_later_slot->error);
    EXPECT_TRUE(!received_first_slot->error);
    EXPECT_TRUE(device.take_transmit(3U));
    EXPECT_TRUE(device.take_transmit(1U));
    EXPECT_TRUE(!device.take_transmit(1U));
    EXPECT_EQ(protection.enters, protection.leaves);
    EXPECT_TRUE(protection.publishes >= 2U);
    EXPECT_TRUE(protection.acquires >= 1U);
}

TEST(stm32h753_ethernet, rejects_short_receive_and_oversize_transmit,
     "REQ-0002", "REQ-0043") {
    dma_mmio mmio{mcuhw::mcu::stm32h753::ethernet_base};
    fake_protection protection;
    dma_device device{mmio, protection};
    ASSERT_TRUE(device.initialize());
    alignas(32) std::array<std::byte, 64> short_buffer{};
    alignas(32) std::array<std::byte, dma_device::buffer_size + 1U> oversize{};
    EXPECT_TRUE(!device.arm_receive(0U, short_buffer));
    EXPECT_TRUE(!device.start_transmit(0U, oversize));
    EXPECT_TRUE(!device.start_transmit(0U, {}));
    EXPECT_TRUE(!device.start_transmit(0U,
        std::span<const std::byte>{oversize}.subspan(1U, 60U)));
}

TEST(stm32h753_ethernet, sequences_rmii_gpio_mac_and_mdio,
     "REQ-0003", "REQ-0010", "REQ-0033", "REQ-0043") {
    sparse_mmio mmio;
    mcuhw::backend::stm32h753::ethernet_rmii_mac<decltype(mmio), mcuhw::board::nucleo_h753zi::ethernet_binding> mac{mmio};
    const mcuhw::backend::stm32h753::ethernet_mac_address address{
        {0x02U, 0x00U, 0x00U, 0x75U, 0x30U, 0x01U}};
    EXPECT_TRUE(mac.initialize(address).has_value());

    constexpr auto rcc = mcuhw::mcu::stm32h753::rcc_base;
    constexpr auto ethernet = mcuhw::mcu::stm32h753::ethernet_base;
    EXPECT_EQ(mmio.value(rcc + 0xD8U) & (0x7U << 15U), 0x7U << 15U);
    EXPECT_EQ(mmio.value(rcc + 0xE0U) & 0x47U, 0x47U);
    EXPECT_EQ(mmio.value(mcuhw::mcu::stm32h753::syscfg_base + 0x04U)
                  & (0x7U << 21U),
              0x4U << 21U);
    EXPECT_EQ(mmio.value(mcuhw::mcu::stm32h753::gpio_base(0U) + 0x20U)
                  & (0xFU << 4U),
              11U << 4U);
    EXPECT_EQ(mmio.value(ethernet + 0x0304U), 0x7500'0002U);
    EXPECT_EQ(mmio.value(ethernet + 0x0300U), 0x0000'0130U);

    mmio.set(ethernet + 0x0204U, 0x0000'0007U);
    const auto phy = mac.read_phy(0U, 2U);
    ASSERT_TRUE(phy.has_value());
    EXPECT_EQ(*phy, 0x0007U);
    EXPECT_TRUE(mac.write_phy(0U, 4U, 0x01E1U));
    EXPECT_EQ(mmio.value(ethernet + 0x0204U), 0x01E1U);
    EXPECT_TRUE(mac.configure_link(0x0018U));
    EXPECT_EQ(mmio.value(ethernet) & ((1U << 14U) | (1U << 13U)),
              (1U << 14U) | (1U << 13U));
}

TEST(stm32h753_ethernet, rejects_multicast_station_address,
     "REQ-0002", "REQ-0043") {
    sparse_mmio mmio;
    mcuhw::backend::stm32h753::ethernet_rmii_mac<decltype(mmio), mcuhw::board::nucleo_h753zi::ethernet_binding> mac{mmio};
    const mcuhw::backend::stm32h753::ethernet_mac_address multicast{
        {0x01U, 0U, 0U, 0U, 0U, 1U}};
    const auto initialized = mac.initialize(multicast);
    ASSERT_TRUE(!initialized.has_value());
    EXPECT_EQ(static_cast<unsigned>(initialized.error()),
              static_cast<unsigned>(
                  mcuhw::backend::stm32h753::ethernet_error::invalid_argument));
}


struct alternate_rmii_binding {
    static constexpr auto rmii_pins = [] {
        auto pins = mcuhw::board::nucleo_h753zi::ethernet_binding::rmii_pins;
        pins.back() = mcuhw::core::pin_id{6U, 14U};
        return pins;
    }();
};

TEST(stm32h753_ethernet, routes_pins_from_the_selected_board_binding,
     "REQ-0003", "REQ-0043") {
    sparse_mmio mmio;
    mcuhw::backend::stm32h753::ethernet_rmii_mac<sparse_mmio, alternate_rmii_binding> mac{mmio};
    ASSERT_TRUE(mac.initialize({{0x02U, 0U, 0U, 0U, 0U, 1U}}).has_value());
    EXPECT_EQ((mmio.value(mcuhw::mcu::stm32h753::gpio_base(6U) + 0x24U) >> 24U) & 0xFU, 11U);
    EXPECT_EQ((mmio.value(mcuhw::mcu::stm32h753::gpio_base(1U) + 0x24U) >> 20U) & 0xFU, 0U);
    EXPECT_EQ(mmio.value(mcuhw::mcu::stm32h753::rcc_base + 0xE0U) & 0x47U, 0x45U);
}

} // namespace
