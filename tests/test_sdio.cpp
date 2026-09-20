#include <array>
#include <cstddef>
#include <cstdint>
#include <expected>

#include <mcutest/mcutest.hpp>

#include <mcuhw/driver/sdio.hpp>

namespace {

using mcuhw::driver::sdio_device;
using mcuhw::driver::sdmmc_command;
using mcuhw::driver::sdmmc_error;
using mcuhw::driver::sdmmc_response;

class fake_delay {
public:
    void delay_us(std::uint32_t duration) noexcept { elapsed_ += duration; }
    [[nodiscard]] std::uint32_t elapsed() const noexcept { return elapsed_; }
private:
    std::uint32_t elapsed_{};
};

class fake_sdio_host {
public:
    fake_sdio_host() noexcept {
        registers_[0x00U] = 0x32U;
        registers_[0x08U] = 0x00U;
        registers_[0x09U] = 0x00U;
        registers_[0x0AU] = 0x10U;
        registers_[0x0BU] = 0x00U;
        registers_[0x1000U] = 0x01U;
        registers_[0x1001U] = 0x03U;
        registers_[0x1002U] = 0xD9U;
        registers_[0x1003U] = 0x01U;
        registers_[0x1004U] = 0xFFU;
        registers_[0x1005U] = 0x20U;
        registers_[0x1006U] = 0x04U;
        registers_[0x1007U] = 0x92U;
        registers_[0x1008U] = 0x00U;
        registers_[0x1009U] = 0x66U;
        registers_[0x100AU] = 0x66U;
        registers_[0x100BU] = 0xFFU;
    }

    [[nodiscard]] bool initialize() noexcept { return true; }
    [[nodiscard]] std::expected<void, sdmmc_error> shutdown() noexcept {
        ++shutdown_count_;
        return {};
    }
    [[nodiscard]] std::expected<sdmmc_response, sdmmc_error>
    command(sdmmc_command command) noexcept {
        ++command_count_;
        sdmmc_response response{};
        if (command.index == 5U) {
            response.words[0] = (1U << 28U) | 0x00FF'8000U;
            if (command.argument != 0U && op_condition_count_++ != 0U) {
                response.words[0] |= 1U << 31U;
            }
        } else if (command.index == 3U) {
            response.words[0] = 0x4321'0000U;
        } else if (command.index == 52U) {
            const auto address = (command.argument >> 9U) & 0x1FFFFU;
            if ((command.argument & (1U << 31U)) != 0U) {
                registers_[address] = static_cast<std::uint8_t>(command.argument);
                if (address == 0x02U
                    && (registers_[address] & (1U << 1U)) != 0U) {
                    registers_[0x03U] |= 1U << 1U;
                }
            }
            response.words[0] = registers_[address];
        }
        return response;
    }
    [[nodiscard]] bool set_bus_width_4() noexcept {
        width_ = 4U;
        return true;
    }
    [[nodiscard]] bool set_transfer_clock() noexcept {
        transfer_clock_ = true;
        return true;
    }
    [[nodiscard]] std::uint8_t reg(std::size_t address) const noexcept {
        return registers_[address];
    }
    [[nodiscard]] unsigned width() const noexcept { return width_; }
    [[nodiscard]] bool transfer_clock() const noexcept {
        return transfer_clock_;
    }
    [[nodiscard]] std::size_t shutdown_count() const noexcept {
        return shutdown_count_;
    }
    [[nodiscard]] std::size_t command_count() const noexcept {
        return command_count_;
    }
private:
    std::array<std::uint8_t, 0x18000> registers_{};
    std::size_t op_condition_count_{};
    std::size_t shutdown_count_{};
    std::size_t command_count_{};
    unsigned width_{};
    bool transfer_clock_{};
};

TEST(sdio, enumerates_cis_enables_function_and_switches_to_four_bit,
     "REQ-0003", "REQ-0014", "REQ-0016", "REQ-0019", "REQ-0034",
     "REQ-0037") {
    fake_sdio_host host;
    fake_delay delay;
    sdio_device device{host, delay};
    const auto initialized = device.initialize();
    ASSERT_TRUE(initialized.has_value());
    EXPECT_EQ(initialized->function_count, 1U);
    EXPECT_EQ(initialized->relative_address, 0x4321U);
    EXPECT_EQ(initialized->cccr_revision, 0x32U);
    EXPECT_EQ(initialized->cis_pointer, 0x1000U);
    EXPECT_EQ(initialized->manufacturer_id, 0x0092U);
    EXPECT_EQ(initialized->card_id, 0x6666U);
    EXPECT_TRUE(initialized->function1_ready);
    EXPECT_TRUE(initialized->four_bit);
    EXPECT_TRUE((host.reg(0x02U) & (1U << 1U)) != 0U);
    EXPECT_EQ(host.reg(0x10U), 0U);
    EXPECT_EQ(host.reg(0x11U), 2U);
    EXPECT_EQ(host.reg(0x110U), 0U);
    EXPECT_EQ(host.reg(0x111U), 2U);
    EXPECT_EQ(host.reg(0x07U) & 0x3U, 0x2U);
    EXPECT_EQ(host.width(), 4U);
    EXPECT_TRUE(host.transfer_clock());
    EXPECT_TRUE(host.command_count() > 20U);
    EXPECT_TRUE(delay.elapsed() >= 11'000U);
    EXPECT_TRUE(device.shutdown().has_value());
}

class no_function_host : public fake_sdio_host {
public:
    [[nodiscard]] std::expected<sdmmc_response, sdmmc_error>
    command(sdmmc_command command) noexcept {
        if (command.index == 5U) {
            sdmmc_response response{};
            response.words[0] = 0x00FF'8000U;
            return response;
        }
        return fake_sdio_host::command(command);
    }
};

TEST(sdio, rejects_a_response_without_io_functions_and_releases_host,
     "REQ-0014", "REQ-0019", "REQ-0037") {
    no_function_host host;
    fake_delay delay;
    sdio_device device{host, delay};
    const auto initialized = device.initialize();
    ASSERT_FALSE(initialized.has_value());
    EXPECT_TRUE(initialized.error() == sdmmc_error::unsupported_card);
    EXPECT_EQ(host.shutdown_count(), 1U);
}

} // namespace
