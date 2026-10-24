// @req-file REQ-0044 REQ-0046
#include <algorithm>
#include <array>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <span>
#include <type_traits>
#include <utility>

#include <mcutest/mcutest.hpp>
#include <mcubsp/device/ethernet/w5100.hpp>
#include <mcubsp/integration/mcuip/w5100.hpp>

namespace {
namespace io = mcubsp::io;
namespace reg = mcubsp::device::w5100_register;
namespace ex = mcubsp::execution;
using error = mcubsp::device::w5100_error;
using namespace std::chrono_literals;

struct fake_chip {
    std::array<std::uint8_t, 0x8000U> memory{};
    std::array<std::uint8_t, 4U> last_command{};
    std::array<ex::deadline, 128U> deadlines{};
    ex::tick time{};
    std::size_t transactions{};
    std::size_t immediate_calls{};
    std::size_t releases{};
    std::size_t payload_writes{};
    bool fail{};
    mcubsp::driver::spi_configuration bus_config{};
    std::uint32_t frequency{6'000'000U};

    [[nodiscard]] io::result<std::size_t>
    transfer(std::span<const std::uint8_t> tx, std::span<std::uint8_t> rx,
             ex::deadline limit) noexcept {
        if (ex::reached(time, limit)) {
            return std::unexpected(io::error{io::errc::timeout});
        }
        if (fail) return std::unexpected(io::error{io::errc::hardware});
        if (tx.size() != 4U || (!rx.empty() && rx.size() != 4U)) {
            return std::unexpected(io::error{io::errc::invalid_operation});
        }
        std::copy(tx.begin(), tx.end(), last_command.begin());
        if (transactions < deadlines.size()) deadlines[transactions] = limit;
        ++transactions;
        ++time;
        const auto address = static_cast<std::uint16_t>(
            (static_cast<std::uint16_t>(tx[1]) << 8U) | tx[2]);
        if (tx[0] == 0x0FU) {
            if (rx.empty()) {
                return std::unexpected(io::error{io::errc::invalid_operation});
            }
            std::fill(rx.begin(), rx.end(), 0U);
            rx[3] = memory[address];
        } else if (tx[0] == 0xF0U) {
            if (address >= reg::socket0_tx_base && address < reg::socket0_rx_base) {
                ++payload_writes;
            }
            if (address == reg::mode && tx[3] == 0x80U) {
                memory.fill(0U);
                memory[reg::rx_memory_size] = 0x55U;
                memory[reg::tx_memory_size] = 0x55U;
                memory[reg::socket0_tx_free_size] = 0x20U;
            } else if (address == reg::socket0_interrupt) {
                memory[address] &= static_cast<std::uint8_t>(~tx[3]);
            } else if (address == reg::socket0_command) {
                memory[address] = 0U;
                if (tx[3] == 0x01U) memory[reg::socket0_status] = 0x42U;
                if (tx[3] == 0x10U) memory[reg::socket0_status] = 0U;
                if (tx[3] == 0x20U) memory[reg::socket0_interrupt] |= 0x10U;
            } else {
                memory[address] = tx[3];
            }
        } else {
            return std::unexpected(io::error{io::errc::invalid_operation});
        }
        return tx.size();
    }
};

template<bool Immediate>
class fake_spi_device {
public:
    explicit fake_spi_device(fake_chip& chip) noexcept : chip_(&chip) {}
    fake_spi_device(const fake_spi_device&) = delete;
    fake_spi_device& operator=(const fake_spi_device&) = delete;
    fake_spi_device(fake_spi_device&& source) noexcept
        : chip_(std::exchange(source.chip_, nullptr)) {}
    ~fake_spi_device() { if (chip_) ++chip_->releases; }

    [[nodiscard]] ex::tick now() const noexcept { return chip_->time; }
    [[nodiscard]] mcubsp::driver::spi_configuration configuration() const noexcept {
        return chip_->bus_config;
    }
    [[nodiscard]] std::uint32_t frequency() const noexcept { return chip_->frequency; }

    template<typename Rep, typename Period>
    [[nodiscard]] io::result<ex::deadline>
    deadline_after(std::chrono::duration<Rep, Period> timeout) const noexcept {
        const auto count = std::chrono::duration_cast<std::chrono::milliseconds>(
            timeout).count();
        if (count < 0 || count > ex::maximum_deadline_interval) {
            return std::unexpected(io::error{io::errc::invalid_timeout});
        }
        return *ex::make_deadline(now(), static_cast<ex::tick>(count));
    }

