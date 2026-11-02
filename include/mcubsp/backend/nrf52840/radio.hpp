#pragma once
#include <algorithm>
// @req-file REQ-0069 REQ-0062
// PS v1.11 6.20; Nordic 4413_505 v1.0 anomaly 172 flow, and anomalies
// 153,191,204,228,236,243,245,248,254,279. Native packet engine; no stack ABI.
#include <mcubsp/backend/nrf52840/detail/peripheral_session.hpp>
#include <mcubsp/mcu/nrf52840/product/radio.hpp>
#include <mcubsp/mcu/nrf52840/serial_revision.hpp>
namespace mcubsp::backend::nrf52840 {
enum class radio_phy : unsigned {
  nordic1 = 0,
  nordic2 = 1,
  ble1 = 3,
  ble2 = 4,
  coded125 = 5,
  coded500 = 6,
  ieee802154 = 15
};
struct radio_packet_format {
  unsigned s0_bytes{1}, length_bits{8}, s1_bits{}, maximum_payload{255},
      static_payload{}, base_bytes{3};
  bool s1_in_ram{}, big_endian{}, whitening{true}, crc_in_length{};
};
struct radio_crc {
  unsigned bytes{3}, skip_address{1};
  std::uint32_t polynomial{0x65B}, initial{0x555555};
};
struct radio_configuration {
  radio_phy phy{radio_phy::ble1};
  unsigned frequency_mhz{2402};
  int power_dbm{};
  bool automatic_packet_format{true}, fast_ramp{}, external_frontend{};
  unsigned idle_carrier{2};
  radio_packet_format format{};
  radio_crc crc{};
  std::uint32_t base0{0x89BED600}, base1{}, prefix0{0x8E}, prefix1{};
  unsigned transmit_address{}, receive_addresses{1}, whitening_iv{37},
      sfd{0xA7}, tifs_us{150};
  unsigned ticks_per_second{64'000'000};
};
struct radio_device_address {
  std::uint32_t low{};
  std::uint16_t high{};
  bool random{};
};
struct radio_result {
  bool receive{}, crc_valid{}, truncated{}, invalid_coding{}, service_overrun{};
  std::size_t bytes{};
  unsigned address_index{}, coding{};
  std::uint32_t received_crc{};
  std::optional<unsigned> rssi_sample;
  std::optional<unsigned> link_quality;
};
struct radio_energy {
  unsigned hardware{}, ieee802154{};
  int dbm{};
};
[[nodiscard]] constexpr int
compensated_rssi_dbm(unsigned sample, int temperature_quarters) noexcept {
  int adjustment = temperature_quarters <= -120  ? 3
                   : temperature_quarters <= -40 ? 2
                   : temperature_quarters <= 40  ? 1
                   : temperature_quarters <= 120 ? 0
                   : temperature_quarters <= 200 ? -1
                   : temperature_quarters <= 280 ? -2
                                                 : -3;
  return -(int(sample) + adjustment);
}
// Direct hardware ownership excludes the explicit stack reservation below.
// Lifecycle is foreground-only; poll() may run in the registered RADIO IRQ.
// Coded reception must be serviced within 500 us using the execution policy's
// monotonic clock (an independent timer IRQ may call poll()). A late service
// stops and returns an overrun result rather than claiming a qualified packet.
template <class Context, class Buffers, class Address = native_dma_address,
          unsigned PollBudget = 1000000>
class radio_controller {
  using session = detail::peripheral_session<Context, Buffers, Address>;

public:
  radio_controller(Context &c, clock_controller<Context> &clocks, Buffers &b,
                   execution::tick timeout = 1'280'000, Address a = {}) noexcept
      : s_(c, clocks, b, a), timeout_(timeout) {}
  radio_controller(const radio_controller &) = delete;
  radio_controller &operator=(const radio_controller &) = delete;
  ~radio_controller() {
    if (dma_)
      s_.context.execution.active_transfer_destroyed();
  }
  [[nodiscard]] system_result
  initialize(radio_configuration configuration = {}) noexcept {
    if (!valid(configuration) || !timeout_ ||
        timeout_ > execution::maximum_deadline_interval)
      return std::unexpected(system_error::invalid);
    auto p = s_.context.read(0x10000130), r = s_.context.read(0x10000134);
    if (!p || !r)
      return std::unexpected(system_error::mmio);
    if (!mcu::nrf52840::serial_revision{*p, *r}.reviewed_modern())
      return std::unexpected(system_error::unsupported);
    auto claim = s_.acquire_routed(peripheral::radio, 76,
                                   endpoint_domain::radio, base, 0x550);
    if (!claim)
      return claim;
    if (!s_.write(0xFFC, 1))
      return fail();
    auto affected = s_.read(0x788);
    if (!affected)
      return fail();
    anomaly172_ = !*affected;
    auto configured = configure(configuration);
    if (!configured)
      return configured;
    initialized_ = true;
    return {};
  }
  [[nodiscard]] system_result
  configure(radio_configuration configuration) noexcept {
    if (!s_.owned() || dma_ || auxiliary_ || s_.routes_referenced() ||
        s_.context.execution.in_isr())
      return std::unexpected(system_error::busy);
    if (!valid(configuration))
      return std::unexpected(system_error::invalid);
    auto state = s_.read(0x550);
    if (!state || *state)
      return std::unexpected(system_error::busy);
    if (!strict(false))
      return fail();
    if (configuration.automatic_packet_format) {
      if (configuration.phy == radio_phy::ieee802154) {
        configuration.format = {0, 8, 0, 127, 0, 0, false, false, false, true};
        configuration.crc = {2, 2, 0x11021, 0};
      } else if (configuration.phy == radio_phy::nordic1 ||
                 configuration.phy == radio_phy::nordic2) {
        configuration.format = {0, 8, 0, 255, 0, 4, false, false, false, false};
        configuration.crc = {2, 1, 0x11021, 0xFFFF};
      } else {
        configuration.format = {};
        configuration.crc = {};
      }
    }
    config_ = configuration;
    auto &f = config_.format;
    unsigned plen = long_range()                           ? 3
                    : config_.phy == radio_phy::ieee802154 ? 2
                    : config_.phy == radio_phy::ble2       ? 1
                                                           : 0;
    unsigned pcnf0 = f.length_bits | (f.s0_bytes << 8) | (f.s1_bits << 16) |
                     (f.s1_in_ram ? 1U << 20 : 0U) |
                     (long_range() ? 2U << 22 : 0U) | (plen << 24) |
                     (f.crc_in_length ? 1U << 26 : 0U) |
                     (long_range() ? 3U << 29 : 0U);
    unsigned pcnf1 = f.maximum_payload | (f.static_payload << 8) |
                     (f.base_bytes << 16) | (f.big_endian ? 1U << 24 : 0U) |
                     (f.whitening ? 1U << 25 : 0U);
    auto lr = s_.read(0x740);
    if (!lr)
      return fail();
    if (!s_.write(0x740, long_range() ? (*lr & 0x7FFF00FF) | 0x8000C400
                                      : (*lr & 0x7FFFFFFF)) ||
        !s_.write(0x308, irq_mask) || !s_.write(0x200, 0) || !clear_events() ||
        !s_.write(0x510, unsigned(config_.phy)) ||
        !s_.write(0x508, config_.frequency_mhz < 2400
                             ? (config_.frequency_mhz - 2360) | (1U << 8)
                             : config_.frequency_mhz - 2400) ||
        !s_.write(0x50C, std::uint8_t(config_.power_dbm)) ||
        !s_.write(0x514, pcnf0) || !s_.write(0x518, pcnf1) ||
        !s_.write(0x51C, config_.base0) || !s_.write(0x520, config_.base1) ||
        !s_.write(0x524, config_.prefix0) ||
        !s_.write(0x528, config_.prefix1) ||
        !s_.write(0x52C, config_.transmit_address) ||
        !s_.write(0x530, config_.receive_addresses) ||
        !s_.write(0x534, config_.crc.bytes | (config_.crc.skip_address << 8)) ||
        !s_.write(0x538, config_.crc.polynomial) ||
        !s_.write(0x53C, config_.crc.initial) ||
        !s_.write(0x554, config_.whitening_iv) ||
        !s_.write(0x544, config_.tifs_us) ||
        !s_.write(0x650, (config_.fast_ramp ? 1U : 0U) |
                             (config_.idle_carrier << 8)) ||
        !s_.write(0x660, config_.sfd) || !s_.write(0x640, 0))
      return fail();
    if (config_.external_frontend && !frontend_configuration())
      return fail();
    s_.publish();
    return {};
  }
  [[nodiscard]] system_result transmit(std::span<const std::uint8_t> packet,
                                       bool start_now = true) noexcept {
    if (packet.size() < header_bytes())
      return std::unexpected(system_error::invalid);
    auto bytes = packet_bytes(packet, false);
    if (!bytes || *bytes > packet.size())
      return std::unexpected(system_error::invalid);
    return start_packet(packet.data(), *bytes, false, start_now);
  }
  [[nodiscard]] system_result receive(std::span<std::uint8_t> packet,
                                      bool start_now = true) noexcept {
    auto required = header_bytes() + config_.format.maximum_payload +
                    (config_.phy == radio_phy::ieee802154 ? 1U : 0U);
    if (packet.size() < required)
      return std::unexpected(system_error::invalid);
    return start_packet(packet.data(), required, true, start_now);
  }
  [[nodiscard]] system_value<std::optional<radio_result>> poll() noexcept {
    return s_.context.execution.critical(
        [&]() -> system_value<std::optional<radio_result>> {
          if (!dma_)
            return std::optional<radio_result>{};
          if (receiving_ && long_range() && !stopping_) {
            auto now = s_.context.execution.now();
            auto state = s_.read(0x550);
            if (!state)
              return std::unexpected(system_error::mmio);
            // An armed PPI transfer may wait indefinitely with RADIO disabled.
            if (*state && static_cast<execution::tick>(now - last_poll_) >
                              std::uint64_t(config_.ticks_per_second) / 2000) {
              result_.service_overrun = true;
              auto stop = request_disable();
              if (!stop)
                return std::unexpected(stop.error());
            }
            last_poll_ = now;
          }
          for (unsigned i = 0; i < event_offsets.size(); ++i) {
            auto event = s_.read(event_offsets[i]);
            if (!event)
              return std::unexpected(system_error::mmio);
            if (*event) {
              observed_ |= 1U << i;
              if (!s_.write(event_offsets[i], 0))
                return std::unexpected(system_error::mmio);
            }
          }
          if (observed_ & event_bit(0x110))
            disabled_seen_ = true;
          if (observed_ & event_bit(0x10C))
            end_seen_ = true;
          if (observed_ & event_bit(0x16C))
            phy_end_seen_ = true;
          if (receiving_ && long_range() && (observed_ & event_bit(0x138)) &&
              !coding_checked_) {
            auto pdu = s_.read(0x414);
            if (!pdu)
              return std::unexpected(system_error::mmio);
            result_.coding = (*pdu >> 1) & 3;
            coding_checked_ = true;
            if (result_.coding > 1) {
              result_.invalid_coding = true;
              auto r = request_disable();
              if (!r)
                return std::unexpected(r.error());
            }
          }
          if (receiving_ && long_range() && anomaly172_ && !stopping_) {
            auto service = service_172();
            if (!service)
              return std::unexpected(service.error());
          }
          if (!stopping_ &&
              ((long_range() || config_.phy == radio_phy::ieee802154)
                   ? phy_end_seen_
                   : end_seen_)) {
            auto off = request_disable();
            if (!off)
              return std::unexpected(off.error());
          }
          if (!disabled_seen_)
            return std::optional<radio_result>{};
          auto state = s_.read(0x550);
          if (!state || *state)
            return std::optional<radio_result>{};
          if (s_.routes_referenced())
            return std::unexpected(system_error::busy);
          if (!strict(false) || !s_.write(0x200, 0))
            return std::unexpected(system_error::mmio);
          auto crc = s_.read(0x400), match = s_.read(0x408),
               rxcrc = s_.read(0x40C), pdu = s_.read(0x414);
          if (!crc || !match || !rxcrc || !pdu)
            return std::unexpected(system_error::mmio);
          result_.crc_valid = !config_.crc.bytes || *crc == 1;
          result_.address_index = *match;
          result_.received_crc = *rxcrc;
          result_.truncated = receiving_ && (*pdu & 1) != 0;
          if (receiving_ && !result_.invalid_coding &&
              !result_.service_overrun) {
            auto bytes = packet_bytes(rx_storage_, true);
            if (!bytes || *bytes > rx_storage_.size())
              result_.truncated = true;
            else {
              result_.bytes = *bytes;
              if (config_.phy == radio_phy::ieee802154 && *bytes)
                result_.link_quality =
                    std::min(unsigned(rx_storage_[*bytes - 1]) * 4, 255U);
            }
          }
          if (observed_ & event_bit(0x11C)) {
            auto sample = s_.read(0x548);
            if (!sample)
              return std::unexpected(system_error::mmio);
            result_.rssi_sample = *sample & 127;
          }
          if (!s_.release_quiet(dma_))
            return std::unexpected(system_error::mmio);
          return std::optional{result_};
        });
  }
  // 243: call after ADDRESS+1us / FRAMESTART and before RX DISABLED when a
  // caller-owned timer/PPI schedule immediately turns reception into coded TX.
  // Native single-packet calls always pass through DISABLED between operations.
  [[nodiscard]] system_result
  prepare_coded_turnaround(radio_phy next) noexcept {
    if (!dma_ || !receiving_ || !long_range() ||
        (next != radio_phy::coded125 && next != radio_phy::coded500))
      return std::unexpected(system_error::invalid);
    auto pdu = s_.read(0x414);
    if (!pdu)
      return fail();
    auto ci = (*pdu >> 1) & 3;
    if (ci > 1)
      return std::unexpected(system_error::invalid);
    auto value = ci == 1 ? (next == radio_phy::coded500 ? 144U : 154U)
                         : (next == radio_phy::coded500 ? 139U : 149U);
    return s_.write(0x544, value) ? system_result{} : fail();
  }
  [[nodiscard]] system_result
  device_addresses(std::span<const radio_device_address> addresses) noexcept {
    if (!initialized_ || dma_ || auxiliary_ || addresses.size() > 8 ||
        s_.routes_referenced())
      return std::unexpected(system_error::busy);
    unsigned config = 0;
    for (unsigned i = 0; i < addresses.size(); ++i) {
      if (!s_.write(0x600 + 4 * i, addresses[i].low) ||
          !s_.write(0x620 + 4 * i, addresses[i].high))
        return fail();
      config |= 1U << i;
      if (addresses[i].random)
        config |= 1U << (8 + i);
    }
    return s_.write(0x640, config) ? system_result{} : fail();
  }
  [[nodiscard]] system_result match_mac_header(std::uint32_t value,
                                               std::uint32_t mask) noexcept {
    if (!initialized_ || config_.phy != radio_phy::ieee802154 || dma_ ||
        auxiliary_)
      return std::unexpected(system_error::busy);
    return s_.write(0x644, value) && s_.write(0x648, mask) ? system_result{}
                                                           : fail();
  }
  [[nodiscard]] system_result bit_counter(unsigned bits,
                                          bool start = true) noexcept {
    if (!initialized_ || !dma_ || (start && !(observed_ & event_bit(0x104))))
      return std::unexpected(system_error::busy);
    if (!s_.write(0x560, bits) || !s_.write(start ? 0x1C : 0x20, 1))
      return fail();
    return {};
  }
  [[nodiscard]] system_value<radio_energy>
  energy_detect(unsigned periods = 0) noexcept {
    auto begun = begin_assessment();
    if (!begun)
      return std::unexpected(begun.error());
    if (periods > 0x1FFFFF || !s_.write(0x664, periods) ||
        !s_.write(0x13C, 0) || !s_.write(0x24, 1))
      return std::unexpected(system_error::invalid);
    if (!wait_event(0x13C))
      return std::unexpected(system_error::timeout);
    auto sample = s_.read(0x668);
    if (!sample)
      return std::unexpected(system_error::mmio);
    auto off = cancel();
    if (!off)
      return std::unexpected(off.error());
    unsigned raw = *sample & 255;
    return radio_energy{raw, std::min(raw * 4, 255U), int(raw) - 92};
  }
  [[nodiscard]] system_value<bool>
  clear_channel(unsigned mode = 0, unsigned energy_threshold = 20,
                unsigned correlation_threshold = 45,
                unsigned correlation_count = 5) noexcept {
    if (mode > 4 || energy_threshold > 255 || correlation_threshold > 255 ||
        correlation_count > 255)
      return std::unexpected(system_error::invalid);
    auto begun = begin_assessment();
    if (!begun)
      return std::unexpected(begun.error());
    if (!s_.write(0x66C, mode | (energy_threshold << 8) |
                             (correlation_threshold << 16) |
                             (correlation_count << 24)) ||
        !s_.write(0x144, 0) || !s_.write(0x148, 0) || !s_.write(0x2C, 1))
      return std::unexpected(system_error::mmio);
    bool clear = false;
    if (!s_.bounded(
            [&] {
              auto a = s_.read(0x144), b = s_.read(0x148);
              if (a && *a) {
                clear = true;
                return true;
              }
              return b && *b;
            },
            timeout_, PollBudget))
      return std::unexpected(system_error::timeout);
    auto off = cancel();
    if (!off)
      return std::unexpected(off.error());
    return clear;
  }
  [[nodiscard]] system_result cancel() noexcept {
    if (s_.context.execution.in_isr())
      return std::unexpected(system_error::invalid);
    if (!s_.owned())
      return {};
    if (s_.routes_referenced())
      return std::unexpected(system_error::busy);
    if (!s_.write(0x200, 0) || !s_.write(0x10, 1))
      return fail();
    if (!s_.bounded(
            [&] {
              auto state = s_.read(0x550);
              return state && !*state;
            },
            timeout_, PollBudget))
      return std::unexpected(system_error::timeout);
    if (!strict(false) || !s_.release_quiet(dma_))
      return fail();
    auxiliary_ = false;
    stopping_ = true;
    return {};
  }
  [[nodiscard]] system_result shutdown() noexcept {
    if (!s_.owned())
      return {};
    auto off = cancel();
    if (!off)
      return off;
    if (!s_.write(0x308, irq_mask) || !clear_events() || !s_.write(0x504, 0) ||
        !s_.write(0xFFC, 0))
      return fail();
    auto r = s_.release();
    if (r)
      initialized_ = fault_ = false;
    return r;
  }
  [[nodiscard]] system_result interrupt_delivery(bool on) noexcept {
    if (!initialized_)
      return std::unexpected(system_error::not_initialized);
    // 228: SYNC produces no IRQ; it remains pollable and routable by PPI.
    return s_.write(on ? 0x304 : 0x308, irq_mask & ~(1U << 26))
               ? system_result{}
               : fail();
  }
  [[nodiscard]] system_endpoint event_endpoint(unsigned offset) const noexcept {
    return s_.endpoint(offset, endpoint_kind::event);
  }
  [[nodiscard]] system_endpoint task_endpoint(unsigned offset) const noexcept {
    return dma_ ? s_.endpoint(offset, endpoint_kind::task) : system_endpoint{};
  }

