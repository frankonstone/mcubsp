#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <span>

namespace mcuhw::driver {

enum class sdmmc_error : std::uint8_t {
    invalid_argument,
    already_initialized,
    not_initialized,
    host_initialization,
    command_timeout,
    response_crc,
    invalid_response,
    card_not_ready,
    unsupported_card,
    out_of_range,
    data_timeout,
    data_crc,
    dma,
    mmio,
};

enum class sdmmc_response_type : std::uint8_t {
    none,
    short_crc,
    short_no_crc,
    long_crc,
};

struct sdmmc_command {
    std::uint8_t index{};
    std::uint32_t argument{};
    sdmmc_response_type response{sdmmc_response_type::none};
    bool send_initial_clocks{};
};

struct sdmmc_response {
    std::array<std::uint32_t, 4> words{};
};

struct sd_card_info {
    std::uint32_t ocr{};
    std::uint16_t relative_address{};
    bool high_capacity{};
    bool four_bit{};
    std::uint64_t sector_count{};
    std::uint8_t manufacturer_id{};
    std::uint16_t oem_id{};
    std::array<char, 6> product_name{};
    std::uint8_t revision{};
    std::uint32_t serial{};
    std::uint16_t manufacturing_date{};
    std::array<std::uint32_t, 4> raw_cid{};
    std::array<std::uint32_t, 4> raw_csd{};
};

[[nodiscard]] constexpr std::uint32_t sd_response_bits(
    const std::array<std::uint32_t, 4>& response, unsigned start,
    unsigned length) noexcept {
    if (length == 0U || length > 32U || start >= 128U
        || start + length > 128U) {
        return 0U;
    }
    const auto word = start / 32U;
    const auto shift = start % 32U;
    const auto mask = length == 32U
        ? 0xFFFF'FFFFU
        : static_cast<std::uint32_t>((std::uint64_t{1U} << length) - 1U);
    const auto right = response[word] >> shift;
    const auto left = length + shift <= 32U
        ? 0U
        : response[word + 1U] << (32U - shift);
    return (left | right) & mask;
}

[[nodiscard]] constexpr std::expected<std::uint64_t, sdmmc_error>
sd_sector_count(const std::array<std::uint32_t, 4>& csd) noexcept {
    const auto version = sd_response_bits(csd, 126U, 2U);
    if (version == 1U) {
        return (static_cast<std::uint64_t>(sd_response_bits(csd, 48U, 22U))
                + 1U) << 10U;
    }
    if (version != 0U) {
        return std::unexpected(sdmmc_error::unsupported_card);
    }
    const auto read_block_length = sd_response_bits(csd, 80U, 4U);
    if (read_block_length > 11U) {
        return std::unexpected(sdmmc_error::unsupported_card);
    }
    auto blocks = (static_cast<std::uint64_t>(
                       sd_response_bits(csd, 62U, 12U))
                   + 1U)
        << (sd_response_bits(csd, 47U, 3U) + 2U);
    const auto block_bytes = std::uint64_t{1U} << read_block_length;
    return blocks * block_bytes / 512U;
}

// Host must provide initialize(), shutdown(), command(), read_data(), write_data(),
// set_bus_width_4(), and set_transfer_clock(). Delay provides delay_us().
// The interface is synchronous and bounded so it is valid before a scheduler
// starts and requires neither allocation nor hidden worker state.
template<typename Host, typename Delay>
// @req REQ-0003 REQ-0014 REQ-0016 REQ-0019 REQ-0034 REQ-0037
class sd_card {
public:
    sd_card(Host& host, Delay& delay) noexcept : host_(host), delay_(delay) {}

