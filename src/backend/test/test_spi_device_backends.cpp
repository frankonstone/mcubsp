#include <array>
#include <cstdint>
#include <expected>
#include <span>

#include <mcutest/mcutest.hpp>

#include <mcubsp/backend/esp32p4/spi.hpp>
#include <mcubsp/backend/ra4m1/spi.hpp>
#include <mcubsp/backend/stm32h753/spi.hpp>
#include <mcubsp/board/arduino/uno_r4_minima/bindings.hpp>
#include <mcubsp/board/st/nucleo_h753zi/bindings.hpp>
#include <mcubsp/core/mmio.hpp>
#include <mcubsp/execution/fake_policy.hpp>

namespace {

// Sparse register model: preserves pin sentinels, records writes, and models
// the documented ESP SPI synchronization command's self-clearing UPDATE bit.
struct device_spi_mmio {
    struct cell { std::uintptr_t address{}; std::uint32_t value{}; };
    std::array<cell, 160> cells{};
    std::size_t count{};
    std::size_t writes{};
    bool fail{};

    std::uint32_t value(std::uintptr_t address) const noexcept {
        for (std::size_t i = 0; i < count; ++i)
            if (cells[i].address == address) return cells[i].value;
        return 0U;
    }
    void set(std::uintptr_t address, std::uint32_t next) noexcept {
        for (std::size_t i = 0; i < count; ++i) {
            if (cells[i].address == address) { cells[i].value = next; return; }
        }
        if (count < cells.size()) cells[count++] = {address, next};
    }
    template<typename Value>
    std::expected<Value, mcubsp::core::mmio_error> read(std::uintptr_t address) noexcept {
        if (fail) return std::unexpected(mcubsp::core::mmio_error::invalid_value);
        return static_cast<Value>(value(address));
    }
    template<typename Value>
    std::expected<void, mcubsp::core::mmio_error> write(
        std::uintptr_t address, Value next) noexcept {
        if (fail) return std::unexpected(mcubsp::core::mmio_error::invalid_value);
        ++writes;
        auto word = static_cast<std::uint32_t>(next);
        if (address == mcubsp::mcu::esp32p4::spi2_base) word &= ~(1U << 23U);
        set(address, word);
        return {};
    }
};

struct device_select {
    bool active{};
    std::uint32_t assertions{};
    std::uint32_t releases{};
    static bool set(void* pointer, bool active) noexcept {
        auto& self = *static_cast<device_select*>(pointer);
        self.active = active;
        if (active) ++self.assertions; else ++self.releases;
        return true;
    }
};

using uno = mcubsp::board::arduino::uno_r4_minima_bindings;
using nucleo = mcubsp::board::st::nucleo_h753zi_bindings;
using ra_binding = mcubsp::backend::ra4m1::spi_binding<
    1, uno::d12_miso, uno::d11_mosi, uno::d13_sck, uno::d10_ssl>;
using stm_binding = mcubsp::backend::stm32h753::spi_binding<
    1, nucleo::arduino_d12_miso, nucleo::arduino_d11_mosi,
    nucleo::arduino_d13_sck, nucleo::arduino_d10_cs>;
using esp_binding = mcubsp::backend::esp32p4::spi_binding<
    2, mcubsp::core::pin_id{0, 20}, mcubsp::core::pin_id{0, 21},
    mcubsp::core::pin_id{0, 22}, mcubsp::core::pin_id{0, 23}>;
using policy = mcubsp::execution::fake_policy;
using ra_backend = mcubsp::backend::ra4m1::spi_master_backend<
    ra_binding, device_spi_mmio, policy, true>;
using stm_backend = mcubsp::backend::stm32h753::spi_master_backend<
    stm_binding, device_spi_mmio, policy, true>;
using esp_backend = mcubsp::backend::esp32p4::spi_master_backend<
    esp_binding, device_spi_mmio, policy, true>;

constexpr auto ra_timing = *mcubsp::backend::ra4m1::solve_spi_timing({48'000'000U, 1'000'000U, 0U});
constexpr auto stm_timing = *mcubsp::backend::stm32h753::solve_stm32_spi_timing(
    {64'000'000U, 1'000'000U, 0U});
constexpr auto esp_timing = *mcubsp::backend::esp32p4::solve_spi_timing(
    {40'000'000U, 1'000'000U, 0U});

TEST(spi_device_backend, ra_external_select_tracks_wire_completion_and_deadline,
     "REQ-0028", "REQ-0045") {
    device_spi_mmio mmio;
    policy execution;
    device_select selected;
    ra_backend spi{mmio, execution};
    const auto fixed_cs = mcubsp::mcu::ra4m1::pfs_address(ra_binding::select_pin);
    mmio.set(fixed_cs, 0x5A5A5A5AU);
    ASSERT_TRUE(spi.initialize(ra_timing).has_value());
    EXPECT_EQ(mmio.value(fixed_cs), 0x5A5A5A5AU);
    ASSERT_TRUE(spi.set_select_handler(&device_select::set, &selected).has_value());
    const std::array<std::uint16_t, 1> tx{0x5AU};
    const std::array segments{mcubsp::driver::spi_segment{.transmit = tx}};
    auto started = spi.transaction(segments);
    ASSERT_TRUE(started.has_value());
    EXPECT_TRUE(selected.active);
    EXPECT_TRUE(spi.busy());
    EXPECT_TRUE(spi.reconfigure(ra_timing).error() == mcubsp::driver::spi_error::busy);
    EXPECT_TRUE(spi.set_select_handler(nullptr, nullptr).error() == mcubsp::driver::spi_error::busy);
    ASSERT_TRUE(spi.handle_transmit_empty_interrupt());
    ASSERT_TRUE(spi.handle_receive_interrupt());
    EXPECT_TRUE(selected.active);
    mmio.set(ra_binding::metadata.base + 3U, 0U);
    ASSERT_TRUE(spi.handle_idle_interrupt());
    EXPECT_FALSE(selected.active);
    ASSERT_TRUE(started->take().has_value());

    const std::array<std::uint8_t, 1> byte{0x55U};
    std::uint32_t checks{};
    const auto immediate = spi.transaction_immediate(byte, {}, 100U,
        [&checks]() noexcept { return ++checks >= 3U; });
    EXPECT_TRUE(immediate.error() == mcubsp::driver::spi_error::timeout);
    EXPECT_EQ(checks, 3U);
    EXPECT_FALSE(selected.active);
    EXPECT_FALSE(spi.busy());
    EXPECT_EQ(selected.assertions, 2U);
    EXPECT_EQ(selected.releases, 2U);
    ASSERT_TRUE(spi.shutdown().has_value());
    EXPECT_EQ(mmio.value(fixed_cs), 0x5A5A5A5AU);
}

TEST(spi_device_backend, ra_reconfigure_rejects_invalid_and_recovers_mmio_failure,
     "REQ-0028", "REQ-0045") {
    device_spi_mmio mmio;
    policy execution;
    device_select selected;
    ra_backend spi{mmio, execution};
    ASSERT_TRUE(spi.initialize(ra_timing).has_value());
    ASSERT_TRUE(spi.set_select_handler(&device_select::set, &selected).has_value());
    const auto writes = mmio.writes;
    EXPECT_TRUE(spi.reconfigure(ra_timing, {
        .mode = static_cast<mcubsp::driver::spi_mode>(255)}).error()
        == mcubsp::driver::spi_error::invalid_frame);
    EXPECT_EQ(mmio.writes, writes);
    mmio.fail = true;
    EXPECT_TRUE(spi.reconfigure(ra_timing).error() == mcubsp::driver::spi_error::mmio);
    mmio.fail = false;
    const std::array<std::uint16_t, 1> tx{0x55U};
    const std::array segments{mcubsp::driver::spi_segment{.transmit = tx}};
    EXPECT_TRUE(spi.transaction(segments).error().cause == mcubsp::driver::spi_error::mmio);
    EXPECT_EQ(selected.assertions, 0U);
    ASSERT_TRUE(spi.reconfigure(ra_timing).has_value());
    auto started = spi.transaction(segments);
    ASSERT_TRUE(started.has_value());
    mmio.fail = true;
    EXPECT_FALSE(started->cancel().has_value());
    EXPECT_FALSE(selected.active);
    EXPECT_EQ(selected.releases, 1U);
    EXPECT_FALSE(started->take().has_value());
    mmio.fail = false;
    ASSERT_TRUE(spi.shutdown().has_value());
}

TEST(spi_device_backend, stm_external_select_releases_even_on_mmio_failure,
     "REQ-0028", "REQ-0045") {
    device_spi_mmio mmio;
    policy execution;
    device_select selected;
    stm_backend spi{mmio, execution};
    ASSERT_TRUE(spi.initialize(stm_timing).has_value());
    ASSERT_TRUE(spi.set_select_handler(&device_select::set, &selected).has_value());
    ASSERT_TRUE(spi.reconfigure(stm_timing, {
        .mode = mcubsp::driver::spi_mode::mode3,
        .frame_width = mcubsp::driver::spi_frame_width::bits16}).has_value());
    EXPECT_EQ(mmio.value(stm_binding::metadata.base + 8U) & 31U, 15U);
    EXPECT_TRUE((mmio.value(stm_binding::metadata.base + 12U) & (3U << 24U))
                == (3U << 24U));
    const std::array<std::uint16_t, 1> tx{0xABCDU};
    const std::array segments{mcubsp::driver::spi_segment{.transmit = tx}};
    auto started = spi.transaction(segments);
    ASSERT_TRUE(started.has_value());
    EXPECT_TRUE(selected.active);
    EXPECT_TRUE(spi.reconfigure(stm_timing).error() == mcubsp::driver::spi_error::busy);
    mmio.fail = true;
    EXPECT_FALSE(started->cancel().has_value());
    EXPECT_FALSE(selected.active);
    EXPECT_FALSE(started->take().has_value());
    mmio.fail = false;
    ASSERT_TRUE(spi.shutdown().has_value());
}

TEST(spi_device_backend, esp_external_select_disables_native_cs_and_waits_for_idle,
     "REQ-0028", "REQ-0045") {
    device_spi_mmio mmio;
    policy execution;
    device_select selected;
    esp_backend spi{mmio, execution};
    const auto fixed_cs = mcubsp::mcu::esp32p4::gpio_matrix_base + 0x558U
        + esp_binding::select_pin.index * 4U;
    mmio.set(fixed_cs, 0x5A5A5A5AU);
    ASSERT_TRUE(spi.initialize(esp_timing).has_value());
    EXPECT_EQ(mmio.value(fixed_cs), 0x5A5A5A5AU);
    EXPECT_EQ(mmio.value(esp_binding::base + 0x20U) & 0x3FU, 0x3FU);
    ASSERT_TRUE(spi.set_select_handler(&device_select::set, &selected).has_value());
    const std::array<std::uint16_t, 1> tx{0x12U};
    const std::array segments{mcubsp::driver::spi_segment{.transmit = tx}};
    auto started = spi.transaction(segments);
    ASSERT_TRUE(started.has_value());
    EXPECT_TRUE(selected.active);
    mmio.set(esp_binding::base + 0x3CU, 1U << 12U);
    EXPECT_FALSE(spi.handle_interrupt());
    EXPECT_TRUE(selected.active);
    mmio.set(esp_binding::base, 0U);
    ASSERT_TRUE(spi.handle_interrupt());
    EXPECT_FALSE(selected.active);
    ASSERT_TRUE(started->take().has_value());
    ASSERT_TRUE(spi.reconfigure(esp_timing, {
        .mode = mcubsp::driver::spi_mode::mode3}).has_value());
    EXPECT_EQ(mmio.value(esp_binding::base + 0x20U) & 0x3FU, 0x3FU);

    auto cancelled = spi.transaction(segments);
    ASSERT_TRUE(cancelled.has_value());
    mmio.fail = true;
    EXPECT_FALSE(cancelled->cancel().has_value());
    EXPECT_FALSE(selected.active);
    EXPECT_FALSE(cancelled->take().has_value());
    mmio.fail = false;
    ASSERT_TRUE(spi.shutdown().has_value());
    EXPECT_EQ(mmio.value(fixed_cs), 0x5A5A5A5AU);
}

} // namespace
