// @req-file REQ-0002 REQ-0009 REQ-0015 REQ-0017 REQ-0019 REQ-0035 REQ-0044

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <optional>
#include <span>

#include <mcutest/mcutest.hpp>

#include <mcuhw/device/ethernet/w5100_spi.hpp>
#include <mcuhw/integration/mcuip/w5100.hpp>

namespace {

namespace reg = mcuhw::device::w5100_register;
using error = mcuhw::device::w5100_error;

class fake_w5100_bus {
public:
    fake_w5100_bus() noexcept { reset_registers(); }

    [[nodiscard]] std::expected<std::uint8_t, error>
    read(std::uint16_t address) noexcept {
        ++reads;
        if (address >= reg::socket0_rx_base) ++payload_reads;
        if (fail_address && *fail_address == address) {
            return std::unexpected(error::transport);
        }
        return memory[address];
    }

    [[nodiscard]] std::expected<void, error>
    write(std::uint16_t address, std::uint8_t value) noexcept {
        ++writes;
        if (address >= reg::socket0_tx_base && address < reg::socket0_rx_base) {
            ++payload_writes;
        }
        if ((fail_address && *fail_address == address)
            || (fail_write_address && *fail_write_address == address)) {
            return std::unexpected(error::transport);
        }
        if (address == reg::mode && (value & 0x80U) != 0U) {
            reset_registers();
            memory[reg::mode] = hold_reset ? 0x80U : 0U;
            return {};
        }
        if (address == reg::socket0_interrupt) {
            memory[address] &= static_cast<std::uint8_t>(~value);
            return {};
        }
        memory[address] = value;
        if (address != reg::socket0_command || value == 0U) return {};
        ++commands;
        if (stall_commands) return {};
        switch (value) {
        case 0x01U:
            memory[reg::socket0_status] =
                memory[reg::socket0_mode] == 0x04U ? 0x42U : 0U;
            break;
        case 0x10U:
            memory[reg::socket0_status] = 0U;
            break;
        case 0x20U:
            if (!defer_send_completion) {
                memory[reg::socket0_interrupt] |= 0x10U;
            }
            break;
        case 0x40U:
            set_u16(reg::socket0_rx_received_size, 0U);
            break;
        default:
            break;
        }
        memory[address] = 0U;
        return {};
    }

    void set_u16(std::uint16_t address, std::uint16_t value) noexcept {
        memory[address] = static_cast<std::uint8_t>(value >> 8U);
        memory[static_cast<std::uint16_t>(address + 1U)] =
            static_cast<std::uint8_t>(value);
    }

    [[nodiscard]] std::uint16_t u16(std::uint16_t address) const noexcept {
        return static_cast<std::uint16_t>(
            (static_cast<std::uint16_t>(memory[address]) << 8U)
            | memory[static_cast<std::uint16_t>(address + 1U)]);
    }

    void load_receive(std::uint16_t pointer,
                      std::span<const std::byte> frame) noexcept {
        const auto stored = static_cast<std::uint16_t>(frame.size() + 2U);
        set_u16(reg::socket0_rx_read_pointer, pointer);
        put_rx(pointer, static_cast<std::uint8_t>(stored >> 8U));
        put_rx(static_cast<std::uint16_t>(pointer + 1U),
               static_cast<std::uint8_t>(stored));
        for (std::size_t index = 0U; index < frame.size(); ++index) {
            put_rx(static_cast<std::uint16_t>(pointer + 2U + index),
                   std::to_integer<std::uint8_t>(frame[index]));
        }
        set_u16(reg::socket0_rx_received_size, stored);
    }

    void load_receive_length(std::uint16_t pointer,
                             std::uint16_t stored) noexcept {
        set_u16(reg::socket0_rx_read_pointer, pointer);
        put_rx(pointer, static_cast<std::uint8_t>(stored >> 8U));
        put_rx(static_cast<std::uint16_t>(pointer + 1U),
               static_cast<std::uint8_t>(stored));
        set_u16(reg::socket0_rx_received_size, stored);
    }

    [[nodiscard]] std::uint8_t tx(std::uint16_t pointer) const noexcept {
        return memory[static_cast<std::uint16_t>(
            reg::socket0_tx_base + (pointer & 0x1FFFU))];
    }

    void complete_send(bool timed_out = false) noexcept {
        memory[reg::socket0_interrupt] |= timed_out ? 0x08U : 0x10U;
    }

