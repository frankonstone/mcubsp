#include <array>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <expected>

#include <mcutest/mcutest.hpp>

#include <mcuhw/backend/esp32p4/sdmmc.hpp>
#include <mcuhw/board/esp32p4_nano.hpp>
#include <mcuhw/core/mmio.hpp>

namespace {

using binding = mcuhw::board::esp32p4_nano::sdmmc_slot0_binding;
constexpr auto base = binding::base;

class sparse_mmio {
public:
    bool stall_data{};
    bool reject_dma_reset{};
    bool reject_poll_demand{};
    std::uint32_t data_error{};

    template<std::unsigned_integral Value>
        requires(sizeof(Value) == 1 || sizeof(Value) == 2 || sizeof(Value) == 4)
    [[nodiscard]] std::expected<Value, mcuhw::core::mmio_error>
    read(std::uintptr_t address) noexcept {
        return static_cast<Value>(value(address));
    }

    template<std::unsigned_integral Value>
        requires(sizeof(Value) == 1 || sizeof(Value) == 2 || sizeof(Value) == 4)
    [[nodiscard]] std::expected<void, mcuhw::core::mmio_error>
    write(std::uintptr_t address, Value next) noexcept {
        auto word = static_cast<std::uint32_t>(next);
        if ((reject_dma_reset && address == base && (word & (1U << 2U)) != 0U)
            || (reject_poll_demand && address == base + 0x84U)) {
            return std::unexpected(mcuhw::core::mmio_error::out_of_range);
        }
        if (address == base + 0x00U && (word & 0x7U) != 0U) {
            word &= ~0x7U;
        }
        if (address == base + 0x80U && (word & 1U) != 0U) {
            word &= ~1U;
        }
        if (address == base + 0x44U) {
            set(address, value(address) & ~word);
            return {};
        }
        if (address == base + 0x8CU) {
            set(address, value(address) & ~word);
            return {};
        }
        if (address == base + 0x2CU && (word & (1U << 31U)) != 0U) {
            const auto update = (word & (1U << 21U)) != 0U;
            set(address, word & ~(1U << 31U));
            if (!update) {
                set(base + 0x44U, value(base + 0x44U) | (1U << 2U));
                if ((word & (1U << 9U)) != 0U) {
                    if (!stall_data) {
                        set(base + 0x44U, value(base + 0x44U) | (1U << 3U));
                        const auto dma_done = (word & (1U << 10U)) != 0U
                            ? 1U << 0U : 1U << 1U;
                        set(base + 0x8CU, value(base + 0x8CU) | dma_done);
                    }
                    set(base + 0x44U, value(base + 0x44U) | data_error);
                }
                ++commands_;
            }
            return {};
        }
        set(address, word);
        return {};
    }

