#pragma once
// @req-file REQ-0068 REQ-0062
// USB 2.0 chapter 9, CDC PSTN 1.2 ACM and HID 1.11 boot keyboard.
// Caller selects its own VID/PID. Every DMA packet lives in private Data RAM;
// descriptor constants are copied by the CPU. Foreground poll services control
// requests and low-power transitions; no 1200-baud bootloader reset is
// implicit.
#include <mcubsp/backend/nrf52840/usbd.hpp>
#include <string_view>
namespace mcubsp::backend::nrf52840 {
enum class usb_profile { cdc_acm, hid_keyboard };
template <class Controller, usb_profile Profile, std::uint16_t Vendor,
          std::uint16_t Product>
class usb_device {
  static constexpr bool cdc = Profile == usb_profile::cdc_acm;
  enum class control_stage { idle, in, out, reconfigure };

public:
  explicit usb_device(Controller &controller) noexcept : u_(controller) {}
  usb_device(const usb_device &) = delete;
  usb_device &operator=(const usb_device &) = delete;
  [[nodiscard]] system_result
  initialize(usb_configuration configuration = {}) noexcept {
    auto r = u_.initialize(configuration);
    if (!r)
      return r;
    return u_.connect();
  }
  [[nodiscard]] system_result shutdown() noexcept { return u_.shutdown(); }
  [[nodiscard]] system_result poll() noexcept {
    auto r = u_.poll();
    if (!r)
      return r;
    auto events = u_.take_events();
    if (events.reset || events.removed) {
      configured_ = remote_ = false;
      u_.remote_wakeup_enabled(false);
      tx_busy_ = rx_armed_ = rx_ready_ = notify_busy_ = false;
      stage_ = control_stage::idle;
      pending_.reset();
      line_state_ = break_ = idle_ = leds_ = 0;
      protocol_ = 1;
    }
    if (u_.resume_pending() && u_.suspended()) {
      r = u_.resume();
      if (!r)
        return r;
    }
    if (u_.suspend_pending() && !u_.suspended() && !u_.active_dma()) {
      r = u_.enter_suspend();
      if (!r)
        return r;
    }
    if (u_.suspended())
      return {};
    if (auto setup = u_.take_setup())
      pending_ = setup;
    auto in = u_.take_completed(0, true);
    auto out = u_.take_completed(0, false);
    // A new SETUP can abort a packet while its DMA remains in flight. Never
    // overwrite staging RAM until the controller has proved that DMA quiet.
    if (pending_ && u_.control_quiescent()) {
      setup_ = *pending_;
      pending_.reset();
      stage_ = control_stage::idle;
      r = dispatch();
      if (!r)
        return r;
    } else if (!pending_) {
      if (in && in->setup_generation == setup_.generation && !in->cancelled &&
          stage_ == control_stage::in) {
        if (in->overrun || !in->acknowledged)
          return stall();
        if (response_at_ < response_size_ || response_zlp_) {
          r = send_control_packet();
          if (!r)
            return r;
        } else {
          r = status();
          if (!r)
            return r;
        }
      }
      if (out && out->setup_generation == setup_.generation &&
          !out->cancelled && stage_ == control_stage::out) {
        if (out->overrun || out->bytes != setup_.length)
          return stall();
        if constexpr (cdc) {
          const auto baud = unsigned(packet_[0]) | unsigned(packet_[1]) << 8 |
                            unsigned(packet_[2]) << 16 |
                            unsigned(packet_[3]) << 24;
          if (!baud || packet_[4] > 2 || packet_[5] > 4 || packet_[6] < 5 ||
              packet_[6] > 8)
            return stall();
          std::copy_n(packet_.begin(), 7, line_coding_.begin());
        } else
          leds_ = packet_[0] & 31;
        r = status();
        if (!r)
          return r;
      }
    }
    if (stage_ == control_stage::reconfigure) {
      r = reconfigure();
      if (!r)
        return r;
    }
    if (auto done = u_.take_completed(1, true)) {
      tx_busy_ = false;
      if (!done->cancelled && !done->overrun)
        last_report_at_ = u_.now();
    }
    if constexpr (cdc) {
      if (auto done = u_.take_completed(1, false)) {
        rx_armed_ = false;
        if (!done->cancelled && !done->overrun) {
          rx_size_ = done->bytes;
          rx_at_ = 0;
          rx_ready_ = true;
        }
      }
      if (auto done = u_.take_completed(2, true)) {
        notify_busy_ = false;
        if (!done->cancelled && notify_at_ < 10) {
          std::copy_n(notification_.begin() + notify_at_, 10 - notify_at_,
                      notify_packet_.begin());
          auto n = 10 - notify_at_;
          notify_at_ = 10;
          r = u_.transmit(2, notify_packet_, n);
          if (!r)
            return r;
          notify_busy_ = true;
        }
      }
      if (configured_ && !rx_armed_ && !rx_ready_) {
        r = u_.receive(1, rx_, 64);
        if (!r)
          return r;
        rx_armed_ = true;
      }
    } else {
      if (configured_ && idle_ && !tx_busy_ &&
          static_cast<execution::tick>(u_.now() - last_report_at_) >=
              std::uint64_t(u_.ticks_per_second()) * idle_ * 4 / 1000) {
        r = send_report(report_);
        if (!r)
          return r;
      }
    }
    return {};
  }
  [[nodiscard]] bool configured() const noexcept { return configured_; }
  [[nodiscard]] bool transmitting() const noexcept { return tx_busy_; }
  [[nodiscard]] std::uint16_t control_lines() const noexcept {
    return line_state_;
  }
  [[nodiscard]] std::uint16_t break_duration() const noexcept { return break_; }
  [[nodiscard]] std::array<std::uint8_t, 7> line_coding() const noexcept {
    return line_coding_;
  }
  [[nodiscard]] unsigned leds() const noexcept { return leds_; }
  [[nodiscard]] system_result
  write(std::span<const std::uint8_t> bytes) noexcept
    requires(cdc)
  {
    if (!configured_ || tx_busy_ || bytes.size() > 64 || u_.suspended())
      return std::unexpected(system_error::busy);
    std::copy(bytes.begin(), bytes.end(), tx_.begin());
    auto r = u_.transmit(1, tx_, bytes.size());
    if (r)
      tx_busy_ = true;
    return r;
  }
  [[nodiscard]] std::size_t read(std::span<std::uint8_t> bytes) noexcept
    requires(cdc)
  {
    if (!rx_ready_)
      return 0;
    auto n = std::min(bytes.size(), rx_size_ - rx_at_);
    std::copy_n(rx_.begin() + rx_at_, n, bytes.begin());
    rx_at_ += n;
    if (rx_at_ == rx_size_)
      rx_ready_ = false;
    return n;
  }
  [[nodiscard]] system_result notify_serial_state(std::uint16_t state) noexcept
    requires(cdc)
  {
    if (!configured_ || notify_busy_ || u_.suspended())
      return std::unexpected(system_error::busy);
    notification_ = {0xA1,
                     0x20,
                     0,
                     0,
                     0,
                     0,
                     2,
                     0,
                     std::uint8_t(state),
                     std::uint8_t(state >> 8)};
    std::copy_n(notification_.begin(), 8, notify_packet_.begin());
    auto r = u_.transmit(2, notify_packet_, 8);
    if (r) {
      notify_busy_ = true;
      notify_at_ = 8;
    }
    return r;
  }
  [[nodiscard]] system_result
  send_report(std::array<std::uint8_t, 8> report) noexcept
    requires(!cdc)
  {
    if (!configured_ || tx_busy_ || u_.suspended())
      return std::unexpected(system_error::busy);
    report_ = report;
    std::copy(report.begin(), report.end(), tx_.begin());
    auto r = u_.transmit(1, tx_, 8);
    if (r)
      tx_busy_ = true;
    return r;
  }

private:
  system_result status() {
    stage_ = control_stage::idle;
    return u_.control_status(setup_.generation);
  }
  system_result stall() {
    stage_ = control_stage::idle;
    return u_.control_stall(setup_.generation);
  }
  system_result response(std::span<const std::uint8_t> bytes) {
    response_size_ = std::min<std::size_t>(setup_.length, bytes.size());
    if (response_size_ > response_.size())
      return stall();
    std::copy_n(bytes.begin(), response_size_, response_.begin());
    response_at_ = 0;
    response_zlp_ = response_size_ < setup_.length && response_size_ % 64 == 0;
    stage_ = control_stage::in;
    return send_control_packet();
  }
  system_result send_control_packet() {
    auto n = std::min(std::size_t{64}, response_size_ - response_at_);
    std::copy_n(response_.begin() + response_at_, n, packet_.begin());
    if (!n)
      response_zlp_ = false;
    auto r = u_.transmit(0, packet_, n);
    if (r)
      response_at_ += n;
    return r;
  }
  bool valid_interface(unsigned i) const {
    return configured_ && i < (cdc ? 2U : 1U);
  }
  bool valid_endpoint(unsigned address) const {
    if (address & ~0x8FU)
      return false;
    if (!(address & 15))
      return true;
    return configured_ &&
           (address == 0x81 || (cdc && (address == 1 || address == 0x82)));
  }
  system_result dispatch() {
    auto &q = setup_;
    auto recipient = q.request_type & 31;
    const bool in = q.request_type & 128;
    if ((q.request_type & 0x60) == 0) {
      switch (q.request) {
      case 5: // SET_ADDRESS is performed entirely by USBD, including status.
        if (q.request_type != 0 || q.index || q.length || q.value > 127)
          return stall();
        return {};
      case 6:
        if (!in || (recipient != 0 && recipient != 1))
          return stall();
        return descriptor();
      case 8:
        if (q.request_type != 0x80 || q.value || q.index || q.length != 1)
          return stall();
        {
          std::array<std::uint8_t, 1> v{std::uint8_t(configured_)};
          return response(v);
        }
      case 9:
        if (q.request_type != 0 || q.value > 1 || q.index || q.length)
          return stall();
        desired_config_ = q.value;
        reconfigure_mask_ = 0x1FE;
        stage_ = control_stage::reconfigure;
        return {};
      case 0: {
        if (!in || q.value || q.length != 2 || recipient > 2)
          return stall();
        std::array<std::uint8_t, 2> value{};
        if (!recipient) {
          if (q.index)
            return stall();
          value[0] = remote_ ? 2 : 0;
        } else if (recipient == 1) {
          if (!valid_interface(q.index))
            return stall();
        } else {
          if (!valid_endpoint(q.index))
            return stall();
          auto h = u_.halted(q.index & 15, q.index & 128);
          if (!h)
            return std::unexpected(h.error());
          value[0] = *h;
        }
        return response(value);
      }
      case 1:
      case 3:
        if (in || q.length)
          return stall();
        if (!recipient && !q.index && q.value == 1 && configured_) {
          remote_ = q.request == 3;
          u_.remote_wakeup_enabled(remote_);
          return status();
        }
        if (recipient == 2 && q.value == 0 && (q.index & 15) &&
            valid_endpoint(q.index)) {
          auto r = u_.halt(q.index & 15, q.index & 128, q.request == 3);
          return r ? status() : r;
        }
        return stall();
      case 10:
        if (q.request_type != 0x81 || q.value || q.length != 1 ||
            !valid_interface(q.index))
          return stall();
        {
          std::array<std::uint8_t, 1> v{};
          return response(v);
        }
      case 11:
        if (q.request_type != 1 || q.value || q.length ||
            !valid_interface(q.index))
          return stall();
        desired_config_ = configured_;
        reconfigure_mask_ = cdc && q.index == 0 ? 4 : 2;
        stage_ = control_stage::reconfigure;
        return {};
      default:
        return stall();
      }
    }
    if ((q.request_type & 0x7F) != 0x21 || q.index || !configured_)
      return stall();
    if constexpr (cdc) {
      switch (q.request) {
      case 0x20:
        if (in || q.value || q.length != 7)
          return stall();
        stage_ = control_stage::out;
        return u_.receive(0, packet_, 7);
      case 0x21:
        if (!in || q.value || q.length != 7)
          return stall();
        return response(line_coding_);
      case 0x22:
        if (in || q.length || q.value & ~3U)
          return stall();
        line_state_ = q.value;
        return status();
      case 0x23:
        if (in || q.length)
          return stall();
        break_ = q.value;
        return status();
      default:
        return stall();
      }
    } else {
      switch (q.request) {
      case 1:
        if (!in || (q.value & 255))
          return stall();
        if (q.value == 0x100)
          return response(report_);
        if (q.value == 0x200) {
          std::array<std::uint8_t, 1> v{std::uint8_t(leds_)};
          return response(v);
        }
        return stall();
      case 9:
        if (in || q.value != 0x200 || q.length != 1)
          return stall();
        stage_ = control_stage::out;
        return u_.receive(0, packet_, 1);
      case 2:
        if (!in || q.value || q.length != 1)
          return stall();
        {
          std::array<std::uint8_t, 1> v{std::uint8_t(idle_)};
          return response(v);
        }
      case 10:
        if (in || (q.value & 255) || q.length)
          return stall();
        idle_ = q.value >> 8;
        return status();
      case 3:
        if (!in || q.value || q.length != 1)
          return stall();
        {
          std::array<std::uint8_t, 1> v{std::uint8_t(protocol_)};
          return response(v);
        }
      case 11:
        if (in || q.value > 1 || q.length)
          return stall();
        protocol_ = q.value;
        return status();
      default:
        return stall();
      }
    }
  }
  system_result reconfigure() {
    auto r = u_.deconfigure_endpoints(reconfigure_mask_);
    if (!r)
      return r.error() == system_error::busy ? system_result{} : r;
    for (unsigned ep = 1; ep <= 8; ++ep)
      if (reconfigure_mask_ & (1U << ep)) {
        (void)u_.take_completed(ep, true);
        (void)u_.take_completed(ep, false);
      }
    if (reconfigure_mask_ & 2)
      tx_busy_ = rx_ready_ = rx_armed_ = false;
    if (reconfigure_mask_ & 4)
      notify_busy_ = false;
    if (desired_config_) {
      if (reconfigure_mask_ & 2) {
        r = u_.configure_endpoint(1, true,
                                  cdc ? usb_endpoint_type::bulk
                                      : usb_endpoint_type::interrupt,
                                  cdc ? 64 : 8);
        if (!r)
          return r;
        if constexpr (cdc) {
          r = u_.configure_endpoint(1, false, usb_endpoint_type::bulk, 64);
          if (!r)
            return r;
        }
      }
      if constexpr (cdc)
        if (reconfigure_mask_ & 4) {
          r = u_.configure_endpoint(2, true, usb_endpoint_type::interrupt, 8);
          if (!r)
            return r;
        }
    }
    configured_ = desired_config_;
    if (!configured_) {
      remote_ = false;
      u_.remote_wakeup_enabled(false);
    }
    return status();
  }
  system_result descriptor() {
    const auto type = setup_.value >> 8, number = setup_.value & 255;
    if (type != 3 && (number || setup_.index))
      return stall();
    if (type == 1 && setup_.request_type == 0x80) {
      const std::array<std::uint8_t, 18> d{18,
                                           1,
                                           0,
                                           2,
                                           std::uint8_t(cdc ? 2 : 0),
                                           0,
                                           0,
                                           64,
                                           std::uint8_t(Vendor),
                                           std::uint8_t(Vendor >> 8),
                                           std::uint8_t(Product),
                                           std::uint8_t(Product >> 8),
                                           0,
                                           1,
                                           1,
                                           2,
                                           0,
                                           1};
      return response(d);
    }
    if (type == 2 && setup_.request_type == 0x80) {
      if constexpr (cdc)
        return response(cdc_config_);
      else
        return response(hid_config_);
    }
    if (type == 3 && setup_.request_type == 0x80) {
      if (!number) {
        const std::array<std::uint8_t, 4> lang{4, 3, 9, 4};
        return response(lang);
      }
      if (setup_.index != 0x409 || number > 2)
        return stall();
      auto text = number == 1 ? std::string_view{"mcubsp"}
                              : (cdc ? std::string_view{"CDC ACM"}
                                     : std::string_view{"HID Keyboard"});
      std::array<std::uint8_t, 32> utf{};
      utf[0] = 2 + 2 * text.size();
      utf[1] = 3;
      for (unsigned i = 0; i < text.size(); ++i)
        utf[2 + 2 * i] = text[i];
      return response(std::span{utf}.first(utf[0]));
    }
    if constexpr (!cdc)
      if (setup_.request_type == 0x81) {
        if (type == 0x21)
          return response(std::span{hid_config_}.subspan(18, 9));
        if (type == 0x22)
          return response(keyboard_descriptor_);
      }
    return stall();
  }
  inline static constexpr std::array<std::uint8_t, 67> cdc_config_{
      9, 2,    67, 0, 2, 1,    0,    0xA0, 50, 9, 4,    0,    0, 1,
      2, 2,    1,  0, 5, 0x24, 0,    0x10, 1,  5, 0x24, 1,    0, 1,
      4, 0x24, 2,  6, 5, 0x24, 6,    0,    1,  7, 5,    0x82, 3, 8,
      0, 16,   9,  4, 1, 0,    2,    10,   0,  0, 0,    7,    5, 1,
      2, 64,   0,  0, 7, 5,    0x81, 2,    64, 0, 0};
  inline static constexpr std::array<std::uint8_t, 63> keyboard_descriptor_{
      5,    1, 9,    6,    0xA1, 1, 5,    7, 0x19, 0xE0, 0x29, 0xE7, 0x15, 0,
      0x25, 1, 0x75, 1,    0x95, 8, 0x81, 2, 0x95, 1,    0x75, 8,    0x81, 1,
      0x95, 5, 0x75, 1,    5,    8, 0x19, 1, 0x29, 5,    0x91, 2,    0x95, 1,
      0x75, 3, 0x91, 1,    0x95, 6, 0x75, 8, 0x15, 0,    0x25, 0x65, 5,    7,
      0x19, 0, 0x29, 0x65, 0x81, 0, 0xC0};
  inline static constexpr std::array<std::uint8_t, 34> hid_config_{
      9, 2, 34,   0,    1, 1, 0, 0xA0, 50, 9, 4, 0, 0,    1, 3, 1, 1,
      0, 9, 0x21, 0x11, 1, 0, 1, 0x22, 63, 0, 7, 5, 0x81, 3, 8, 0, 10};
  Controller &u_;
  usb_setup setup_{};
  std::optional<usb_setup> pending_;
  control_stage stage_{};
  alignas(4) std::array<std::uint8_t, 64> packet_{}, tx_{}, rx_{};
  std::array<std::uint8_t, 256> response_{};
  alignas(4) std::array<std::uint8_t, 8> notify_packet_{};
  std::array<std::uint8_t, 10> notification_{};
  std::array<std::uint8_t, 8> report_{};
  std::array<std::uint8_t, 7> line_coding_{0, 0xC2, 1, 0, 0, 0, 8};
  std::size_t response_size_{}, response_at_{}, rx_size_{}, rx_at_{},
      notify_at_{};
  execution::tick last_report_at_{};
  unsigned reconfigure_mask_{}, idle_{}, protocol_{1}, leds_{};
  std::uint16_t line_state_{}, break_{};
  bool response_zlp_{}, configured_{}, desired_config_{}, remote_{}, tx_busy_{},
      rx_armed_{}, rx_ready_{}, notify_busy_{};
};
} // namespace mcubsp::backend::nrf52840