  [[nodiscard]] bool event_observed(unsigned offset) const noexcept {
    return (observed_ & event_bit(offset)) != 0;
  }
  void clear_observed_event(unsigned offset) noexcept {
    s_.context.execution.critical([&] { observed_ &= ~event_bit(offset); });
  }

private:
  static bool coded(radio_phy p) {
    return p == radio_phy::coded125 || p == radio_phy::coded500;
  }
  bool long_range() const { return coded(config_.phy); }
  static bool valid(radio_configuration c) {
    auto p = unsigned(c.phy);
    bool known =
        p == 0 || p == 1 || p == 3 || p == 4 || p == 5 || p == 6 || p == 15;
    bool power = false;
    for (auto value :
         std::array{8, 7, 6, 5, 4, 3, 2, 0, -4, -8, -12, -16, -20, -30, -40})
      if (value == c.power_dbm)
        power = true;
    auto &f = c.format;
    auto &crc = c.crc;
    if (!known || !power || c.frequency_mhz < 2360 || c.frequency_mhz > 2500 ||
        c.idle_carrier > 2 || c.transmit_address > 7 || !c.receive_addresses ||
        c.receive_addresses > 255 || c.whitening_iv > 127 || c.sfd > 255 ||
        c.tifs_us > 65535 || c.ticks_per_second < 2000)
      return false;
    if (!c.automatic_packet_format &&
        (f.s0_bytes > 1 || f.length_bits > 8 || f.s1_bits > 8 ||
         f.maximum_payload > 255 || f.static_payload > f.maximum_payload ||
         f.base_bytes > 4 || crc.bytes > 3 || crc.skip_address > 2 ||
         crc.polynomial > 0xFFFFFF || crc.initial > 0xFFFFFF ||
         (f.whitening && crc.bytes && crc.skip_address == 0) || // 245
         f.maximum_payload + f.s0_bytes + (f.length_bits ? 1 : 0) +
                 (f.s1_bits || f.s1_in_ram ? 1 : 0) >
             258))
      return false;
    return true;
  }
  unsigned header_bytes() const {
    auto &f = config_.format;
    return f.s0_bytes + (f.length_bits ? 1 : 0) +
           (f.s1_bits || f.s1_in_ram ? 1 : 0);
  }
  system_value<std::size_t> packet_bytes(std::span<const std::uint8_t> packet,
                                         bool rx) const {
    auto &f = config_.format;
    auto header = header_bytes();
    if (packet.size() < header)
      return std::unexpected(system_error::invalid);
    unsigned dynamic =
        f.length_bits ? (packet[f.s0_bytes] & ((1U << f.length_bits) - 1)) : 0;
    if (f.crc_in_length) {
      if (dynamic < config_.crc.bytes)
        return std::unexpected(system_error::invalid);
      dynamic -= config_.crc.bytes;
    }
    if (dynamic + f.static_payload > f.maximum_payload)
      return std::unexpected(system_error::invalid);
    return header + dynamic + f.static_payload +
           (rx && config_.phy == radio_phy::ieee802154 ? 1U : 0U);
  }
  system_result start_packet(const void *data, std::size_t bytes, bool rx,
                             bool now) {
    if (!initialized_ || dma_ || auxiliary_ || fault_ ||
        s_.routes_referenced() || s_.context.execution.in_isr())
      return std::unexpected(system_error::busy);
    auto state = s_.read(0x550);
    if (!state || *state)
      return std::unexpected(system_error::busy);
    auto lease = s_.prepare(data, bytes, bytes, {258, 1, 4},
                            rx ? dma_access::write : dma_access::read);
    if (!lease)
      return std::unexpected(system_error::invalid);
    dma_ = *lease;
    bool special = rx && long_range() && anomaly172_;
    unsigned shorts =
        (special ? 0U : 1U) |
        ((long_range() || config_.phy == radio_phy::ieee802154) ? 1U << 20
                                                                : 2U) |
        (rx ? 16U | 256U : 0U);
    if (!clear_events() || !s_.write(0x504, s_.address(data, bytes)) ||
        !s_.write(0x200, shorts) || !strict(special)) {
      s_.discard(dma_);
      return fail();
    }
    if (!s_.buffers.arm(*dma_)) {
      s_.discard(dma_);
      return fail();
    }
    receiving_ = rx;
    if (rx)
      rx_storage_ = {static_cast<std::uint8_t *>(const_cast<void *>(data)),
                     bytes};
    result_ = {};
    result_.receive = rx;
    result_.bytes = rx ? 0 : bytes;
    observed_ = 0;
    stopping_ = disabled_seen_ = end_seen_ = phy_end_seen_ = coding_checked_ =
        receiver_started_ = address_seen_ = rssi_pending_ = false;
    last_poll_ = s_.context.execution.now();
    next_check_ = {static_cast<execution::tick>(
        last_poll_ + config_.ticks_per_second / 100)};
    if (now && !s_.write(rx ? 4 : 0, 1))
      return fail();
    return {};
  }
  bool clear_events() {
    for (auto off : event_offsets)
      if (!s_.write(off, 0))
        return false;
    return true;
  }
  bool wait_event(unsigned off) {
    return s_.bounded(
        [&] {
          auto r = s_.read(off);
          return r && *r;
        },
        timeout_, PollBudget);
  }
  system_result request_disable() {
    stopping_ = true;
    if (!s_.write(0x10, 1))
      return fail();
    return {};
  }
  bool strict(bool on) {
    if (!anomaly172_)
      return true;
    auto dbc = s_.read(0x73C), peak = s_.read(0x77C);
    if (!dbc || !peak)
      return false;
    if (!s_.write(0x73C, on ? (*dbc & 0x7FFFFF00) | 0x8000007D : 0x40003034) ||
        !s_.write(0x77C, on ? (*peak & 0x7FFFFF8F) | 0x80000060
                            : (*peak & 0x7FFFFFFF)))
      return false;
    strict_ = on;
    return true;
  }
  system_result service_172() {
    if (observed_ & event_bit(0x104)) {
      address_seen_ = true;
    }
    if (address_seen_)
      return {};
    if (!receiver_started_ && (observed_ & event_bit(0x100))) {
      receiver_started_ = true;
      if (!s_.write(8, 1) || !begin_rssi())
        return fail();
    }
    if (rssi_pending_) {
      if (!(observed_ & event_bit(0x11C)))
        return {};
      auto sample = s_.read(0x548), counts = s_.read(0x574);
      if (!sample || !counts)
        return fail();
      bool want = *sample < 95 ||
                  (!strict_ && (*counts & 65535) > 15 && (*counts >> 16) < 2);
      if (!strict(want) || !s_.write(0x40, 1) || !s_.write(0x38, 1) ||
          !s_.write(0x11C, 0))
        return fail();
      rssi_pending_ = false;
      next_check_ = {static_cast<execution::tick>(
          s_.context.execution.now() + config_.ticks_per_second / 100)};
    } else if (receiver_started_ &&
               execution::reached(s_.context.execution.now(), next_check_)) {
      if (!begin_rssi())
        return fail();
    }
    return {};
  }
  bool begin_rssi() {
    if (!s_.write(0x11C, 0))
      return false;
    observed_ &= ~event_bit(0x11C);
    rssi_pending_ = true;
    return s_.write(0x14, 1);
  }
  system_result begin_assessment() {
    if (!initialized_ || config_.phy != radio_phy::ieee802154 || dma_ ||
        auxiliary_ || fault_ || s_.routes_referenced() ||
        s_.context.execution.in_isr())
      return std::unexpected(system_error::busy);
    auxiliary_ = true;
    if (!s_.write(0x200, 0) || !s_.write(0x100, 0) || !s_.write(4, 1))
      return fail();
    if (!wait_event(0x100))
      return std::unexpected(system_error::timeout);
    return {};
  }
  bool frontend_configuration() {
    auto rx = s_.context.read(0x10000330), a = s_.context.read(0x10000334),
         b = s_.context.read(0x10000338);
    if (!rx || !a || !b)
      return false;
    if (*rx != 0xFFFFFFFF && !s_.write(0x74C, *rx))
      return false;
    if (config_.phy == radio_phy::ieee802154) {
      if ((*a != 0xFFFFFFFF && !s_.write(0x584, *a)) ||
          (*b != 0xFFFFFFFF && !s_.write(0x588, *b)))
        return false;
    } else {
      if (*a != 0xFFFFFFFF) {
        auto old = s_.read(0x584);
        if (!old || !s_.write(0x584, (*old & 0x0BFFFFFF) | 0x10000))
          return false;
      }
      if (*b != 0xFFFFFFFF) {
        auto old = s_.read(0x588);
        if (!old || !s_.write(0x588, *old & 0x0BFFFFFF))
          return false;
      }
    }
    return true;
  }
  static constexpr std::uint32_t event_bit(unsigned off) {
    for (unsigned i = 0; i < event_offsets.size(); ++i)
      if (event_offsets[i] == off)
        return 1U << i;
    return 0;
  }
  system_result fail() {
    fault_ = true;
    return std::unexpected(system_error::mmio);
  }
  static constexpr std::uintptr_t base = 0x40001000;
  static constexpr std::uint32_t irq_mask = 0x0CFFF4FF;
  static constexpr std::array<unsigned, 23> event_offsets{
      0x100, 0x104, 0x108, 0x10C, 0x110, 0x114, 0x118, 0x11C,
      0x128, 0x130, 0x134, 0x138, 0x13C, 0x140, 0x144, 0x148,
      0x14C, 0x150, 0x154, 0x158, 0x15C, 0x168, 0x16C};
  session s_;
  radio_configuration config_{};
  execution::tick timeout_, last_poll_{};
  execution::deadline next_check_{};
  std::optional<dma_buffer_token> dma_;
  std::span<std::uint8_t> rx_storage_;
  radio_result result_{};
  std::uint32_t observed_{};
  bool initialized_{}, fault_{}, receiving_{}, auxiliary_{}, stopping_{},
      disabled_seen_{}, end_seen_{}, phy_end_seen_{}, coding_checked_{},
      anomaly172_{}, strict_{}, receiver_started_{}, address_seen_{},
      rssi_pending_{};
};
// Explicit optional stack boundary. The caller starts/stops its chosen stack;
// this class supplies exclusion only and never installs SoftDevice implicitly.
template <class Context> class radio_stack_reservation {
public:
  explicit radio_stack_reservation(Context &c) : c_(c) {}
  ~radio_stack_reservation() {
    if (owned_)
      c_.execution.active_transfer_destroyed();
  }
  [[nodiscard]] system_result acquire(bool stack_owns_irqs_and_clock) noexcept {
    if (!stack_owns_irqs_and_clock || owned_)
      return std::unexpected(system_error::invalid);
    auto radio = c_.claim(76, endpoint_domain::radio);
    if (!radio)
      return radio;
    auto crypto = c_.claim(77, endpoint_domain::ccm);
    if (!crypto) {
      (void)c_.release(76);
      return crypto;
    }
    owned_ = true;
    c_.work_started();
    return {};
  }
  [[nodiscard]] system_result release(bool stack_quiescent) noexcept {
    if (!owned_)
      return {};
    if (!stack_quiescent || c_.referenced(76) || c_.referenced(77))
      return std::unexpected(system_error::busy);
    auto a = c_.release(77);
    if (!a)
      return a;
    auto b = c_.release(76);
    if (!b)
      return b;
    owned_ = false;
    c_.work_stopped();
    return {};
  }

private:
  Context &c_;
  bool owned_{};
};
} // namespace mcubsp::backend::nrf52840