    [[nodiscard]] io::result<std::size_t>
    transaction_immediate(std::span<const std::uint8_t> tx,
                          std::span<std::uint8_t> rx,
                          ex::deadline limit) noexcept requires(Immediate) {
        ++chip_->immediate_calls;
        return chip_->transfer(tx, rx, limit);
    }

    [[nodiscard]] io::result<mcubsp::driver::spi_transfer_result>
    transaction_until(std::span<const mcubsp::driver::spi_segment> segments,
                      ex::deadline limit) noexcept {
        if (segments.size() != 1U || segments[0].transmit.size() != 4U) {
            return std::unexpected(io::error{io::errc::invalid_operation});
        }
        const auto& segment = segments[0];
        std::array<std::uint8_t, 4U> tx{};
        std::array<std::uint8_t, 4U> rx{};
        for (std::size_t index = 0U; index < tx.size(); ++index) {
            tx[index] = static_cast<std::uint8_t>(segment.transmit[index]);
        }
        const auto result = chip_->transfer(tx,
            segment.receive.empty() ? std::span<std::uint8_t>{} : std::span{rx},
            limit);
        if (!result) return std::unexpected(result.error());
        for (std::size_t index = 0U; index < segment.receive.size(); ++index) {
            segment.receive[index] = rx[index];
        }
        return mcubsp::driver::spi_transfer_result{.frames = *result};
    }

private:
    fake_chip* chip_;
};

constexpr mcubsp::device::w5100_configuration config{
    .mac = {{0x02U, 0x11U, 0x22U, 0x33U, 0x44U, 0x55U}},
    .command_poll_limit = 4U,
    .stable_read_limit = 3U,
};

using immediate_device = mcubsp::device::w5100<fake_spi_device<true>>;
static_assert(!std::is_move_constructible_v<immediate_device>);
static_assert(!std::is_copy_constructible_v<immediate_device>);
#if defined(MCUBSP_TEST_MCUIP_CONTRACTS)
static_assert(mcubsp::integration::mcuip::w5100_valid_contract<fake_spi_device<true>>);
static_assert(mcubsp::integration::mcuip::w5100_valid_contract<fake_spi_device<false>>);
#endif

TEST(w5100_io, owned_endpoint_initializes_and_uses_original_chip_framing,
     "REQ-0044", "REQ-0046") {
    fake_chip chip;
    {
        mcubsp::device::w5100 ethernet{fake_spi_device<true>{chip}};
        ASSERT_TRUE(ethernet.begin(config, 100ms).has_value());
        EXPECT_TRUE(ethernet.initialized());
        EXPECT_TRUE(chip.immediate_calls > 0U);
        EXPECT_EQ(chip.immediate_calls, chip.transactions);
        EXPECT_EQ(chip.releases, 0U);
        for (std::size_t index = 0U; index < config.mac.octets.size(); ++index) {
            EXPECT_EQ(chip.memory[reg::source_hardware_address + index],
                      config.mac.octets[index]);
        }
        for (std::size_t index = 0U; index < chip.transactions; ++index) {
            EXPECT_EQ(chip.deadlines[index].value, 100U);
        }
        EXPECT_EQ(chip.last_command[0], 0x0FU);
        EXPECT_EQ(chip.last_command[1], 0x04U);
        EXPECT_EQ(chip.last_command[2], 0x03U);
    }
    EXPECT_EQ(chip.releases, 1U);
}

TEST(w5100_io, initialization_deadline_is_shared_across_all_register_accesses,
     "REQ-0044", "REQ-0046") {
    fake_chip chip;
    mcubsp::device::w5100 ethernet{fake_spi_device<true>{chip}};
    const auto result = ethernet.begin(config, 5ms);
    ASSERT_FALSE(result.has_value());
    EXPECT_TRUE(result.error() == error::timeout);
    EXPECT_FALSE(ethernet.initialized());
    EXPECT_EQ(chip.transactions, 5U);
    EXPECT_EQ(ethernet.metrics().timeouts, 1U);
    for (std::size_t index = 0U; index < chip.transactions; ++index) {
        EXPECT_EQ(chip.deadlines[index].value, 5U);
    }
    ASSERT_TRUE(ethernet.begin(config, 100ms).has_value());
    EXPECT_TRUE(ethernet.initialized());
}

TEST(w5100_io, fallback_endpoint_uses_checked_transactions_and_same_deadline,
     "REQ-0044", "REQ-0046") {
    fake_chip chip;
    mcubsp::device::w5100 ethernet{fake_spi_device<false>{chip}};
    ASSERT_TRUE(ethernet.initialize(config, 100ms).has_value());
    EXPECT_EQ(chip.immediate_calls, 0U);
    EXPECT_TRUE(chip.transactions > 0U);
    std::array<std::byte, 60U> frame{};
    frame[0] = std::byte{0x42U};
    ASSERT_TRUE(ethernet.start_transmit(0U, frame));
    const auto limit = ex::deadline{1000U};
    ASSERT_TRUE(ethernet.poll_until(limit).has_value());
    EXPECT_EQ(chip.memory[reg::socket0_tx_base], 0x42U);
    ASSERT_TRUE(ethernet.poll_until(limit).has_value());
    EXPECT_TRUE(ethernet.take_transmit(0U));
}

TEST(w5100_io, expired_and_invalid_deadlines_do_not_access_the_bus,
     "REQ-0044", "REQ-0046") {
    fake_chip chip;
    mcubsp::device::w5100 ethernet{fake_spi_device<true>{chip}};
    const auto invalid = ethernet.begin(config, -1ms);
    ASSERT_FALSE(invalid.has_value());
    EXPECT_TRUE(invalid.error() == error::invalid_argument);
    EXPECT_EQ(chip.transactions, 0U);
    const auto expired = ethernet.begin_until(config, ex::deadline{0U});
    ASSERT_FALSE(expired.has_value());
    EXPECT_TRUE(expired.error() == error::timeout);
    EXPECT_EQ(chip.transactions, 0U);
    ASSERT_TRUE(ethernet.begin(config, 100ms).has_value());
    const auto before = chip.transactions;
    const auto poll = ethernet.poll_until(ex::deadline{chip.time});
    ASSERT_FALSE(poll.has_value());
    EXPECT_TRUE(poll.error() == error::timeout);
    EXPECT_EQ(chip.transactions, before);
}

TEST(w5100_io, rejects_incompatible_spi_configuration_before_register_access,
     "REQ-0044", "REQ-0046") {
    fake_chip chip;
    mcubsp::device::w5100 ethernet{fake_spi_device<false>{chip}};
    chip.bus_config.frame_width = mcubsp::driver::spi_frame_width::bits16;
    const auto wide = ethernet.begin(config, 100ms);
    ASSERT_FALSE(wide.has_value());
    EXPECT_TRUE(wide.error() == error::invalid_argument);
    chip.bus_config = {};
    chip.bus_config.mode = mcubsp::driver::spi_mode::mode3;
    EXPECT_FALSE(ethernet.begin(config, 100ms).has_value());
    chip.bus_config = {};
    chip.bus_config.bit_order = mcubsp::driver::spi_bit_order::least_significant_first;
    EXPECT_FALSE(ethernet.begin(config, 100ms).has_value());
    chip.bus_config = {};
    chip.bus_config.chip_select = mcubsp::driver::spi_chip_select_polarity::active_high;
    EXPECT_FALSE(ethernet.begin(config, 100ms).has_value());
    chip.bus_config = {};
    chip.frequency = 20'000'000U;
    EXPECT_FALSE(ethernet.begin(config, 100ms).has_value());
    EXPECT_EQ(chip.transactions, 0U);
    chip.frequency = 6'000'000U;
    EXPECT_TRUE(ethernet.begin(config, 100ms).has_value());
}

TEST(w5100_io, deadline_during_payload_copy_retains_progress_for_next_poll,
     "REQ-0044", "REQ-0046") {
    fake_chip chip;
    mcubsp::device::w5100 ethernet{fake_spi_device<true>{chip}};
    ASSERT_TRUE(ethernet.begin(config, 100ms).has_value());
    std::array<std::byte, 60U> frame{};
    ASSERT_TRUE(ethernet.start_transmit(0U, frame));
    const auto first = ethernet.poll_until(ex::deadline{chip.time + 24U});
    ASSERT_FALSE(first.has_value());
    EXPECT_TRUE(first.error() == error::timeout);
    EXPECT_TRUE(chip.payload_writes > 0U);
    EXPECT_TRUE(chip.payload_writes < frame.size());
    for (unsigned poll = 0U; poll < 4U; ++poll) {
        static_cast<void>(ethernet.poll_until(ex::deadline{chip.time + 24U}));
    }
    const auto result = ethernet.take_transmit_result(0U);
    ASSERT_TRUE(result.has_value());
    EXPECT_FALSE(result->error);
    EXPECT_EQ(chip.payload_writes, frame.size());
}

} // namespace
