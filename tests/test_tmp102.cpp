// @req-file REQ-0045 REQ-0046
#include <array>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <span>
#include <utility>
#include <mcutest/mcutest.hpp>
#include <mcuhw/device/sensor/tmp102.hpp>

namespace {
using namespace std::chrono_literals;
enum class bus_error { nack, timeout, invalid_timeout, moved };
struct limit { std::uint32_t value; };
struct transfer_result { std::size_t bytes_written; std::size_t bytes_read; };
struct fake_state {
    std::uint16_t config{0x60A0U};
    std::uint16_t temperature{0x1900U};
    std::uint32_t now{};
    std::uint32_t last_deadline{};
    std::uint32_t transfer_cost{1U};
    unsigned deadlines{};
    unsigned reads{};
    unsigned leases{};
    bool nack{};
    bool short_read{};
    bool combined{true};
};
class fake_endpoint {
public:
    using error_type = bus_error;
    using deadline_type = limit;
    explicit fake_endpoint(fake_state& state) noexcept : state_(&state) { ++state_->leases; }
    fake_endpoint(const fake_endpoint&) = delete;
    fake_endpoint& operator=(const fake_endpoint&) = delete;
    fake_endpoint(fake_endpoint&& other) noexcept : state_(std::exchange(other.state_, nullptr)) {}
    ~fake_endpoint() { if (state_) --state_->leases; }
    template<class Duration>
    std::expected<limit, bus_error> deadline_after(Duration timeout) noexcept {
        if (!state_) return std::unexpected(bus_error::moved);
        ++state_->deadlines;
        const auto count = std::chrono::duration_cast<std::chrono::milliseconds>(timeout).count();
        if (count < 0) return std::unexpected(bus_error::invalid_timeout);
        return limit{state_->now + static_cast<std::uint32_t>(count)};
    }
    std::expected<transfer_result, bus_error> write_read_until(
        std::span<const std::uint8_t> output, std::span<std::uint8_t> input,
        limit deadline) noexcept {
        if (!state_) return std::unexpected(bus_error::moved);
        state_->last_deadline = deadline.value;
        ++state_->reads;
        state_->combined = state_->combined && output.size() == 1U && input.size() == 2U;
        if (state_->nack) return std::unexpected(bus_error::nack);
        if (state_->now + state_->transfer_cost >= deadline.value) {
            state_->now = deadline.value;
            return std::unexpected(bus_error::timeout);
        }
        state_->now += state_->transfer_cost;
        const auto value = output[0] == 1U ? state_->config : state_->temperature;
        input[0] = static_cast<std::uint8_t>(value >> 8U);
        input[1] = static_cast<std::uint8_t>(value);
        return transfer_result{1U, state_->short_read ? 1U : 2U};
    }
private:
    fake_state* state_;
};
using sensor = mcuhw::device::tmp102<fake_endpoint>;
using errc = mcuhw::device::tmp102_errc;

TEST(tmp102, owns_endpoint_and_preserves_configuration, "REQ-0046") {
    fake_state state;
    state.config = 0x7FB0U; // alternate alert, shutdown, extended settings
    {
        sensor temperature{fake_endpoint{state}};
        EXPECT_EQ(state.leases, 1U);
        EXPECT_TRUE(temperature.read().error().code == errc::not_started);
        EXPECT_EQ(state.reads, 0U);
        ASSERT_TRUE(temperature.begin());
        EXPECT_EQ(state.config, 0x7FB0U);
        auto moved = std::move(temperature);
        EXPECT_FALSE(temperature.initialized());
        EXPECT_TRUE(moved.initialized());
        EXPECT_EQ(state.leases, 1U);
    }
    EXPECT_EQ(state.leases, 0U);
}

TEST(tmp102, decodes_signed_normal_and_extended_samples_using_sample_mode, "REQ-0046") {
    fake_state state;
    sensor temperature{fake_endpoint{state}};
    ASSERT_TRUE(temperature.begin());
    constexpr std::array<std::pair<std::uint16_t, std::int16_t>, 7U> samples{{
        {0x1900U, 400}, {0xE700U, -400}, {0xFFF0U, -1}, {0x7FF0U, 2047},
        {0x4B01U, 2400}, {0xE481U, -880}, {0xFFF9U, -1},
    }};
    for (const auto [raw, counts] : samples) {
        state.temperature = raw;
        const auto read = temperature.read(20ms);
        ASSERT_TRUE(read);
        EXPECT_EQ(read->celsius_sixteenths, counts);
        EXPECT_EQ(read->micro_celsius(), static_cast<std::int32_t>(counts) * 62'500);
    }
    EXPECT_TRUE(state.combined);
}

TEST(tmp102, invalid_configuration_short_response_and_reserved_temperature_bits_fail, "REQ-0046") {
    fake_state state;
    sensor temperature{fake_endpoint{state}};
    state.config = 0U;
    EXPECT_TRUE(temperature.begin().error().code == errc::invalid_response);
    EXPECT_FALSE(temperature.initialized());
    state.config = 0x60A0U;
    ASSERT_TRUE(temperature.begin());
    state.short_read = true;
    EXPECT_TRUE(temperature.read().error().code == errc::invalid_response);
    state.short_read = false;
    state.temperature = 0x1902U;
    EXPECT_TRUE(temperature.read().error().code == errc::invalid_response);
    state.temperature = 0x4B03U;
    EXPECT_TRUE(temperature.read().error().code == errc::invalid_response);
}

TEST(tmp102, transport_failure_preserves_cause_and_one_deadline_covers_read, "REQ-0046") {
    fake_state state;
    sensor temperature{fake_endpoint{state}};
    ASSERT_TRUE(temperature.begin());
    state.now = 100U;
    const auto deadline_count = state.deadlines;
    ASSERT_TRUE(temperature.read(20ms));
    EXPECT_EQ(state.last_deadline, 120U);
    EXPECT_EQ(state.deadlines, deadline_count + 1U);
    state.nack = true;
    const auto failed = temperature.read();
    ASSERT_FALSE(failed);
    EXPECT_TRUE(failed.error().code == errc::transport);
    EXPECT_TRUE(failed.error().transport == bus_error::nack);
    state.nack = false;
    state.transfer_cost = 10U;
    const auto timed_out = temperature.read(5ms);
    ASSERT_FALSE(timed_out);
    EXPECT_TRUE(timed_out.error().transport == bus_error::timeout);
    EXPECT_TRUE(temperature.read(-1ms).error().transport == bus_error::invalid_timeout);
}
} // namespace