    std::array<std::uint8_t, 0x8000U> memory{};
    std::optional<std::uint16_t> fail_address{};
    std::optional<std::uint16_t> fail_write_address{};
    std::size_t reads{};
    std::size_t writes{};
    std::size_t commands{};
    std::size_t payload_reads{};
    std::size_t payload_writes{};
    bool hold_reset{};
    bool stall_commands{};
    bool defer_send_completion{};

private:
    void reset_registers() noexcept {
        memory.fill(0U);
        memory[reg::rx_memory_size] = 0x55U;
        memory[reg::tx_memory_size] = 0x55U;
        set_u16(reg::socket0_tx_free_size, 8192U);
    }

    void put_rx(std::uint16_t pointer, std::uint8_t value) noexcept {
        memory[static_cast<std::uint16_t>(
            reg::socket0_rx_base + (pointer & 0x1FFFU))] = value;
    }
};

class fake_immediate_spi {
public:
    [[nodiscard]] std::expected<std::size_t, mcuhw::driver::spi_error>
    transaction_immediate(std::span<const std::uint8_t> transmit,
                          std::span<std::uint8_t> receive,
                          std::uint32_t poll_limit) noexcept {
        ++transactions;
        last_size = transmit.size();
        last_poll_limit = poll_limit;
        std::copy(transmit.begin(), transmit.end(), last_command.begin());
        if (fail) return std::unexpected(mcuhw::driver::spi_error::timeout);
        if (!receive.empty()) {
            std::copy(transmit.begin(), transmit.end(), receive.begin());
            receive.back() = response;
        }
        return transmit.size();
    }

