#include <array>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <span>

#include <mcutest/mcutest.hpp>

#include <mcubsp/backend/esp32p4/ethernet.hpp>
#include <mcubsp/board/waveshare/esp32p4_nano/bindings.hpp>
#include <mcubsp/testing/recording_mmio.hpp>

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
    [[nodiscard]] std::expected<Value, mcubsp::core::mmio_error>
    read(std::uintptr_t address) noexcept {
        constexpr auto ethernet = mcubsp::mcu::esp32p4::ethernet_base;
        if (address == ethernet + 0x14U) {
            const auto reg = (value(ethernet + 0x10U) >> 6U) & 0x1FU;
            if (reg == 2U) return static_cast<Value>(0x0243U);
            if (reg == 3U) return static_cast<Value>(0x0C51U);
            if (reg == 1U) return static_cast<Value>(0x0004U);
        }
        return static_cast<Value>(value(address));
    }

    template<std::unsigned_integral Value>
    [[nodiscard]] std::expected<void, mcubsp::core::mmio_error>
    write(std::uintptr_t address, Value written) noexcept {
        auto stored = static_cast<std::uint32_t>(written);
        constexpr auto ethernet = mcubsp::mcu::esp32p4::ethernet_base;
        if ((address == ethernet + 0x1000U || address == ethernet + 0x10U)
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

using dma_mmio = mcubsp::testing::recording_mmio<0x1100, 128>;
using dma_device = mcubsp::backend::esp32p4::ethernet_dma_device<
    dma_mmio, fake_protection, 4U, 4U>;


TEST(esp32p4_ethernet, sequences_rmii_clock_pins_mac_and_ip101_mdio,
     "REQ-0003", "REQ-0010", "REQ-0011", "REQ-0034", "REQ-0043") {
    sparse_mmio mmio;
    mcubsp::backend::esp32p4::ethernet_rmii_mac<decltype(mmio), mcubsp::board::waveshare::esp32p4_nano_bindings::ethernet_binding> mac{mmio};
    const mcubsp::backend::esp32p4::ethernet_mac_address address{
        {0x02U, 0x00U, 0x00U, 0x04U, 0x00U, 0x01U}};
    EXPECT_TRUE(mac.initialize(address).has_value());

    constexpr auto ethernet = mcubsp::mcu::esp32p4::ethernet_base;
    EXPECT_EQ(mmio.value(0x500E'6018U) & (1U << 13U), 1U << 13U);
    EXPECT_EQ(mmio.value(0x500E'514CU) & (0x7U << 2U), 4U << 2U);
    EXPECT_EQ(mmio.value(0x500E'6030U) & ((1U << 27U) | (1U << 29U)),
              (1U << 27U) | (1U << 29U));
    EXPECT_EQ(mmio.value(mcubsp::mcu::esp32p4::io_mux_base + 0x04U
                         + 50U * 4U) & ((0x7U << 12U) | (1U << 9U)),
              (3U << 12U) | (1U << 9U));
    EXPECT_EQ(mmio.value(mcubsp::mcu::esp32p4::gpio_matrix_base + 0x558U
                         + 31U * 4U) & 0x1FFU, 108U);
    EXPECT_EQ(mmio.value(ethernet + 0x44U), 0x0400'0002U);
    EXPECT_EQ(mmio.value(ethernet + 0x40U), 0x0000'0100U);

    const auto identity = mac.identify_ip101();
    ASSERT_TRUE(identity.has_value());
    EXPECT_TRUE(identity->valid());
    EXPECT_EQ(identity->oui(), 0x90C3U);
    EXPECT_EQ(identity->model(), 5U);
    EXPECT_TRUE(mac.write_phy(1U, 0U, 0x1200U));
}

TEST(esp32p4_ethernet, rejects_multicast_and_invalid_phy_arguments,
     "REQ-0002", "REQ-0043") {
    sparse_mmio mmio;
    mcubsp::backend::esp32p4::ethernet_rmii_mac<decltype(mmio), mcubsp::board::waveshare::esp32p4_nano_bindings::ethernet_binding> mac{mmio};
    const mcubsp::backend::esp32p4::ethernet_mac_address multicast{
        {0x01U, 0U, 0U, 0U, 0U, 1U}};
    EXPECT_TRUE(!mac.initialize(multicast).has_value());
    EXPECT_TRUE(!mac.read_phy(32U, 0U).has_value());
    EXPECT_TRUE(!mac.write_phy(1U, 32U, 0U));
}

TEST(esp32p4_ethernet, fixed_enhanced_rings_preserve_slot_completion,
     "REQ-0002", "REQ-0009", "REQ-0013", "REQ-0043") {
    dma_mmio mmio{mcubsp::mcu::esp32p4::ethernet_base};
    fake_protection protection;
    dma_device device{mmio, protection};
    ASSERT_TRUE(device.initialize());

    alignas(64) std::array<std::array<std::byte, dma_device::buffer_size>, 4>
        receive_buffers{};
    alignas(64) std::array<std::array<std::byte, 64>, 4> transmit_frames{};
    EXPECT_TRUE(device.arm_receive(2U, receive_buffers[2]));
    EXPECT_TRUE(device.start_transmit(3U, transmit_frames[3]));
    EXPECT_TRUE(!device.arm_receive(2U, receive_buffers[2]));
    EXPECT_TRUE(!device.start_transmit(3U, transmit_frames[3]));

    auto receive = device.receive_descriptors();
    auto transmit = device.transmit_descriptors();
    EXPECT_EQ(receive[2].word0 & 0x8000'0000U, 0x8000'0000U);
    EXPECT_EQ(receive[2].word1 & 0x0000'4000U, 0x0000'4000U);
    EXPECT_EQ(transmit[3].word0 & 0xF010'0000U, 0xF010'0000U);

    receive[2].word0 = (1514U << 16U) | (1U << 9U) | (1U << 8U);
    transmit[3].word0 = 0U;
    device.poll();
    const auto received = device.take_receive(2U);
    ASSERT_TRUE(received.has_value());
    EXPECT_EQ(received->size, 1514U);
    EXPECT_TRUE(!received->error);
    EXPECT_TRUE(device.take_transmit(3U));
    EXPECT_TRUE(!device.take_transmit(3U));
    EXPECT_EQ(protection.enters, protection.leaves);
}

TEST(esp32p4_ethernet, rejects_short_oversize_and_unaligned_dma_buffers,
     "REQ-0002", "REQ-0013", "REQ-0043") {
    dma_mmio mmio{mcubsp::mcu::esp32p4::ethernet_base};
    fake_protection protection;
    dma_device device{mmio, protection};
    ASSERT_TRUE(device.initialize());
    alignas(64) std::array<std::byte, 64> short_buffer{};
    alignas(64) std::array<std::byte, dma_device::buffer_size + 65U> storage{};
    EXPECT_TRUE(!device.arm_receive(0U, short_buffer));
    EXPECT_TRUE(!device.arm_receive(0U,
        std::span<std::byte>{storage}.subspan(1U, dma_device::buffer_size)));
    EXPECT_TRUE(!device.start_transmit(0U, {}));
    EXPECT_TRUE(!device.start_transmit(0U, storage));
    EXPECT_TRUE(!device.start_transmit(0U,
        std::span<const std::byte>{storage}.subspan(1U, 60U)));
}

} // namespace
