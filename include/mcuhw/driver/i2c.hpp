#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <span>

#include <mcuhw/core/transfer.hpp>

namespace mcuhw::driver {

enum class i2c_error : std::uint8_t {
    invalid_clock,
    invalid_rate,
    invalid_rise_fall,
    unattainable_tolerance,
    invalid_address,
    empty_transfer,
    excessive_length,
    busy,
    bus_stuck,
    address_nack,
    data_nack,
    arbitration_lost,
    timeout,
    transport,
    not_initialized,
    already_owned,
    mmio,
};

struct i2c_address {
    std::uint8_t value;

    friend constexpr bool operator==(i2c_address, i2c_address) = default;
};

[[nodiscard]] constexpr std::expected<i2c_address, i2c_error>
make_i2c_address(std::uint8_t unshifted) noexcept {
    // 0000xxx and 1111xxx are reserved by the I2C specification.
    if (unshifted < 0x08U || unshifted > 0x77U) {
        return std::unexpected(i2c_error::invalid_address);
    }
    return i2c_address{unshifted};
}

struct i2c_timing_request {
    std::uint32_t clock_hz;
    std::uint32_t target_hz;
    std::uint32_t maximum_error_ppm;
    std::uint32_t rise_time_ns;
    std::uint32_t fall_time_ns;
};

struct i2c_timing {
    std::uint32_t clock_hz;
    std::uint32_t target_hz;
    std::uint32_t achieved_hz;
    std::uint32_t error_ppm;
    std::uint32_t rise_time_ns;
    std::uint32_t fall_time_ns;
    std::uint8_t clock_select;
    std::uint8_t brl;
    std::uint8_t brh;