    [[nodiscard]] std::expected<sd_card_info, sdmmc_error>
    initialize() noexcept {
        if (initialized_) {
            return std::unexpected(sdmmc_error::already_initialized);
        }
        if (!host_.initialize()) {
            return std::unexpected(sdmmc_error::host_initialization);
        }
        delay_.delay_us(10'000U);
        if (!send({0U, 0U, sdmmc_response_type::none, true})) {
            return fail(sdmmc_error::host_initialization);
        }
        delay_.delay_us(1'000U);

        const auto interface = send(
            {8U, 0x1AAU, sdmmc_response_type::short_crc, false});
        if (!interface || (interface->words[0] & 0xFFFU) != 0x1AAU) {
            return fail(interface ? sdmmc_error::invalid_response
                                  : interface.error());
        }

        constexpr std::uint32_t operating_conditions =
            0x00FF'8000U | (1U << 30U);
        bool ready{};
        for (std::size_t attempt = 0U; attempt < 1'000U; ++attempt) {
            const auto app = send(
                {55U, 0U, sdmmc_response_type::short_crc, false});
            if (!app || (app->words[0] & (1U << 5U)) == 0U) {
                return fail(app ? sdmmc_error::invalid_response : app.error());
            }
            const auto condition = send(
                {41U, operating_conditions,
                 sdmmc_response_type::short_no_crc, false});
            if (!condition) return fail(condition.error());
            info_.ocr = condition->words[0];
            if ((info_.ocr & (1U << 31U)) != 0U) {
                ready = true;
                break;
            }
            delay_.delay_us(1'000U);
        }
        if (!ready) return fail(sdmmc_error::card_not_ready);
        info_.high_capacity = (info_.ocr & (1U << 30U)) != 0U;

        const auto cid = send(
            {2U, 0U, sdmmc_response_type::long_crc, false});
        if (!cid) return fail(cid.error());
        info_.raw_cid = cid->words;
        decode_cid();

        const auto address = send(
            {3U, 0U, sdmmc_response_type::short_crc, false});
        if (!address) return fail(address.error());
        info_.relative_address = static_cast<std::uint16_t>(
            address->words[0] >> 16U);
        if (info_.relative_address == 0U) {
            return fail(sdmmc_error::invalid_response);
        }

        const auto csd = send(
            {9U, static_cast<std::uint32_t>(info_.relative_address) << 16U,
             sdmmc_response_type::long_crc, false});
        if (!csd) return fail(csd.error());
        info_.raw_csd = csd->words;
        const auto sectors = sd_sector_count(info_.raw_csd);
        if (!sectors) return fail(sectors.error());
        info_.sector_count = *sectors;

        if (!send({7U,
                   static_cast<std::uint32_t>(info_.relative_address) << 16U,
                   sdmmc_response_type::short_crc, false})) {
            return fail(sdmmc_error::invalid_response);
        }
        if (!info_.high_capacity
            && !send({16U, 512U, sdmmc_response_type::short_crc, false})) {
            return fail(sdmmc_error::invalid_response);
        }

        const auto app = send(
            {55U, static_cast<std::uint32_t>(info_.relative_address) << 16U,
             sdmmc_response_type::short_crc, false});
        if (!app || (app->words[0] & (1U << 5U)) == 0U
            || !send({6U, 2U, sdmmc_response_type::short_crc, false})
            || !host_.set_bus_width_4()
            || !host_.set_transfer_clock()) {
            return fail(sdmmc_error::invalid_response);
        }
        info_.four_bit = true;
        initialized_ = true;
        return info_;
    }

    [[nodiscard]] std::expected<void, sdmmc_error>
    read_sector(std::uint32_t sector, std::span<std::byte, 512> data) noexcept {
        if (!initialized_) {
            return std::unexpected(sdmmc_error::not_initialized);
        }
        if (sector >= info_.sector_count) {
            return std::unexpected(sdmmc_error::out_of_range);
        }
        const auto argument = info_.high_capacity
            ? sector
            : static_cast<std::uint32_t>(sector * 512U);
        const auto result = host_.read_data(
            {17U, argument, sdmmc_response_type::short_crc, false}, data);
        if (!result) return std::unexpected(result.error());
        return {};
    }

    [[nodiscard]] std::expected<void, sdmmc_error>
    write_sector(std::uint32_t sector,
                 std::span<const std::byte, 512> data) noexcept {
        if (!initialized_) {
            return std::unexpected(sdmmc_error::not_initialized);
        }
        if (sector >= info_.sector_count) {
            return std::unexpected(sdmmc_error::out_of_range);
        }
        const auto argument = info_.high_capacity
            ? sector
            : static_cast<std::uint32_t>(sector * 512U);
        const auto written = host_.write_data(
            {24U, argument, sdmmc_response_type::short_crc, false}, data);
        if (!written) return std::unexpected(written.error());

        for (std::size_t attempt = 0U; attempt < 1'000U; ++attempt) {
            const auto status = send(
                {13U,
                 static_cast<std::uint32_t>(info_.relative_address) << 16U,
                 sdmmc_response_type::short_crc, false});
            if (!status) return std::unexpected(status.error());
            const auto ready_for_data = (status->words[0] & (1U << 8U)) != 0U;
            const auto state = (status->words[0] >> 9U) & 0xFU;
            if (ready_for_data && state == 4U) return {};
            delay_.delay_us(1'000U);
        }
        return std::unexpected(sdmmc_error::card_not_ready);
    }

    [[nodiscard]] const sd_card_info& info() const noexcept { return info_; }

    [[nodiscard]] std::expected<void, sdmmc_error> shutdown() noexcept {
        if (!initialized_) {
            return std::unexpected(sdmmc_error::not_initialized);
        }
        initialized_ = false;
        return host_.shutdown();
    }

private:
    [[nodiscard]] std::expected<sdmmc_response, sdmmc_error>
    send(sdmmc_command command) noexcept {
        return host_.command(command);
    }

    [[nodiscard]] std::expected<sd_card_info, sdmmc_error>
    fail(sdmmc_error error) noexcept {
        static_cast<void>(host_.shutdown());
        return std::unexpected(error);
    }

    void decode_cid() noexcept {
        info_.manufacturer_id = static_cast<std::uint8_t>(
            sd_response_bits(info_.raw_cid, 120U, 8U));
        info_.oem_id = static_cast<std::uint16_t>(
            sd_response_bits(info_.raw_cid, 104U, 16U));
        for (unsigned index = 0U; index < 5U; ++index) {
            info_.product_name[index] = static_cast<char>(
                sd_response_bits(info_.raw_cid, 96U - index * 8U, 8U));
        }
        info_.product_name[5] = '\0';
        info_.revision = static_cast<std::uint8_t>(
            sd_response_bits(info_.raw_cid, 56U, 8U));
        info_.serial = sd_response_bits(info_.raw_cid, 24U, 32U);
        info_.manufacturing_date = static_cast<std::uint16_t>(
            sd_response_bits(info_.raw_cid, 8U, 12U));
    }

    Host& host_;
    Delay& delay_;
    sd_card_info info_{};
    bool initialized_{};
};

} // namespace mcuhw::driver
