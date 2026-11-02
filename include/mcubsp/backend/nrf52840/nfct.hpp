#pragma once
// @req-file REQ-0070 REQ-0062
// PS v1.11 6.14; anomalies 190 (timed field settling) and 218 (idle delay).
#include <mcubsp/backend/nrf52840/detail/peripheral_session.hpp>
#include <mcubsp/mcu/nrf52840/product/nfct.hpp>
#include <mcubsp/mcu/nrf52840/serial_revision.hpp>
namespace mcubsp::backend::nrf52840 {
struct nfct_configuration {
  // NFC1=P0.9 / NFC2=P0.10 are reserved by the MCU GPIO policy. This explicit
  // attachment declaration is separate from UICR.NFCPINS and never writes UICR.
  bool dedicated_pads_and_antenna{};
  std::array<std::uint8_t, 10> id{};
  unsigned id_bytes{7};
  bool automatic_collision_resolution{true};
  std::uint16_t sensres{0x0004};
  std::uint8_t selres{};
  unsigned ticks_per_second{64'000'000};
};
struct nfct_frame_configuration {
  bool crc{true}, parity{true}, start_of_frame{true}, discard_start{};
};
struct nfct_frame_timing {
  unsigned minimum{0x480}, maximum{0x1000}, mode{1};
};
struct nfct_events {
  bool field_detected{}, field_lost{}, ready{}, selected{}, collision{},
      automatic_collision_started{};
};
struct nfct_frame_result {
  bool receive{}, field_lost{}, cancelled{};
  std::size_t bytes{};
  unsigned extra_bits{};
  std::uint32_t receive_errors{}, timing_errors{};
};
template <class Context, class Buffers, class Address = native_dma_address,
          unsigned PollBudget = 1000000>
class nfct_controller {
  using session = detail::peripheral_session<Context, Buffers, Address>;

public:
  nfct_controller(Context &c, clock_controller<Context> &clocks, Buffers &b,
                  execution::tick timeout = 1'280'000, Address a = {}) noexcept
      : s_(c, clocks, b, a), timeout_(timeout) {}
  nfct_controller(const nfct_controller &) = delete;
  nfct_controller &operator=(const nfct_controller &) = delete;
  ~nfct_controller() {
    if (dma_)
      s_.context.execution.active_transfer_destroyed();
  }
  [[nodiscard]] system_result
  initialize(nfct_configuration configuration) noexcept {
    if (!configuration.dedicated_pads_and_antenna ||
        (configuration.id_bytes != 4 && configuration.id_bytes != 7 &&
         configuration.id_bytes != 10) ||
        !configuration.ticks_per_second || !timeout_ ||
        timeout_ > execution::maximum_deadline_interval ||
        configuration.selres & ~0x60U || configuration.sensres & ~0x0F1FU)
      return std::unexpected(system_error::invalid);
    auto nfc = s_.context.read(0x1000120C), p = s_.context.read(0x10000130),
         r = s_.context.read(0x10000134);
    if (!nfc || !p || !r)
      return std::unexpected(system_error::mmio);
    if (!(*nfc & 1) ||
        !mcu::nrf52840::serial_revision{*p, *r}.reviewed_modern())
      return std::unexpected(system_error::unsupported);
    auto claim = s_.acquire(peripheral::nfct, base, {}, std::nullopt, false);
    if (!claim)
      return claim;
    auto state = s_.read(0x410);
    if (!state || *state) {
      (void)s_.release();
      return std::unexpected(state ? system_error::busy : system_error::mmio);
    }
    config_ = configuration;
    settle_ticks_ =
        (std::uint64_t(configuration.ticks_per_second) + 999) / 1000;
    s_.publish();
    if (!s_.write(0x308, irq_mask) || !s_.write(0x200, 0) || !clear_events() ||
        !s_.write(0x404, 1) || !s_.write(0x40C, 13) ||
        !s_.write(0x59C, !configuration.automatic_collision_resolution) ||
        !s_.write(0x5A0,
                  configuration.sensres | ((configuration.id_bytes == 4   ? 0
                                            : configuration.id_bytes == 7 ? 1
                                                                          : 2)
                                           << 6)) ||
        !s_.write(0x5A4, configuration.selres) || !idle_timing())
      return fail();
    unsigned at = 0;
    if (configuration.id_bytes == 10) {
      if (!s_.write(0x598, pack(configuration.id, at, 3)))
        return fail();
      at += 3;
    }
    if (configuration.id_bytes >= 7) {
      if (!s_.write(0x594, pack(configuration.id, at, 3)))
        return fail();
      at += 3;
    }
    if (!s_.write(0x590, pack(configuration.id, at, 4)))
      return fail();
    initialized_ = true;
    return sense();
  }
  [[nodiscard]] system_result sense() noexcept {
    if (!initialized_ || dma_ || s_.context.execution.in_isr())
      return std::unexpected(system_error::busy);
    if (!s_.write(4, 1) || !wait_disabled() || !idle_timing() ||
        !s_.write(0x104, 0) || !s_.write(8, 1))
      return fail();
    activated_ = ready_ = selected_ = field_pending_ = wake_ready_ = false;
    sensing_ = true;
    return s_.allow_low_power_idle();
  }
  [[nodiscard]] system_result activate() noexcept {
    if (!initialized_ || dma_ || fault_ || !field_pending_ ||
        !execution::reached(s_.context.execution.now(), field_until_))
      return std::unexpected(system_error::busy);
    auto hf = s_.require_high_frequency();
    if (!hf)
      return hf;
    // 190: caller polls after a full 1 ms from FIELDDETECTED. No shortcut can
    // bypass this delay. Clear FIELDDETECTED only AFTER ACTIVATE (PS 6.14.2).
    if (!s_.write(0x100, 0) || !s_.write(0, 1) || !s_.write(0x104, 0))
      return fail();
    activated_ = true;
    sensing_ = field_pending_ = wake_ready_ = false;
    return {};
  }
  [[nodiscard]] system_value<nfct_events> poll_events() noexcept {
    return s_.context.execution.critical([&]() -> system_value<nfct_events> {
      if (!initialized_)
        return std::unexpected(system_error::not_initialized);
      nfct_events result{};
      for (auto off :
           std::array<unsigned, 6>{0x104, 0x108, 0x100, 0x14C, 0x148, 0x138}) {
        auto event = s_.read(off);
        if (!event)
          return std::unexpected(system_error::mmio);
        if (!*event)
          continue;
        if (off == 0x104) {
          result.field_detected = true;
          if (!field_pending_ && !activated_) {
            field_pending_ = true;
            field_until_ = {static_cast<execution::tick>(
                s_.context.execution.now() + settle_ticks_)};
          }
        } else if (off == 0x108) {
          result.field_lost = true;
          field_lost_ = true;
          selected_ = false;
        } else if (off == 0x100) {
          result.ready = true;
          ready_ = true;
        } else if (off == 0x14C) {
          result.selected = true;
          selected_ = true;
        } else if (off == 0x148)
          result.collision = true;
        else
          result.automatic_collision_started = true;
        // A field event remains asserted until activation, preventing the PS
        // duplicate-event condition and avoiding resetting the settling timer.
        if (off != 0x104 && !s_.write(off, 0))
          return std::unexpected(system_error::mmio);
      }
      return result;
    });
  }
  [[nodiscard]] system_result
  receive(std::span<std::uint8_t> output,
          nfct_frame_configuration frame = {}) noexcept {
    return start_frame(output.data(), output.size(), true, 0, frame, {});
  }
  [[nodiscard]] system_result transmit(std::span<const std::uint8_t> input,
                                       unsigned bits = 0,
                                       nfct_frame_configuration frame = {},
                                       nfct_frame_timing timing = {}) noexcept {
    if (bits > 7 || input.empty() || (bits && frame.crc))
      return std::unexpected(system_error::invalid);
    auto full = input.size() - (bits ? 1 : 0);
    return start_frame(input.data(), input.size(), false, (full << 3) | bits,
                       frame, timing);
  }
  [[nodiscard]] system_value<std::optional<nfct_frame_result>>
  poll_frame() noexcept {
    return s_.context.execution.critical(
        [&]() -> system_value<std::optional<nfct_frame_result>> {
          if (!dma_)
            return std::optional<nfct_frame_result>{};
          auto lost = s_.read(0x108),
               done = s_.read(receiving_ ? 0x118 : 0x110),
               error = s_.read(receiving_ ? 0x128 : 0x11C);
          if (!lost || !done || !error)
            return std::unexpected(system_error::mmio);
          field_lost_ = field_lost_ || *lost;
          terminal_ = terminal_ || *done;
          if (field_lost_) {
            auto r = cancel_impl();
            if (!r)
              return std::unexpected(r.error());
            return std::optional{
                nfct_frame_result{receiving_, true, false, 0, 0, 0, 0}};
          }
          // RXERROR or ERROR alone is not DMA/final-wire completion evidence.
          if (*error && !terminal_) {
            auto frame = s_.read(0x40C), timing = s_.read(0x404);
            if (!frame || !timing)
              return std::unexpected(system_error::mmio);
            auto r = cancel_impl();
            if (!r)
              return std::unexpected(r.error());
            return std::optional{nfct_frame_result{receiving_, false, true, 0,
                                                   0, *frame, *timing}};
          }
          if (!terminal_)
            return std::optional<nfct_frame_result>{};
          auto amount = s_.read(receiving_ ? 0x524 : 0x51C),
               frame = s_.read(0x40C), timing = s_.read(0x404);
          if (!amount || !frame || !timing)
            return std::unexpected(system_error::mmio);
          unsigned bytes = (*amount >> 3) & 511, bits = *amount & 7;
          if (!s_.write(receiving_ ? 0x118 : 0x110, 0) ||
              !s_.write(receiving_ ? 0x128 : 0x11C, 0) ||
              !s_.write(0x404, *timing) || !s_.write(0x40C, *frame) ||
              !idle_timing() || !s_.release_quiet(dma_))
            return std::unexpected(system_error::mmio);
          if (bytes + (bits ? 1 : 0) > capacity_)
            return std::unexpected(system_error::faulted);
          return std::optional{nfct_frame_result{receiving_, false, false,
                                                 bytes, bits, *frame, *timing}};
        });
  }
  [[nodiscard]] system_result go_idle(bool sleep = false) noexcept {
    if (!activated_ || dma_)
      return std::unexpected(system_error::busy);
    if (!idle_timing() || !s_.write(sleep ? 0x28 : 0x24, 1))
      return fail();
    selected_ = false;
    return {};
  }
  [[nodiscard]] system_result cancel() noexcept {
    if (s_.context.execution.in_isr())
      return std::unexpected(system_error::invalid);
    return cancel_impl();
  }
  [[nodiscard]] system_result prepare_system_off() noexcept {
    auto r = sense();
    if (!r)
      return r;
    if (!s_.write(0x308, irq_mask))
      return fail();
    wake_ready_ = true;
    return {};
  }
  [[nodiscard]] bool wake_ready() const noexcept { return wake_ready_; }
  [[nodiscard]] system_result interrupt_delivery(bool on) noexcept {
    if (!initialized_)
      return std::unexpected(system_error::not_initialized);
    return s_.write(on ? 0x304 : 0x308, irq_mask) ? system_result{} : fail();
  }
  [[nodiscard]] system_result shutdown() noexcept {
    if (s_.context.execution.in_isr())
      return std::unexpected(system_error::invalid);
    if (!s_.owned())
      return {};
    auto r = cancel_impl();
    if (!r)
      return r;
    if (!s_.write(0x308, irq_mask) || !s_.write(0x200, 0) || !s_.write(4, 1) ||
        !wait_disabled() || !clear_events())
      return fail();
    r = s_.release();
    if (r)
      initialized_ = fault_ = false;
    return r;
  }

private:
  static std::uint32_t pack(const std::array<std::uint8_t, 10> &id, unsigned at,
                            unsigned n) {
    std::uint32_t word = 0;
    for (unsigned i = 0; i < n; ++i)
      word = (word << 8) | id[at + i];
    return word;
  }
  bool idle_timing() {
    return s_.write(0x508, 0x1000);
  } // 218: never leave a custom maximum in IDLE/SLEEP_A.
  bool wait_disabled() {
    return s_.bounded(
        [&] {
          auto r = s_.read(0x410);
          return r && !*r;
        },
        timeout_, PollBudget);
  }
  bool clear_events() {
    for (auto off : event_offsets)
      if (!s_.write(off, 0))
        return false;
    return true;
  }
  system_result start_frame(const void *data, std::size_t bytes, bool rx,
                            unsigned amount, nfct_frame_configuration frame,
                            nfct_frame_timing timing) {
    if (!initialized_ || !activated_ || !ready_ || dma_ || fault_ ||
        s_.context.execution.in_isr() ||
        (config_.automatic_collision_resolution && !selected_))
      return std::unexpected(system_error::busy);
    if (!bytes || bytes > 511 || timing.minimum > 65535 ||
        timing.maximum > 0xFFFFF || timing.minimum > timing.maximum ||
        timing.mode > 3)
      return std::unexpected(system_error::invalid);
    auto lease = s_.prepare(data, bytes, bytes, {511, 1, 4},
                            rx ? dma_access::write : dma_access::read);
    if (!lease)
      return std::unexpected(system_error::invalid);
    dma_ = *lease;
    unsigned format =
        (frame.crc ? 16U : 0U) | (frame.start_of_frame ? 4U : 0U) |
        (frame.parity ? 1U : 0U) | (!rx && frame.discard_start ? 2U : 0U);
    if (!s_.write(rx ? 0x118 : 0x110, 0) || !s_.write(rx ? 0x128 : 0x11C, 0) ||
        !s_.write(0x404, 1) || !s_.write(0x40C, 13) ||
        !s_.write(0x510, s_.address(data, bytes)) || !s_.write(0x514, bytes) ||
        !s_.write(rx ? 0x520 : 0x518, format) ||
        (!rx &&
         (!s_.write(0x51C, amount) || !s_.write(0x504, timing.minimum) ||
          !s_.write(0x508, timing.maximum) || !s_.write(0x50C, timing.mode)))) {
      s_.discard(dma_);
      return fail();
    }
    if (!s_.buffers.arm(*dma_)) {
      s_.discard(dma_);
      return fail();
    }
    receiving_ = rx;
    capacity_ = bytes;
    terminal_ = field_lost_ = false;
    if (!s_.write(rx ? 0x1C : 0xC, 1))
      return fail();
    return {};
  }
  system_result cancel_impl() {
    if (!s_.owned())
      return {};
    if (!s_.write(4, 1) || !wait_disabled() || !s_.release_quiet(dma_))
      return fail();
    activated_ = ready_ = selected_ = sensing_ = field_pending_ = wake_ready_ =
        false;
    return {};
  }
  system_result fail() {
    fault_ = true;
    return std::unexpected(system_error::mmio);
  }
  static constexpr std::uintptr_t base = 0x40005000;
  static constexpr std::uint32_t irq_mask = 0x1C5CFF;
  static constexpr std::array<unsigned, 15> event_offsets{
      0x100, 0x104, 0x108, 0x10C, 0x110, 0x114, 0x118, 0x11C,
      0x128, 0x12C, 0x130, 0x138, 0x148, 0x14C, 0x150};
  session s_;
  nfct_configuration config_{};
  execution::tick timeout_, settle_ticks_{};
  execution::deadline field_until_{};
  std::optional<dma_buffer_token> dma_;
  std::size_t capacity_{};
  bool initialized_{}, fault_{}, sensing_{}, field_pending_{}, activated_{},
      ready_{}, selected_{}, field_lost_{}, wake_ready_{}, terminal_{},
      receiving_{};
};
struct type2_response {
  std::array<std::uint8_t, 16> data{};
  unsigned bytes{}, extra_bits{};
  bool silent{}, sleep{};
};
template <unsigned Pages = 16> class readonly_type2_tag {
  static_assert(Pages >= 16 && Pages <= 256 && Pages % 2 == 0);

public:
  [[nodiscard]] system_result
  initialize(std::array<std::uint8_t, 7> uid,
             std::span<const std::uint8_t> ndef) noexcept {
    auto header = ndef.size() < 255 ? 2U : 4U;
    if (ndef.size() + header + 1 > memory_.size() - 16)
      return std::unexpected(system_error::invalid);
    memory_ = {};
    memory_[0] = uid[0];
    memory_[1] = uid[1];
    memory_[2] = uid[2];
    memory_[3] = 0x88 ^ uid[0] ^ uid[1] ^ uid[2];
    for (unsigned i = 0; i < 4; ++i)
      memory_[4 + i] = uid[3 + i];
    memory_[8] = uid[3] ^ uid[4] ^ uid[5] ^ uid[6];
    memory_[9] = 0x48;
    memory_[10] = memory_[11] = 255;
    memory_[12] = 0xE1;
    memory_[13] = 0x10;
    memory_[14] = (memory_.size() - 16) / 8;
    memory_[15] = 0x0F;
    unsigned at = 16;
    memory_[at++] = 3;
    if (ndef.size() < 255)
      memory_[at++] = ndef.size();
    else {
      memory_[at++] = 255;
      memory_[at++] = ndef.size() >> 8;
      memory_[at++] = ndef.size();
    }
    for (auto byte : ndef)
      memory_[at++] = byte;
    memory_[at] = 0xFE;
    initialized_ = true;
    return {};
  }
  // Pass CRC-stripped frames only after NFCT reports parity/CRC success. READ
  // wraps through page 0 as required by Type 2; writes return a 4-bit NAK.
  [[nodiscard]] type2_response
  respond(std::span<const std::uint8_t> frame) const noexcept {
    type2_response response{};
    if (!initialized_ || frame.empty()) {
      response.silent = true;
      return response;
    }
    if (frame.size() == 2 && frame[0] == 0x30 && frame[1] < Pages) {
      response.bytes = 16;
      auto offset = unsigned(frame[1]) * 4;
      for (unsigned i = 0; i < 16; ++i)
        response.data[i] = memory_[(offset + i) % memory_.size()];
      return response;
    }
    if (frame.size() == 2 && frame[0] == 0x50 && frame[1] == 0) {
      response.silent = response.sleep = true;
      return response;
    }
    response.bytes = 1;
    response.extra_bits = 4;
    response.data[0] = 0;
    return response;
  }

private:
  std::array<std::uint8_t, Pages * 4> memory_{};
  bool initialized_{};
};
} // namespace mcubsp::backend::nrf52840