    void set(std::uintptr_t address, std::uint32_t next) noexcept {
        for (std::size_t index = 0U; index < count_; ++index) {
            if (cells_[index].address == address) {
                cells_[index].value = next;
                return;
            }
        }
        if (count_ < cells_.size()) cells_[count_++] = {address, next};
    }
    [[nodiscard]] std::uint32_t value(std::uintptr_t address) const noexcept {
        for (std::size_t index = 0U; index < count_; ++index) {
            if (cells_[index].address == address) return cells_[index].value;
        }
        return 0U;
    }
    [[nodiscard]] std::size_t commands() const noexcept { return commands_; }

private:
    struct cell { std::uintptr_t address{}; std::uint32_t value{}; };
    std::array<cell, 96> cells_{};
    std::size_t count_{};
    std::size_t commands_{};
};

class fake_memory {
public:
    const void* last_data{};
    std::byte read_pattern{};
    [[nodiscard]] std::expected<std::uint32_t,
                                mcuhw::driver::sdmmc_error>
    address(const void*) noexcept {
        const auto result = next_;
        next_ += 0x100U;
        return result;
    }
    [[nodiscard]] bool prepare_for_device(const void* pointer, std::size_t size) noexcept {
        if (size == 512U) last_data = pointer;
        ++prepared_;
        return true;
    }
    [[nodiscard]] bool complete_from_device(void* pointer, std::size_t size) noexcept {
        std::memset(pointer, std::to_integer<int>(read_pattern), size);
        ++completed_;
        return true;
    }
    [[nodiscard]] std::size_t prepared() const noexcept { return prepared_; }
    [[nodiscard]] std::size_t completed() const noexcept { return completed_; }

private:
    std::uint32_t next_{0x4FF0'1000U};
    std::size_t prepared_{};
    std::size_t completed_{};
};

using backend = mcuhw::backend::esp32p4::sdmmc_host_backend<
    sparse_mmio, fake_memory, binding>;
using wifi_binding = mcuhw::board::esp32p4_nano::sdmmc_slot1_wifi_binding;
using wifi_backend = mcuhw::backend::esp32p4::sdmmc_host_backend<
    sparse_mmio, fake_memory, wifi_binding>;

TEST(esp32p4_sdmmc, configures_dedicated_slot_clock_reset_and_pins,
     "REQ-0009", "REQ-0010", "REQ-0011", "REQ-0034") {
    sparse_mmio mmio;
    fake_memory memory;
    backend host{mmio, memory};
    ASSERT_TRUE(host.initialize());
    EXPECT_TRUE((mmio.value(binding::pmu_base + 0x1D8U)
                 & ((1U << 7U) | (1U << 8U) | (1U << 14U)))
                == ((1U << 7U) | (1U << 8U) | (1U << 14U)));
    EXPECT_TRUE((mmio.value(binding::pmu_base + 0x1DCU)
                 & (1U << 26U)) != 0U);
    EXPECT_TRUE((mmio.value(binding::hp_clock_base + 0x18U)
                 & (1U << 14U)) != 0U);
    EXPECT_EQ(mmio.value(binding::lp_clock_base + 0x4CU)
                  & (1U << 28U), 0U);
    EXPECT_EQ(mmio.value(base + 0x08U), 20U);
    EXPECT_TRUE((mmio.value(base + 0x10U) & 1U) != 0U);
    EXPECT_EQ(mmio.value(base + 0x90U), 0x103U);
    for (const auto pin : {binding::d0, binding::d1, binding::d2,
                           binding::d3, binding::clock,
                           binding::command}) {
        const auto pad = mmio.value(mcuhw::mcu::esp32p4::io_mux_base
                                    + 0x04U + pin * 4U);
        EXPECT_EQ(pad & (0x7U << 12U), 0U);
        EXPECT_TRUE((pad & (1U << 9U)) != 0U);
    }
    EXPECT_TRUE(host.shutdown().has_value());
}

TEST(esp32p4_sdmmc, encodes_commands_and_completes_sector_dma_read_and_write,
     "REQ-0009", "REQ-0013", "REQ-0014", "REQ-0016", "REQ-0019",
     "REQ-0034", "REQ-0037") {
    sparse_mmio mmio;
    fake_memory memory;
    backend host{mmio, memory};
    ASSERT_TRUE(host.initialize());
    mmio.set(base + 0x30U, 0x1AAU);
    const auto response = host.command(
        {8U, 0x1AAU, mcuhw::driver::sdmmc_response_type::short_crc, false});
    ASSERT_TRUE(response.has_value());
    EXPECT_EQ(response->words[0], 0x1AAU);
    EXPECT_EQ(mmio.value(base + 0x28U), 0x1AAU);
    EXPECT_EQ(mmio.value(base + 0x2CU) & 0x3FU, 8U);

    std::array<std::byte, 512> sector{};
    memory.read_pattern = std::byte{0xA5};
    const auto read = host.read_data(
        {17U, 7U, mcuhw::driver::sdmmc_response_type::short_crc, false},
        sector);
    ASSERT_TRUE(read.has_value());
    EXPECT_EQ(mmio.value(base + 0x1CU), 512U);
    EXPECT_EQ(mmio.value(base + 0x20U), 512U);
    EXPECT_TRUE(memory.prepared() >= 2U);
    EXPECT_EQ(memory.completed(), 1U);
    EXPECT_EQ(sector.front(), std::byte{0xA5});
    EXPECT_EQ(sector.back(), std::byte{0xA5});
    EXPECT_TRUE(memory.last_data != sector.data());

    std::array<std::byte, 512> output{};
    output.fill(std::byte{0x5A});
    const auto write = host.write_data(
        {24U, 9U, mcuhw::driver::sdmmc_response_type::short_crc, false},
        output);
    ASSERT_TRUE(write.has_value());
    EXPECT_EQ(mmio.value(base + 0x28U), 9U);
    EXPECT_TRUE((mmio.value(base + 0x2CU) & (1U << 10U)) != 0U);
    EXPECT_TRUE(memory.prepared() >= 4U);
    EXPECT_EQ(memory.completed(), 1U);
    EXPECT_TRUE(memory.last_data != output.data());
    EXPECT_EQ(std::memcmp(memory.last_data, output.data(), output.size()), 0);
    EXPECT_TRUE(host.set_bus_width_4());
    EXPECT_TRUE((mmio.value(base + 0x18U) & 1U) != 0U);
    EXPECT_TRUE(host.set_transfer_clock());
    EXPECT_EQ(mmio.value(base + 0x08U), 0U);
    EXPECT_TRUE(host.shutdown().has_value());
}

TEST(esp32p4_sdmmc, data_timeouts_stop_dma_before_returning,
     "REQ-0013", "REQ-0014", "REQ-0019", "REQ-0037") {
    sparse_mmio mmio;
    fake_memory memory;
    backend host{mmio, memory};
    ASSERT_TRUE(host.initialize());
    mmio.stall_data = true;
    std::array<std::byte, 512> sector{};
    sector.fill(std::byte{0x33});
    const auto read = host.read_data(
        {17U, 0U, mcuhw::driver::sdmmc_response_type::short_crc, false}, sector);
    ASSERT_FALSE(read.has_value());
    EXPECT_TRUE(read.error() == mcuhw::driver::sdmmc_error::data_timeout);
    EXPECT_EQ(mmio.value(base) & ((1U << 5U) | (1U << 25U) | 0x6U), 0U);
    EXPECT_EQ(mmio.value(base + 0x80U) & 0x83U, 0U);
    EXPECT_EQ(memory.completed(), 0U);
    EXPECT_EQ(sector.front(), std::byte{0x33});
    EXPECT_TRUE(memory.last_data != sector.data());

    const auto write = host.write_data(
        {24U, 0U, mcuhw::driver::sdmmc_response_type::short_crc, false}, sector);
    ASSERT_FALSE(write.has_value());
    EXPECT_TRUE(write.error() == mcuhw::driver::sdmmc_error::data_timeout);
    EXPECT_EQ(mmio.value(base) & ((1U << 5U) | (1U << 25U) | 0x6U), 0U);
    EXPECT_EQ(mmio.value(base + 0x80U) & 0x83U, 0U);
    EXPECT_TRUE(host.shutdown().has_value());
}

TEST(esp32p4_sdmmc, hardware_and_setup_errors_also_stop_dma,
     "REQ-0013", "REQ-0014", "REQ-0019", "REQ-0037") {
    sparse_mmio mmio;
    fake_memory memory;
    backend host{mmio, memory};
    ASSERT_TRUE(host.initialize());
    std::array<std::byte, 512> sector{};
    mmio.data_error = 1U << 7U;
    const auto read = host.read_data(
        {17U, 0U, mcuhw::driver::sdmmc_response_type::short_crc, false}, sector);
    ASSERT_FALSE(read.has_value());
    EXPECT_TRUE(read.error() == mcuhw::driver::sdmmc_error::data_crc);
    EXPECT_EQ(mmio.value(base) & ((1U << 5U) | (1U << 25U)), 0U);
    EXPECT_EQ(mmio.value(base + 0x80U) & 0x82U, 0U);

    mmio.data_error = 0U;
    mmio.reject_poll_demand = true;
    const auto write = host.write_data(
        {24U, 0U, mcuhw::driver::sdmmc_response_type::short_crc, false}, sector);
    ASSERT_FALSE(write.has_value());
    EXPECT_TRUE(write.error() == mcuhw::driver::sdmmc_error::dma);
    EXPECT_EQ(mmio.value(base) & ((1U << 5U) | (1U << 25U)), 0U);
    EXPECT_EQ(mmio.value(base + 0x80U) & 0x82U, 0U);
    EXPECT_TRUE(host.shutdown().has_value());
}

TEST(esp32p4_sdmmc, unconfirmed_reset_retains_storage_and_host_ownership,
     "REQ-0013", "REQ-0014", "REQ-0019", "REQ-0037") {
    sparse_mmio mmio;
    fake_memory memory;
    backend host{mmio, memory};
    ASSERT_TRUE(host.initialize());
    mmio.data_error = 1U << 9U;
    mmio.reject_dma_reset = true;
    std::array<std::byte, 512> sector{};
    sector.fill(std::byte{0x33});
    const auto read = host.read_data(
        {17U, 0U, mcuhw::driver::sdmmc_response_type::short_crc, false}, sector);
    ASSERT_FALSE(read.has_value());
    EXPECT_TRUE(read.error() == mcuhw::driver::sdmmc_error::dma);
    // Even an unexpectedly late DMA write targets retained static storage.
    ASSERT_TRUE(memory.last_data != sector.data());
    std::memset(const_cast<void*>(memory.last_data), 0xEE, sector.size());
    EXPECT_EQ(sector.front(), std::byte{0x33});
    EXPECT_EQ(sector.back(), std::byte{0x33});
    EXPECT_FALSE(host.command({}).has_value());
    EXPECT_FALSE(host.shutdown().has_value());
    backend competing{mmio, memory};
    EXPECT_FALSE(competing.initialize());

    mmio.reject_dma_reset = false;
    EXPECT_TRUE(host.shutdown().has_value());
    EXPECT_TRUE(competing.initialize());
    EXPECT_TRUE(competing.shutdown().has_value());
}

TEST(esp32p4_sdmmc, routes_slot1_through_gpio_matrix_and_selects_card_one,
     "REQ-0009", "REQ-0010", "REQ-0011", "REQ-0034") {
    sparse_mmio mmio;
    fake_memory memory;
    wifi_backend host{mmio, memory};
    ASSERT_TRUE(host.initialize());
    EXPECT_EQ(mmio.value(wifi_binding::pmu_base + 0x1D8U), 0U);
    EXPECT_TRUE((mmio.value(base + 0x10U) & ((1U << 1U) | (1U << 17U)))
                == ((1U << 1U) | (1U << 17U)));

    const auto output_address = [](std::uint8_t pin) {
        return mcuhw::mcu::esp32p4::gpio_matrix_base + 0x558U + pin * 4U;
    };
    const auto input_address = [](std::uint16_t signal) {
        return mcuhw::mcu::esp32p4::gpio_matrix_base + 0x158U + signal * 4U;
    };
    EXPECT_EQ(mmio.value(output_address(wifi_binding::clock)) & 0x3FFU,
              wifi_binding::clock_signal);
    EXPECT_EQ(mmio.value(output_address(wifi_binding::command)) & 0x3FFU,
              wifi_binding::command_signal);
    EXPECT_EQ(mmio.value(input_address(wifi_binding::command_signal)) & 0xFFU,
              static_cast<std::uint32_t>(wifi_binding::command) | (1U << 7U));
    EXPECT_EQ(mmio.value(input_address(wifi_binding::d3_signal)) & 0xFFU,
              static_cast<std::uint32_t>(wifi_binding::d3) | (1U << 7U));
    for (const auto pin : {wifi_binding::command, wifi_binding::d0,
                           wifi_binding::d1, wifi_binding::d2,
                           wifi_binding::d3}) {
        const auto pad = mmio.value(mcuhw::mcu::esp32p4::io_mux_base
                                    + 0x04U + pin * 4U);
        EXPECT_EQ(pad & (0x7U << 12U), 1U << 12U);
        EXPECT_TRUE((pad & ((1U << 8U) | (1U << 9U)))
                    == ((1U << 8U) | (1U << 9U)));
    }

    mmio.set(base + 0x30U, 0xA5U);
    const auto response = host.command(
        {52U, 0U, mcuhw::driver::sdmmc_response_type::short_crc, false});
    ASSERT_TRUE(response.has_value());
    EXPECT_EQ(response->words[0], 0xA5U);
    EXPECT_EQ((mmio.value(base + 0x2CU) >> 16U) & 0x1FU, 1U);
    EXPECT_TRUE(host.set_bus_width_4());
    EXPECT_TRUE((mmio.value(base + 0x18U) & (1U << 1U)) != 0U);
    EXPECT_EQ(mmio.value(base + 0x18U) & (1U << 17U), 0U);
    EXPECT_TRUE(host.shutdown().has_value());
}


struct alternate_slot_binding : wifi_binding {
    static constexpr std::uint8_t clock = 20U;
};

TEST(esp32p4_sdmmc, board_binding_controls_the_gpio_matrix_route,
     "REQ-0003", "REQ-0034") {
    sparse_mmio mmio;
    fake_memory memory;
    mcuhw::backend::esp32p4::sdmmc_host_backend<sparse_mmio, fake_memory, alternate_slot_binding> host{mmio, memory};
    ASSERT_TRUE(host.initialize());
    const auto pad = mcuhw::mcu::esp32p4::io_mux_base + 0x04U;
    EXPECT_EQ((mmio.value(pad + 20U * 4U) >> 12U) & 0x7U, 1U);
    EXPECT_EQ(mmio.value(pad + wifi_binding::clock * 4U), 0U);
    EXPECT_TRUE(host.shutdown().has_value());
}

} // namespace