    friend constexpr bool operator==(i2c_timing, i2c_timing) = default;
};

namespace i2c_detail {

[[nodiscard]] constexpr std::uint64_t divide_rounded(
    std::uint64_t numerator, std::uint64_t denominator) noexcept {
    return (numerator + denominator / 2U) / denominator;
}

[[nodiscard]] constexpr std::uint64_t ceil_divide(
    std::uint64_t numerator, std::uint64_t denominator) noexcept {
    return (numerator + denominator - 1U) / denominator;
}

} // namespace i2c_detail

// Initial RA IIC timing uses the manual's normal SCLE=1, NFE=0 mode.
// Rise/fall time and Standard/Fast-mode minimum high and low periods are
// enforced before selecting BRH/BRL.
// @req REQ-0011 REQ-0024
[[nodiscard]] constexpr std::expected<i2c_timing, i2c_error>
solve_i2c_timing(i2c_timing_request request) noexcept {
    if (request.clock_hz == 0U) {
        return std::unexpected(i2c_error::invalid_clock);
    }
    if (request.target_hz == 0U || request.target_hz > 400'000U) {
        return std::unexpected(i2c_error::invalid_rate);
    }
    const bool standard = request.target_hz <= 100'000U;
    const auto maximum_rise = standard ? 1'000U : 300U;
    if (request.rise_time_ns > maximum_rise
        || request.fall_time_ns > 300U) {
        return std::unexpected(i2c_error::invalid_rise_fall);
    }
    const std::uint32_t minimum_low_ns = standard ? 4'700U : 1'300U;
    const std::uint32_t minimum_high_ns = standard ? 4'000U : 600U;

    bool found{};
    i2c_timing best{};
    std::uint32_t divider = 1U;
    for (std::uint8_t cks = 0U; cks < 8U; ++cks) {
        const auto reference = request.clock_hz / divider;
        if (reference == 0U) {
            break;
        }
        const auto offset = cks == 0U ? 3U : 2U;
        const auto minimum_low_counts = i2c_detail::ceil_divide(
            static_cast<std::uint64_t>(minimum_low_ns) * reference,
            1'000'000'000ULL);
        const auto minimum_high_counts = i2c_detail::ceil_divide(
            static_cast<std::uint64_t>(minimum_high_ns) * reference,
            1'000'000'000ULL);
        const auto brl_min = minimum_low_counts > offset
            ? minimum_low_counts - offset : 1U;
        const auto brh_min = minimum_high_counts > offset
            ? minimum_high_counts - offset : 1U;
        if (brl_min > 32U || brh_min > 32U) {
            divider *= 2U;
            continue;
        }

        for (std::uint64_t low = brl_min; low <= 32U; ++low) {
            for (std::uint64_t high = brh_min; high <= 32U; ++high) {
                const auto wire_ns = static_cast<std::uint64_t>(
                    request.rise_time_ns) + request.fall_time_ns;
                const auto clock_ns = i2c_detail::divide_rounded(
                    (low + high + 2U * offset) * 1'000'000'000ULL,
                    reference);
                const auto period_ns = wire_ns + clock_ns;
                if (period_ns == 0U) {
                    continue;
                }
                const auto achieved = static_cast<std::uint32_t>(
                    i2c_detail::divide_rounded(1'000'000'000ULL,
                                               period_ns));
                const auto delta = achieved > request.target_hz
                    ? achieved - request.target_hz
                    : request.target_hz - achieved;
                const auto error = static_cast<std::uint32_t>(
                    i2c_detail::divide_rounded(
                        static_cast<std::uint64_t>(delta) * 1'000'000ULL,
                        request.target_hz));
                const i2c_timing candidate{
                    .clock_hz = request.clock_hz,
                    .target_hz = request.target_hz,
                    .achieved_hz = achieved,
                    .error_ppm = error,
                    .rise_time_ns = request.rise_time_ns,
                    .fall_time_ns = request.fall_time_ns,
                    .clock_select = cks,
                    .brl = static_cast<std::uint8_t>(low - 1U),
                    .brh = static_cast<std::uint8_t>(high - 1U),
                };
                if (!found || candidate.error_ppm < best.error_ppm
                    || (candidate.error_ppm == best.error_ppm
                        && cks < best.clock_select)) {
                    best = candidate;
                    found = true;
                }
            }
        }
        divider *= 2U;
    }
    if (!found || best.error_ppm > request.maximum_error_ppm) {
        return std::unexpected(i2c_error::unattainable_tolerance);
    }
    return best;
}

struct i2c_transfer_result {
    std::size_t bytes_written;
    std::size_t bytes_read;

    friend constexpr bool operator==(i2c_transfer_result,
                                     i2c_transfer_result) = default;
};

template<typename Backend>
class i2c_controller {
public:
    explicit i2c_controller(Backend& backend) noexcept : backend_(backend) {}

    [[nodiscard]] std::expected<void, i2c_error>
    initialize(i2c_timing timing) noexcept {
        return backend_.initialize(timing);
    }

    [[nodiscard]] auto write(i2c_address address,
                             std::span<const std::uint8_t> bytes) noexcept {
        return backend_.write(address, bytes);
    }

    [[nodiscard]] auto read(i2c_address address,
                            std::span<std::uint8_t> bytes) noexcept {
        return backend_.read(address, bytes);
    }

    [[nodiscard]] auto write_read(
        i2c_address address, std::span<const std::uint8_t> write_bytes,
        std::span<std::uint8_t> read_bytes) noexcept {
        return backend_.write_read(address, write_bytes, read_bytes);
    }

    [[nodiscard]] std::expected<void, i2c_error> shutdown() noexcept {
        return backend_.shutdown();
    }

private:
    Backend& backend_;
};

enum class register_byte_order : std::uint8_t { most_significant_first,
                                                 least_significant_first };

template<std::size_t Capacity, std::size_t RegisterBytes = 1U>
struct i2c_register_request {
    static_assert(Capacity > 0U);
    static_assert(RegisterBytes == 1U || RegisterBytes == 2U);
    std::array<std::uint8_t, RegisterBytes> command{};
    std::array<std::uint8_t, Capacity> data{};
};

// Address binding is a value-only view; it does not duplicate bus ownership.
// The request object owns every byte touched by the async operation and must
// outlive its returned transfer handle.
template<typename Bus, std::uint8_t Address>
// @req REQ-0018 REQ-0026
class i2c_device {
    static_assert(Address >= 0x08U && Address <= 0x77U);

public:
    explicit i2c_device(Bus& bus) noexcept : bus_(bus) {}

    template<std::size_t Capacity, std::size_t RegisterBytes>
    // @req REQ-0026
    [[nodiscard]] auto read_register(
        std::uint16_t register_address,
        i2c_register_request<Capacity, RegisterBytes>& request,
        std::size_t length,
        register_byte_order order = register_byte_order::most_significant_first)
        noexcept {
        using return_type = decltype(bus_.write_read(
            i2c_address{Address}, std::span<const std::uint8_t>{},
            std::span<std::uint8_t>{}));
        if (length > Capacity) {
            return return_type{std::unexpected(typename return_type::error_type{
                .kind = core::transfer_error_kind::failure,
                .cause = i2c_error::excessive_length})};
        }
        if constexpr (RegisterBytes == 1U) {
            request.command[0] = static_cast<std::uint8_t>(register_address);
        } else if (order == register_byte_order::most_significant_first) {
            request.command[0] = static_cast<std::uint8_t>(register_address >> 8U);
            request.command[1] = static_cast<std::uint8_t>(register_address);
        } else {
            request.command[0] = static_cast<std::uint8_t>(register_address);
            request.command[1] = static_cast<std::uint8_t>(register_address >> 8U);
        }
        return bus_.write_read(i2c_address{Address}, request.command,
                               std::span{request.data}.first(length));
    }

private:
    Bus& bus_;
};

} // namespace mcuhw::driver
