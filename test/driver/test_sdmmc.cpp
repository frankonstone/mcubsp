#include <array>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <span>

#include <mcutest/mcutest.hpp>

#include <mcuhw/driver/sdmmc.hpp>

namespace {

using mcuhw::driver::sdmmc_command;
using mcuhw::driver::sdmmc_error;
using mcuhw::driver::sdmmc_response;

class fake_delay {
public:
    void delay_us(std::uint32_t value) noexcept { elapsed_ += value; }
    [[nodiscard]] std::uint32_t elapsed() const noexcept { return elapsed_; }
private:
    std::uint32_t elapsed_{};
};

class fake_host {
public:
    [[nodiscard]] bool initialize() noexcept {
        ++initialize_count_;
        return !fail_initialize_;
    }
    [[nodiscard]] std::expected<void, sdmmc_error> shutdown() noexcept {
        ++shutdown_count_;
        return {};
    }
    [[nodiscard]] std::expected<sdmmc_response, sdmmc_error>
    command(sdmmc_command command) noexcept {
        if (count_ < commands_.size()) commands_[count_++] = command;
        sdmmc_response response{};
        switch (command.index) {
        case 8U: response.words[0] = 0x1AAU; break;
        case 55U: response.words[0] = 1U << 5U; break;
        case 41U:
            response.words[0] = (acmd41_count_++ == 0U ? 0U : 1U << 31U)
                | (1U << 30U) | 0x00FF'8000U;
            break;
        case 2U:
            response.words = cid_;
            break;
        case 3U: response.words[0] = 0x1234'0000U; break;
        case 9U:
            response.words = csd_;
            break;
        case 13U:
            response.words[0] = (1U << 8U) | (4U << 9U);
            break;
        default: break;
        }
        return response;
    }
    [[nodiscard]] std::expected<sdmmc_response, sdmmc_error>
    read_data(sdmmc_command command, std::span<std::byte, 512> data) noexcept {
        read_argument_ = command.argument;
        for (std::size_t index = 0U; index < data.size(); ++index) {
            data[index] = static_cast<std::byte>(index & 0xFFU);
        }
        return sdmmc_response{};
    }
    [[nodiscard]] std::expected<sdmmc_response, sdmmc_error>
    write_data(sdmmc_command command,
               std::span<const std::byte, 512> data) noexcept {
        write_argument_ = command.argument;
        written_first_byte_ = data[0];
        return sdmmc_response{};
    }
    [[nodiscard]] bool set_bus_width_4() noexcept {
        width_ = 4U;
        return true;
    }
    [[nodiscard]] bool set_transfer_clock() noexcept {
        transfer_clock_ = true;
        return true;
    }

    void fail_initialize() noexcept { fail_initialize_ = true; }
    [[nodiscard]] std::size_t command_count() const noexcept { return count_; }
    [[nodiscard]] const sdmmc_command& command_at(std::size_t index) const noexcept {
        return commands_[index];
    }
    [[nodiscard]] unsigned width() const noexcept { return width_; }
    [[nodiscard]] bool transfer_clock() const noexcept { return transfer_clock_; }
    [[nodiscard]] std::uint32_t read_argument() const noexcept {
        return read_argument_;
    }
    [[nodiscard]] std::uint32_t write_argument() const noexcept {
        return write_argument_;
    }
    [[nodiscard]] std::byte written_first_byte() const noexcept {
        return written_first_byte_;
    }
    [[nodiscard]] std::size_t shutdown_count() const noexcept {
        return shutdown_count_;
    }

private:
    static constexpr std::array<std::uint32_t, 4> cid_{
        0x1ABC'DE00U, 0x325A'0055U, 0x5445'5354U, 0x0000'5344U};
    // CSD structure v2, C_SIZE=0x3fff => 8 GiB / 512-byte sectors.
    static constexpr std::array<std::uint32_t, 4> csd_{
        0U, 0x3FFFU << 16U, 0U, 1U << 30U};
    std::array<sdmmc_command, 16> commands_{};
    std::size_t count_{};
    std::size_t initialize_count_{};
    std::size_t shutdown_count_{};
    std::size_t acmd41_count_{};
    std::uint32_t read_argument_{};
    std::uint32_t write_argument_{};
    std::byte written_first_byte_{};
    unsigned width_{};
    bool transfer_clock_{};
    bool fail_initialize_{};
};

TEST(sdmmc, decodes_capacity_and_initializes_native_sd_sequence,
     "REQ-0014", "REQ-0016", "REQ-0019", "REQ-0034", "REQ-0037") {
    fake_host host;
    fake_delay delay;
    mcuhw::driver::sd_card card{host, delay};

    const auto initialized = card.initialize();
    ASSERT_TRUE(initialized.has_value());
    EXPECT_TRUE(initialized->high_capacity);
    EXPECT_TRUE(initialized->four_bit);
    EXPECT_EQ(initialized->relative_address, 0x1234U);
    EXPECT_EQ(initialized->sector_count, 16'777'216ULL);
    EXPECT_EQ(host.width(), 4U);
    EXPECT_TRUE(host.transfer_clock());
    EXPECT_EQ(host.command_at(0U).index, 0U);
    EXPECT_TRUE(host.command_at(0U).send_initial_clocks);
    EXPECT_EQ(host.command_at(1U).index, 8U);
    EXPECT_EQ(host.command_at(2U).index, 55U);
    EXPECT_EQ(host.command_at(3U).index, 41U);
    EXPECT_TRUE(delay.elapsed() >= 11'000U);
}

TEST(sdmmc, exposes_bounded_sector_read_and_write_operations,
     "REQ-0016", "REQ-0019", "REQ-0034", "REQ-0037") {
    fake_host host;
    fake_delay delay;
    mcuhw::driver::sd_card card{host, delay};
    std::array<std::byte, 512> sector{};
    std::array<std::byte, 512> output{};
    output[0] = std::byte{0xA5U};
    EXPECT_TRUE(card.read_sector(0U, sector).error()
                == sdmmc_error::not_initialized);
    ASSERT_TRUE(card.initialize().has_value());
    EXPECT_TRUE(card.read_sector(7U, sector).has_value());
    EXPECT_EQ(host.read_argument(), 7U);
    EXPECT_EQ(std::to_integer<unsigned>(sector[255]), 255U);
    EXPECT_TRUE(card.read_sector(20'000'000U, sector).error()
                == sdmmc_error::out_of_range);
    EXPECT_TRUE(card.write_sector(9U, output).has_value());
    EXPECT_EQ(host.write_argument(), 9U);
    EXPECT_EQ(std::to_integer<unsigned>(host.written_first_byte()), 0xA5U);
    EXPECT_TRUE(card.write_sector(20'000'000U, output).error()
                == sdmmc_error::out_of_range);
    EXPECT_TRUE(card.shutdown().has_value());
}

TEST(sdmmc, cleans_up_failed_initialization,
     "REQ-0014", "REQ-0019", "REQ-0037") {
    fake_host host;
    host.fail_initialize();
    fake_delay delay;
    mcuhw::driver::sd_card card{host, delay};
    const auto initialized = card.initialize();
    ASSERT_FALSE(initialized.has_value());
    EXPECT_TRUE(initialized.error() == sdmmc_error::host_initialization);
    EXPECT_EQ(host.shutdown_count(), 0U);
}

} // namespace