    std::array<std::uint8_t, 4U> last_command{};
    std::size_t transactions{};
    std::size_t last_size{};
    std::uint32_t last_poll_limit{};
    std::uint8_t response{0x5AU};
    bool fail{};
};

using device = mcuhw::device::w5100_macraw_device<fake_w5100_bus>;
#if defined(MCUHW_TEST_MCUIP_CONTRACTS)
static_assert(::mcuip::NetworkDevice<device>);
static_assert(::mcuip::NetworkDevice<mcuhw::integration::mcuip::
    w5100_macraw_network_device<fake_w5100_bus>>);
#endif

constexpr mcuhw::device::w5100_configuration configuration{
    .mac = {{0x02U, 0x00U, 0x00U, 0x04U, 0x51U, 0x00U}},
    .command_poll_limit = 4U,
    .stable_read_limit = 3U,
};

[[nodiscard]] std::array<std::byte, 60U> small_frame() noexcept {
    std::array<std::byte, 60U> frame{};
    for (std::size_t index = 0U; index < frame.size(); ++index) {
        frame[index] = static_cast<std::byte>(index + 1U);
    }
    return frame;
}

TEST(w5100, reset_identity_macraw_and_fixed_memory_configuration,
     "REQ-0044") {
    fake_w5100_bus bus;
    device ethernet{bus};
    const auto initialized = ethernet.initialize(configuration);
    ASSERT_TRUE(initialized.has_value());
    EXPECT_TRUE(ethernet.initialized());
    EXPECT_EQ(bus.memory[reg::rx_memory_size], 0x03U);
    EXPECT_EQ(bus.memory[reg::tx_memory_size], 0x03U);
    EXPECT_EQ(bus.memory[reg::socket0_mode], 0x04U);
    EXPECT_EQ(bus.memory[reg::socket0_status], 0x42U);
    for (std::size_t index = 0U; index < configuration.mac.octets.size();
         ++index) {
        EXPECT_EQ(bus.memory[reg::source_hardware_address + index],
                  configuration.mac.octets[index]);
    }
    EXPECT_TRUE(bus.commands >= 2U);
}

TEST(w5100, immediate_spi_transport_preserves_one_command_per_chip_select,
     "REQ-0028", "REQ-0044") {
    fake_immediate_spi spi;
    mcuhw::device::w5100_spi_immediate_transport transport{spi, 77U};
    const auto value = transport.read(0x1234U);
    ASSERT_TRUE(value.has_value());
    EXPECT_EQ(*value, 0x5AU);
    EXPECT_EQ(spi.last_command[0], 0x0FU);
    EXPECT_EQ(spi.last_command[1], 0x12U);
    EXPECT_EQ(spi.last_command[2], 0x34U);
    EXPECT_EQ(spi.last_command[3], 0U);
    EXPECT_EQ(spi.last_size, 4U);
    EXPECT_EQ(spi.last_poll_limit, 77U);

    ASSERT_TRUE(transport.write(0x4321U, 0xA5U).has_value());
    EXPECT_EQ(spi.last_command[0], 0xF0U);
    EXPECT_EQ(spi.last_command[1], 0x43U);
    EXPECT_EQ(spi.last_command[2], 0x21U);
    EXPECT_EQ(spi.last_command[3], 0xA5U);
    EXPECT_EQ(spi.transactions, 2U);

    spi.fail = true;
    const auto timed_out = transport.read(0U);
    ASSERT_FALSE(timed_out.has_value());
    EXPECT_TRUE(timed_out.error() == error::timeout);
}

TEST(w5100, receive_backpressure_and_circular_wrap_preserve_frame,
     "REQ-0044") {
    fake_w5100_bus bus;
    device ethernet{bus};
    ASSERT_TRUE(ethernet.initialize(configuration).has_value());
    const auto expected = small_frame();
    bus.load_receive(0x1FFFU, expected);

    ethernet.poll();
    EXPECT_EQ(ethernet.metrics().receive_backpressure, 1U);
    EXPECT_EQ(bus.u16(reg::socket0_rx_received_size), expected.size() + 2U);

    std::array<std::byte, device::buffer_size> buffer{};
    ASSERT_TRUE(ethernet.arm_receive(1U, buffer));
    ethernet.poll();
    const auto completion = ethernet.take_receive(1U);
    ASSERT_TRUE(completion.has_value());
    EXPECT_TRUE(!completion->error);
    EXPECT_EQ(completion->size, expected.size());
    EXPECT_TRUE(std::equal(expected.begin(), expected.end(), buffer.begin()));
    EXPECT_EQ(bus.u16(reg::socket0_rx_read_pointer),
              static_cast<std::uint16_t>(0x1FFFU + expected.size() + 2U));
    EXPECT_EQ(ethernet.metrics().received_frames, 1U);
}

TEST(w5100, transmit_waits_for_space_wraps_and_ignores_stale_send_ok,
     "REQ-0044") {
    fake_w5100_bus bus;
    device ethernet{bus};
    ASSERT_TRUE(ethernet.initialize(configuration).has_value());
    const auto frame = small_frame();
    bus.set_u16(reg::socket0_tx_write_pointer, 0x1FFEU);
    bus.set_u16(reg::socket0_tx_free_size, 0U);
    bus.memory[reg::socket0_interrupt] = 0x10U;
    bus.defer_send_completion = true;
    ASSERT_TRUE(ethernet.start_transmit(0U, frame));

    ethernet.poll();
    EXPECT_EQ(bus.u16(reg::socket0_tx_write_pointer), 0x1FFEU);
    EXPECT_TRUE(!ethernet.take_transmit(0U));

    bus.set_u16(reg::socket0_tx_free_size, 8192U);
    ethernet.poll();
    EXPECT_EQ(bus.u16(reg::socket0_tx_write_pointer),
              static_cast<std::uint16_t>(0x1FFEU + frame.size()));
    EXPECT_EQ(bus.tx(0x1FFEU), std::to_integer<std::uint8_t>(frame[0]));
    EXPECT_EQ(bus.tx(0x1FFFU), std::to_integer<std::uint8_t>(frame[1]));
    EXPECT_EQ(bus.tx(0x2000U), std::to_integer<std::uint8_t>(frame[2]));
    EXPECT_TRUE(!ethernet.take_transmit(0U));

    bus.complete_send();
    ethernet.poll();
    EXPECT_TRUE(ethernet.take_transmit(0U));
    EXPECT_EQ(ethernet.metrics().transmitted_frames, 1U);
}

TEST(w5100, malformed_and_oversize_receive_complete_with_error_and_recover,
     "REQ-0044") {
    fake_w5100_bus bus;
    device ethernet{bus};
    ASSERT_TRUE(ethernet.initialize(configuration).has_value());
    std::array<std::byte, device::buffer_size> buffer{};

    ASSERT_TRUE(ethernet.arm_receive(0U, buffer));
    bus.load_receive_length(0x1234U, 1U);
    ethernet.poll();
    const auto malformed = ethernet.take_receive(0U);
    ASSERT_TRUE(malformed.has_value());
    EXPECT_TRUE(malformed->error);
    EXPECT_EQ(ethernet.metrics().malformed_frames, 1U);
    EXPECT_EQ(ethernet.metrics().recoveries, 1U);
    EXPECT_EQ(bus.memory[reg::socket0_status], 0x42U);

    ASSERT_TRUE(ethernet.arm_receive(0U, buffer));
    bus.load_receive_length(0x1FFDU,
        static_cast<std::uint16_t>(device::buffer_size + 3U));
    ethernet.poll();
    const auto oversize = ethernet.take_receive(0U);
    ASSERT_TRUE(oversize.has_value());
    EXPECT_TRUE(oversize->error);
    EXPECT_EQ(ethernet.metrics().oversize_frames, 1U);
    EXPECT_EQ(bus.u16(reg::socket0_rx_received_size), 0U);
}

TEST(w5100, typed_timeouts_transport_errors_cancellation_and_reuse,
     "REQ-0044") {
    fake_w5100_bus stalled_bus;
    stalled_bus.stall_commands = true;
    device stalled{stalled_bus};
    const auto command_timeout = stalled.initialize(configuration);
    ASSERT_TRUE(!command_timeout.has_value());
    EXPECT_EQ(static_cast<unsigned>(command_timeout.error()),
              static_cast<unsigned>(error::command_timeout));

    fake_w5100_bus bus;
    device ethernet{bus};
    ASSERT_TRUE(ethernet.initialize(configuration).has_value());
    std::array<std::byte, device::buffer_size> rx{};
    const auto frame = small_frame();
    ASSERT_TRUE(ethernet.arm_receive(0U, rx));
    ASSERT_TRUE(ethernet.start_transmit(0U, frame));
    ethernet.cancel_all();
    EXPECT_EQ(ethernet.metrics().cancellations, 1U);
    EXPECT_TRUE(ethernet.arm_receive(0U, rx));
    EXPECT_TRUE(ethernet.start_transmit(0U, frame));
    ethernet.cancel_all();

    bus.fail_address = reg::socket0_rx_received_size;
    ethernet.poll();
    ASSERT_TRUE(ethernet.last_error().has_value());
    EXPECT_EQ(static_cast<unsigned>(*ethernet.last_error()),
              static_cast<unsigned>(error::transport));
    EXPECT_EQ(ethernet.metrics().transport_errors, 1U);

    bus.fail_address.reset();
    bus.defer_send_completion = true;
    ASSERT_TRUE(ethernet.start_transmit(0U, frame));
    ethernet.poll();
    bus.complete_send(true);
    ethernet.poll();
    const auto timed_out = ethernet.take_transmit_result(0U);
    ASSERT_TRUE(timed_out.has_value());
    EXPECT_TRUE(timed_out->error);
    EXPECT_TRUE(ethernet.metrics().timeouts >= 1U);
}

TEST(w5100, stable_reads_require_two_samples_and_nonzero_poll_budget,
     "REQ-0044", "REQ-0046") {
    fake_w5100_bus bus;
    device ethernet{bus};
    auto invalid = configuration;
    invalid.stable_read_limit = 1U;
    const auto result = ethernet.initialize(invalid);
    ASSERT_FALSE(result.has_value());
    EXPECT_TRUE(result.error() == error::invalid_argument);
    EXPECT_EQ(bus.writes, 0U);
    invalid = configuration;
    invalid.payload_bytes_per_poll = 0U;
    EXPECT_FALSE(ethernet.initialize(invalid).has_value());
    EXPECT_EQ(bus.writes, 0U);
}

TEST(w5100, shared_poll_budget_resumes_both_directions_without_starvation,
     "REQ-0044", "REQ-0046") {
    fake_w5100_bus bus;
    device ethernet{bus};
    auto bounded = configuration;
    bounded.payload_bytes_per_poll = 16U;
    ASSERT_TRUE(ethernet.initialize(bounded).has_value());
    const auto frame = small_frame();
    std::array<std::byte, device::buffer_size> rx{};
    bus.load_receive(0x1FFAU, frame);
    bus.set_u16(reg::socket0_tx_write_pointer, 0x1FFEU);
    ASSERT_TRUE(ethernet.arm_receive(0U, rx));
    ASSERT_TRUE(ethernet.start_transmit(0U, frame));
    std::optional<mcuhw::device::w5100_receive_completion> received;
    std::optional<mcuhw::device::w5100_transmit_completion> sent;
    for (unsigned poll = 0U; poll < 10U; ++poll) {
        const auto before = bus.payload_reads + bus.payload_writes;
        ethernet.poll();
        // The ring's two-byte RX header is control work, read only once.
        EXPECT_TRUE(bus.payload_reads + bus.payload_writes - before <= 18U);
        if (!received) received = ethernet.take_receive(0U);
        if (!sent) sent = ethernet.take_transmit_result(0U);
        if (poll == 1U) {
            EXPECT_TRUE(bus.payload_reads > 2U);
            EXPECT_TRUE(bus.payload_writes > 0U);
            EXPECT_FALSE(received.has_value());
            EXPECT_FALSE(sent.has_value());
        }
    }
    ASSERT_TRUE(received.has_value());
    ASSERT_TRUE(sent.has_value());
    EXPECT_FALSE(received->error);
    EXPECT_FALSE(sent->error);
    EXPECT_EQ(received->size, frame.size());
    EXPECT_TRUE(std::equal(frame.begin(), frame.end(), rx.begin()));
    EXPECT_EQ(bus.payload_reads, frame.size() + 2U);
    EXPECT_EQ(bus.payload_writes, frame.size());
    EXPECT_EQ(bus.tx(0x2000U), std::to_integer<std::uint8_t>(frame[2]));
}

TEST(w5100, ambiguous_pointer_commit_requires_recovery_without_repeating_send,
     "REQ-0044", "REQ-0046") {
    fake_w5100_bus bus;
    device ethernet{bus};
    ASSERT_TRUE(ethernet.initialize(configuration).has_value());
    const auto frame = small_frame();
    bus.set_u16(reg::socket0_tx_write_pointer, 0x00FFU);
    ASSERT_TRUE(ethernet.start_transmit(0U, frame));
    bus.fail_write_address = reg::socket0_tx_write_pointer + 1U;
    EXPECT_FALSE(ethernet.poll_once().has_value());
    const auto transactions = bus.reads + bus.writes;
    bus.fail_write_address.reset();
    EXPECT_FALSE(ethernet.poll_once().has_value());
    EXPECT_EQ(bus.reads + bus.writes, transactions);
    ASSERT_TRUE(ethernet.recover().has_value());
    const auto result = ethernet.take_transmit_result(0U);
    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(result->error);
    EXPECT_TRUE(ethernet.start_transmit(0U, frame));
}

TEST(w5100, cancellation_discards_partial_copy_state_for_buffer_reuse,
     "REQ-0044", "REQ-0046") {
    fake_w5100_bus bus;
    device ethernet{bus};
    auto bounded = configuration;
    bounded.payload_bytes_per_poll = 8U;
    ASSERT_TRUE(ethernet.initialize(bounded).has_value());
    const auto frame = small_frame();
    std::array<std::byte, device::buffer_size> rx{};
    bus.load_receive(0x1FFAU, frame);
    ASSERT_TRUE(ethernet.arm_receive(0U, rx));
    ethernet.poll();
    EXPECT_FALSE(ethernet.take_receive(0U).has_value());
    ethernet.cancel_all();
    rx.fill(std::byte{});
    ASSERT_TRUE(ethernet.arm_receive(0U, rx));
    for (unsigned poll = 0U; poll < 8U; ++poll) ethernet.poll();
    const auto result = ethernet.take_receive(0U);
    ASSERT_TRUE(result.has_value());
    EXPECT_FALSE(result->error);
    EXPECT_TRUE(std::equal(frame.begin(), frame.end(), rx.begin()));
}

TEST(w5100, cancellation_of_issued_send_requires_recovery_before_reuse,
     "REQ-0044", "REQ-0046") {
    fake_w5100_bus bus;
    device ethernet{bus};
    ASSERT_TRUE(ethernet.initialize(configuration).has_value());
    const auto frame = small_frame();
    bus.defer_send_completion = true;
    ASSERT_TRUE(ethernet.start_transmit(0U, frame));
    ethernet.poll();
    ethernet.cancel_all();
    bus.complete_send();
    EXPECT_FALSE(ethernet.start_transmit(0U, frame));
    const auto before = bus.reads + bus.writes;
    EXPECT_FALSE(ethernet.poll_once().has_value());
    EXPECT_EQ(bus.reads + bus.writes, before);
    ASSERT_TRUE(ethernet.recover().has_value());
    EXPECT_EQ(bus.memory[reg::socket0_interrupt] & 0x10U, 0U);
    ASSERT_TRUE(ethernet.start_transmit(0U, frame));
    ethernet.poll();
    EXPECT_FALSE(ethernet.take_transmit_result(0U).has_value());
    bus.complete_send();
    ethernet.poll();
    EXPECT_TRUE(ethernet.take_transmit(0U));
}

} // namespace
