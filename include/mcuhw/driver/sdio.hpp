#pragma once

#include <cstddef>
#include <cstdint>
#include <expected>

#include <mcuhw/driver/sdmmc.hpp>

namespace mcuhw::driver {

struct sdio_device_info {
    std::uint32_t ocr{};
    std::uint32_t cis_pointer{};
    std::uint16_t relative_address{};
    std::uint16_t manufacturer_id{};
    std::uint16_t card_id{};
    std::uint8_t function_count{};
    std::uint8_t cccr_revision{};
    std::uint8_t card_capabilities{};
    bool memory_present{};
    bool function1_ready{};
    bool four_bit{};
};

// Scheduler-independent SDIO enumeration for an IO-only coprocessor. Host
// supplies the same command/clock contract as sd_card; Delay supplies
// delay_us(). The sequence is bounded and uses no allocation.
template<typename Host, typename Delay>
// @req REQ-0003 REQ-0014 REQ-0016 REQ-0019 REQ-0034 REQ-0037
class sdio_device {
public:
    sdio_device(Host& host, Delay& delay) noexcept
        : host_(host), delay_(delay) {}

    [[nodiscard]] std::expected<sdio_device_info, sdmmc_error>
    initialize() noexcept {
        if (initialized_) {
            return std::unexpected(sdmmc_error::already_initialized);
        }
        info_ = {};
        if (!host_.initialize()) {
            return std::unexpected(sdmmc_error::host_initialization);
        }
        delay_.delay_us(10'000U);
        if (!send({0U, 0U, sdmmc_response_type::none, true})) {
            return fail(sdmmc_error::host_initialization);
        }

        const auto query = send(
            {5U, 0U, sdmmc_response_type::short_no_crc, false});
        if (!query) return fail(query.error());
        info_.ocr = query->words[0];
        info_.function_count = static_cast<std::uint8_t>(
            (info_.ocr >> 28U) & 0x7U);
        info_.memory_present = (info_.ocr & (1U << 27U)) != 0U;
        const auto voltage = info_.ocr & 0x00FF'FFF0U;
        if (info_.function_count == 0U || voltage == 0U) {
            return fail(sdmmc_error::unsupported_card);
        }

        bool ready{};
        for (std::size_t attempt = 0U; attempt < 100U; ++attempt) {
            const auto condition = send(
                {5U, voltage, sdmmc_response_type::short_no_crc, false});
            if (!condition) return fail(condition.error());
            info_.ocr = condition->words[0];
            if ((info_.ocr & (1U << 31U)) != 0U) {
                ready = true;
                break;
            }
            delay_.delay_us(1'000U);
        }
        if (!ready) return fail(sdmmc_error::card_not_ready);

        const auto address = send(
            {3U, 0U, sdmmc_response_type::short_crc, false});
        if (!address) return fail(address.error());
        info_.relative_address = static_cast<std::uint16_t>(
            address->words[0] >> 16U);
        if (info_.relative_address == 0U) {
            return fail(sdmmc_error::invalid_response);
        }
        if (!send({7U,
                   static_cast<std::uint32_t>(info_.relative_address) << 16U,
                   sdmmc_response_type::short_crc, false})) {
            return fail(sdmmc_error::invalid_response);
        }

        const auto revision = read_byte(0U, cccr_revision);
        const auto capabilities = read_byte(0U, cccr_card_capabilities);
        if (!revision || !capabilities) {
            return fail(revision ? capabilities.error() : revision.error());
        }
        info_.cccr_revision = *revision;
        info_.card_capabilities = *capabilities;
        if (!read_cis_pointer() || !read_identity()) {
            return fail(sdmmc_error::invalid_response);
        }

        const auto enabled = read_byte(0U, cccr_function_enable);
        if (!enabled
            || !write_byte(0U, cccr_function_enable,
                           static_cast<std::uint8_t>(*enabled | function1_bit))) {
            return fail(enabled ? sdmmc_error::invalid_response
                                : enabled.error());
        }
        for (std::size_t attempt = 0U; attempt < 10U; ++attempt) {
            const auto function_ready = read_byte(0U, cccr_function_ready);
            if (!function_ready) return fail(function_ready.error());
            if ((*function_ready & function1_bit) != 0U) {
                info_.function1_ready = true;
                break;
            }
            delay_.delay_us(10'000U);
        }
        if (!info_.function1_ready) {
            return fail(sdmmc_error::card_not_ready);
        }
        if (!set_block_size(0U, 512U) || !set_block_size(1U, 512U)) {
            return fail(sdmmc_error::invalid_response);
        }

        const auto width = read_byte(0U, cccr_bus_width);
        if (!width
            || !write_byte(0U, cccr_bus_width,
                           static_cast<std::uint8_t>((*width & ~0x3U) | 0x2U))
            || !host_.set_bus_width_4() || !host_.set_transfer_clock()) {
            return fail(width ? sdmmc_error::invalid_response : width.error());
        }
        info_.four_bit = true;
        initialized_ = true;
        return info_;
    }

    [[nodiscard]] const sdio_device_info& info() const noexcept {
        return info_;
    }

    [[nodiscard]] std::expected<void, sdmmc_error> shutdown() noexcept {
        if (!initialized_) {
            return std::unexpected(sdmmc_error::not_initialized);
        }
        initialized_ = false;
        return host_.shutdown();
    }

private:
    static constexpr std::uint32_t cccr_function_enable = 0x02U;
    static constexpr std::uint32_t cccr_function_ready = 0x03U;
    static constexpr std::uint32_t cccr_bus_width = 0x07U;
    static constexpr std::uint32_t cccr_card_capabilities = 0x08U;
    static constexpr std::uint32_t cccr_cis_pointer = 0x09U;
    static constexpr std::uint32_t cccr_revision = 0x00U;
    static constexpr std::uint32_t fbr_size = 0x100U;
    static constexpr std::uint32_t block_size_low = 0x10U;
    static constexpr std::uint32_t block_size_high = 0x11U;
    static constexpr std::uint8_t function1_bit = 1U << 1U;
    static constexpr std::uint16_t r5_error_mask = 0xCB00U;

    [[nodiscard]] std::expected<sdmmc_response, sdmmc_error>
    send(sdmmc_command command) noexcept {
        return host_.command(command);
    }

    [[nodiscard]] std::expected<std::uint8_t, sdmmc_error>
    read_byte(std::uint8_t function, std::uint32_t address) noexcept {
        const auto argument = (static_cast<std::uint32_t>(function) << 28U)
            | ((address & 0x1FFFFU) << 9U);
        const auto response = send(
            {52U, argument, sdmmc_response_type::short_crc, false});
        if (!response) return std::unexpected(response.error());
        if ((response->words[0] & r5_error_mask) != 0U) {
            return std::unexpected(sdmmc_error::invalid_response);
        }
        return static_cast<std::uint8_t>(response->words[0] & 0xFFU);
    }

    [[nodiscard]] bool write_byte(std::uint8_t function,
                                  std::uint32_t address,
                                  std::uint8_t value) noexcept {
        const auto argument = (1U << 31U)
            | (static_cast<std::uint32_t>(function) << 28U) | (1U << 27U)
            | ((address & 0x1FFFFU) << 9U) | value;
        const auto response = send(
            {52U, argument, sdmmc_response_type::short_crc, false});
        return response && (response->words[0] & r5_error_mask) == 0U
            && (response->words[0] & 0xFFU) == value;
    }

    [[nodiscard]] bool read_cis_pointer() noexcept {
        info_.cis_pointer = 0U;
        for (std::uint32_t index = 0U; index < 3U; ++index) {
            const auto value = read_byte(0U, cccr_cis_pointer + index);
            if (!value) return false;
            info_.cis_pointer |= static_cast<std::uint32_t>(*value)
                << (index * 8U);
        }
        return info_.cis_pointer >= 0x1000U
            && info_.cis_pointer <= 0x17FFFU;
    }

    [[nodiscard]] bool read_identity() noexcept {
        auto address = info_.cis_pointer;
        for (std::size_t tuple = 0U; tuple < 64U
             && address <= 0x17FFFU; ++tuple) {
            const auto code = read_byte(0U, address);
            if (!code) return false;
            if (*code == 0xFFU) return false;
            if (*code == 0x00U) {
                ++address;
                continue;
            }
            const auto size = read_byte(0U, address + 1U);
            if (!size) return false;
            if (*code == 0x20U && *size >= 4U) {
                const auto manf_low = read_byte(0U, address + 2U);
                const auto manf_high = read_byte(0U, address + 3U);
                const auto card_low = read_byte(0U, address + 4U);
                const auto card_high = read_byte(0U, address + 5U);
                if (!manf_low || !manf_high || !card_low || !card_high) {
                    return false;
                }
                info_.manufacturer_id = static_cast<std::uint16_t>(
                    *manf_low | static_cast<std::uint16_t>(*manf_high) << 8U);
                info_.card_id = static_cast<std::uint16_t>(
                    *card_low | static_cast<std::uint16_t>(*card_high) << 8U);
                return true;
            }
            address += 2U + *size;
        }
        return false;
    }

    [[nodiscard]] bool set_block_size(std::uint8_t function,
                                      std::uint16_t size) noexcept {
        const auto base = static_cast<std::uint32_t>(function) * fbr_size;
        if (!write_byte(0U, base + block_size_low,
                        static_cast<std::uint8_t>(size & 0xFFU))
            || !write_byte(0U, base + block_size_high,
                           static_cast<std::uint8_t>(size >> 8U))) {
            return false;
        }
        const auto low = read_byte(0U, base + block_size_low);
        const auto high = read_byte(0U, base + block_size_high);
        return low && high
            && static_cast<std::uint16_t>(
                   *low | static_cast<std::uint16_t>(*high) << 8U) == size;
    }

    [[nodiscard]] std::expected<sdio_device_info, sdmmc_error>
    fail(sdmmc_error error) noexcept {
        static_cast<void>(host_.shutdown());
        return std::unexpected(error);
    }

    Host& host_;
    Delay& delay_;
    sdio_device_info info_{};
    bool initialized_{};
};

} // namespace mcuhw::driver
