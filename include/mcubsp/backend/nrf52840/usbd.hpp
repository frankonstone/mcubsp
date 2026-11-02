#pragma once
// @req-file REQ-0068 REQ-0062
// PS v1.11 6.35. One shared EasyDMA arbiter; packet ACK and DMA END are
// distinct. Anomalies 166,171,187,199 are applied explicitly.
#include <algorithm>
#include <mcubsp/backend/nrf52840/detail/peripheral_session.hpp>
#include <mcubsp/mcu/nrf52840/product/usbd.hpp>
#include <mcubsp/mcu/nrf52840/serial_revision.hpp>
namespace mcubsp::backend::nrf52840 {
enum class usb_endpoint_type : unsigned {
  control,
  isochronous,
  bulk,
  interrupt
};
struct usb_setup {
  std::uint8_t request_type{}, request{};
  std::uint16_t value{}, index{}, length{};
  std::uint32_t generation{};
};
struct usb_packet_result {
  unsigned endpoint{};
  bool in{}, isochronous{}, acknowledged{}, cancelled{}, bus_reset{}, overrun{},
      crc_error{};
  std::size_t bytes{};
  unsigned frame{};
  std::uint32_t setup_generation{};
};
struct usb_events {
  bool reset{}, removed{}, suspend{}, resume{}, sof{};
  unsigned frame{};
};
struct usb_configuration {
  bool split_iso_buffer{}, iso_in_zero_length{true};
  unsigned ticks_per_second{64'000'000};
};
struct bootloader_handoff {
  std::uintptr_t begin{}, end{};
  std::uint8_t gpregret{};
  bool reviewed_bootloader_abi{}, application_quiescent{};
};
template <class Context, class Buffers, class Address = native_dma_address,
          unsigned PollBudget = 1000000>
class usbd_controller {
  using session = detail::peripheral_session<Context, Buffers, Address>;
  enum class stage { free, queued, dma, wire, complete };
  struct endpoint {
    bool enabled{};
    unsigned maximum{};
    usb_endpoint_type type{};
  };
  struct slot {
    stage state{stage::free};
    std::optional<dma_buffer_token> lease;
    const void *data{};
    std::size_t count{}, amount{};
    bool data_ready{}, ack{}, terminal{}, cancel{}, reset{}, overrun{},
        crc_error{};
    unsigned frame{};
    std::uint32_t generation{};
  };

public:
  usbd_controller(Context &c, clock_controller<Context> &clocks, Buffers &b,
                  execution::tick timeout = 1'280'000, Address a = {}) noexcept
      : s_(c, clocks, b, a), timeout_(timeout) {}
  usbd_controller(const usbd_controller &) = delete;
  usbd_controller &operator=(const usbd_controller &) = delete;
  ~usbd_controller() {
    for (auto &v : slots_)
      if (v.lease) {
        s_.context.execution.active_transfer_destroyed();
        break;
      }
  }
  [[nodiscard]] system_result
  initialize(usb_configuration configuration = {}) noexcept {
    if (!timeout_ || timeout_ > execution::maximum_deadline_interval ||
        !configuration.ticks_per_second)
      return std::unexpected(system_error::invalid);
    auto p = s_.context.read(0x10000130), r = s_.context.read(0x10000134),
         power = s_.context.read(0x40000438);
    if (!p || !r || !power)
      return std::unexpected(system_error::mmio);
    if (!mcu::nrf52840::serial_revision{*p, *r}.reviewed_modern())
      return std::unexpected(system_error::unsupported);
    if (!(*power & 1))
      return std::unexpected(system_error::not_initialized);
    auto acquired = s_.acquire(peripheral::usbd, base);
    if (!acquired)
      return acquired;
    config_ = configuration;
    s_.publish();
    // 187 and 171: retain the forcing latches until READY, with no WFI in the
    // bounded enable handshake. They are cleared on explicit recovery too.
    if (!errata_power(true, true) || !s_.write(0x308, irq_mask) ||
        !s_.write(0x400, 0xF01) || !s_.write(0x500, 1))
      return fail();
    if (!s_.bounded(
            [&] {
              auto ready = s_.read(0x400);
              auto pwr = s_.context.read(0x40000438);
              return ready && (*ready & 2048) && pwr && (*pwr & 3) == 3;
            },
            timeout_, PollBudget))
      return std::unexpected(system_error::timeout);
    if (!s_.write(0x400, 2048) || !s_.write(0x158, 0) ||
        !errata_power(false, true) || !s_.write(0x800, 0x7E3) ||
        !s_.write(0x804, 0x40) || !s_.write(0x200, 0) ||
        !s_.write(0x51C, configuration.split_iso_buffer ? 128 : 0) ||
        !s_.write(0x530, configuration.iso_in_zero_length) ||
        !s_.write(0x510, 1) || !s_.write(0x514, 1))
      return fail();
    endpoints_ = {};
    endpoints_[0] = endpoints_[9] = {true, 64, usb_endpoint_type::control};
    current_.reset();
    disable_requested_ = false;
    initialized_ = true;
    suspended_ = false;
    return {};
  }
  [[nodiscard]] system_result connect(bool on = true) noexcept {
    if (!awake() || s_.context.execution.in_isr())
      return std::unexpected(system_error::busy);
    if (!s_.write(0x504, on))
      return fail();
    connected_ = on;
    return {};
  }
  [[nodiscard]] system_result configure_endpoint(unsigned ep, bool in,
                                                 usb_endpoint_type type,
                                                 unsigned maximum,
                                                 bool enable = true) noexcept {
    if (!awake() || ep > 8 || slots_[index(ep, in)].state != stage::free ||
        s_.context.execution.in_isr())
      return std::unexpected(system_error::busy);
    if ((ep == 0 && (type != usb_endpoint_type::control || maximum != 64)) ||
        (ep == 8 && (type != usb_endpoint_type::isochronous || !maximum ||
                     maximum > (config_.split_iso_buffer ? 512U : 1023U))) ||
        (ep > 0 && ep < 8 &&
         (type != usb_endpoint_type::bulk &&
          type != usb_endpoint_type::interrupt)) ||
        (ep < 8 && (!maximum || maximum > 64)) ||
        (type == usb_endpoint_type::bulk && maximum != 8 && maximum != 16 &&
         maximum != 32 && maximum != 64))
      return std::unexpected(system_error::invalid);
    if (ep == 8 && !config_.split_iso_buffer && enable &&
        endpoints_[index(8, !in)].enabled)
      return std::unexpected(system_error::busy);
    auto mask = s_.read(in ? 0x510 : 0x514);
    if (!mask)
      return fail();
    if (!s_.write(in ? 0x510 : 0x514,
                  enable ? *mask | (1U << ep) : *mask & ~(1U << ep)))
      return fail();
    if (ep > 0 && ep < 8 && enable &&
        !s_.write(0x50C, ep | (in ? 128U : 0U) | 256U))
      return fail();
    endpoints_[index(ep, in)] = {enable, maximum, type};
    return {};
  }
  // Storage includes word padding; count is the USB byte length. Even a ZLP
  // has a private, valid RAM pointer. Payload count itself need not be /4.
  [[nodiscard]] system_result transmit(unsigned ep,
                                       std::span<const std::uint8_t> storage,
                                       std::size_t count) noexcept {
    return enqueue(ep, true, storage.data(), storage.size(), count);
  }
  [[nodiscard]] system_result receive(unsigned ep,
                                      std::span<std::uint8_t> storage,
                                      std::size_t maximum) noexcept {
    return enqueue(ep, false, storage.data(), storage.size(), maximum);
  }
  [[nodiscard]] std::optional<usb_packet_result>
  take_completed(unsigned ep, bool in) noexcept {
    return s_.context.execution.critical(
        [&]() -> std::optional<usb_packet_result> {
          if (ep > 8)
            return {};
          auto &v = slots_[index(ep, in)];
          if (v.state != stage::complete)
            return {};
          usb_packet_result r{ep,       in,      ep == 8,     v.ack,
                              v.cancel, v.reset, v.overrun,   v.crc_error,
                              v.amount, v.frame, v.generation};
          v = {};
          return r;
        });
  }
  [[nodiscard]] std::optional<usb_setup> take_setup() noexcept {
    return s_.context.execution.critical([&]() -> std::optional<usb_setup> {
      if (!setup_pending_)
        return {};
      setup_pending_ = false;
      return setup_;
    });
  }
  [[nodiscard]] usb_events take_events() noexcept {
    return s_.context.execution.critical([&] {
      auto r = events_;
      events_ = {};
      return r;
    });
  }
  [[nodiscard]] system_result
  control_status(std::uint32_t generation) noexcept {
    if (!awake() || generation != setup_.generation ||
        (setup_.request_type == 0 && setup_.request == 5))
      return std::unexpected(system_error::invalid);
    return s_.write(0x50, 1) ? system_result{} : fail();
  }
  [[nodiscard]] system_result control_stall(std::uint32_t generation) noexcept {
    if (!awake() || generation != setup_.generation)
      return std::unexpected(system_error::invalid);
    return s_.write(0x54, 1) ? system_result{} : fail();
  }
  [[nodiscard]] system_result halt(unsigned ep, bool in, bool halted) noexcept {
    if (!awake() || ep > 7 || ep == 0)
      return std::unexpected(system_error::invalid);
    if (!s_.write(0x518, ep | (in ? 128U : 0U) | (halted ? 256U : 0U)) ||
        (!halted && !s_.write(0x50C, ep | (in ? 128U : 0U) | 256U)))
      return fail();
    return {};
  }
  [[nodiscard]] system_value<bool> halted(unsigned ep, bool in) noexcept {
    if (!awake() || ep > 7)
      return std::unexpected(system_error::invalid);
    auto r = s_.read((in ? 0x420 : 0x444) + 4 * ep);
    if (!r)
      return std::unexpected(system_error::mmio);
    return (*r & 1) != 0;
  }
  [[nodiscard]] system_result poll() noexcept {
    return s_.context.execution.critical([&]() -> system_result {
      if (!s_.owned())
        return std::unexpected(system_error::not_initialized);
      if (disable_requested_) {
        auto enabled = s_.read(0x500);
        if (!enabled)
          return fail();
        if (!*enabled)
          return finish_disable();
        return {};
      }
      if (!initialized_)
        return std::unexpected(system_error::not_initialized);
      auto power = s_.context.read(0x40000438);
      if (!power)
        return fail();
      if (!(*power & 1)) {
        events_.removed = true;
        return request_disable();
      }
      auto bus = s_.read(0x158);
      if (!bus)
        return fail();
      if (*bus) {
        auto cause = s_.read(0x400);
        if (!cause)
          return fail();
        cause_pending_ |= *cause;
        events_.suspend = events_.suspend || (*cause & 256);
        events_.resume = events_.resume || (*cause & 512);
        if (*cause & 256)
          suspend_pending_ = true;
        if (*cause & 512)
          resume_pending_ = true;
        if (*cause & 1)
          slots_[index(8, false)].crc_error = true;
        if (!s_.write(0x400, *cause & 0xF01) || !s_.write(0x158, 0))
          return fail();
      }
      // Tasks and setup/status/endpoint registers are inaccessible in LOWPOWER.
      // The foreground resumes HFXO and performs 171's wake handshake first.
      if (suspended_)
        return {};
      if (current_) {
        auto &v = slots_[*current_];
        auto event = s_.read(end_event(*current_));
        if (!event)
          return fail();
        v.terminal = v.terminal || *event;
        if (v.terminal) {
          auto amount = s_.read(registers(*current_) + 8);
          if (!amount)
            return fail();
          if (endpoint_number(*current_) == 8) {
            auto frame = s_.read(0x520);
            if (!frame)
              return fail();
            if ((*frame & 2047) != v.frame)
              v.overrun = true;
          }
          v.amount = *amount;
          if (*amount > v.count)
            v.overrun = true;
          if (!s_.write(end_event(*current_), 0) || !s_.write(0xC1C, 0))
            return fail();
          v.state = stage::wire;
          if (!is_in(*current_) || endpoint_number(*current_) == 8 || v.ack ||
              v.cancel) {
            if (!publish(*current_))
              return fail();
          }
          current_.reset();
        }
      }
      auto reset = s_.read(0x100);
      if (!reset)
        return fail();
      if (*reset) {
        reset_pending_ = true;
        events_.reset = true;
        if (!s_.write(0x100, 0))
          return fail();
        for (unsigned i = 0; i < 18; ++i)
          if (slots_[i].state != stage::free &&
              slots_[i].state != stage::complete) {
            slots_[i].cancel = slots_[i].reset = true;
            if (!current_ || *current_ != i) {
              if (!publish(i))
                return fail();
            }
          }
      }
      if (reset_pending_ && !current_) {
        if (!s_.write(0x510, 1) || !s_.write(0x514, 1))
          return fail();
        endpoints_ = {};
        endpoints_[0] = endpoints_[9] = {true, 64, usb_endpoint_type::control};
        reset_pending_ = false;
      }
      auto setup = s_.read(0x15C);
      if (!setup)
        return fail();
      if (*setup) {
        if (setup_.generation == 0xFFFFFFFF)
          return fail();
        usb_setup next{};
        std::array<std::uint32_t, 8> values{};
        for (unsigned i = 0; i < 8; ++i) {
          auto v = s_.read(0x480 + 4 * i);
          if (!v)
            return fail();
          values[i] = *v & 255;
        }
        next = {std::uint8_t(values[0]),
                std::uint8_t(values[1]),
                std::uint16_t(values[2] | values[3] << 8),
                std::uint16_t(values[4] | values[5] << 8),
                std::uint16_t(values[6] | values[7] << 8),
                setup_.generation + 1};
        setup_ = next;
        setup_pending_ = true;
        for (auto i : std::array{0U, 9U})
          if (slots_[i].state != stage::free &&
              slots_[i].state != stage::complete) {
            slots_[i].cancel = true;
            if (!current_ || *current_ != i)
              if (!publish(i))
                return fail();
          }
        if (!s_.write(0x15C, 0) || !s_.write(0x128, 0))
          return fail();
      }
      auto ep0 = s_.read(0x128);
      if (!ep0)
        return fail();
      if (*ep0) {
        auto i = index(0, (setup_.request_type & 128) != 0);
        auto &v = slots_[i];
        if (v.state != stage::free) {
          if (is_in(i))
            v.ack = true;
          else
            v.data_ready = true;
        }
        if (!s_.write(0x128, 0))
          return fail();
      }
      auto data = s_.read(0x160);
      if (!data)
        return fail();
      if (*data) {
        auto status = s_.read(0x46C);
        if (!status)
          return fail();
        data_pending_ |= *status;
        for (unsigned ep = 1; ep < 8; ++ep) {
          if (data_pending_ & (1U << ep))
            slots_[index(ep, true)].ack = true;
          if (data_pending_ & (1U << (16 + ep)))
            slots_[index(ep, false)].data_ready = true;
        }
        if (!s_.write(0x46C, *status) || !s_.write(0x160, 0))
          return fail();
        data_pending_ = 0;
      }
      auto sof = s_.read(0x154);
      if (!sof)
        return fail();
      if (*sof) {
        auto frame = s_.read(0x520);
        if (!frame)
          return fail();
        frame_ = *frame & 2047;
        events_.sof = true;
        events_.frame = frame_;
        if (current_ && endpoint_number(*current_) == 8 &&
            slots_[*current_].frame != frame_)
          slots_[*current_].overrun = true;
        for (bool in : std::array{true, false}) {
          auto &v = slots_[index(8, in)];
          if (v.state == stage::queued) {
            if (v.data_ready && v.frame != frame_)
              v.overrun = true;
            v.frame = frame_;
            v.data_ready = in;
            if (!in) {
              auto size = s_.read(0x4C0);
              if (!size)
                return fail();
              v.data_ready = (*size & 0x3FF) || (*size & 65536);
            }
          }
        }
        if (!s_.write(0x154, 0))
          return fail();
      }
      for (unsigned i = 0; i < 18; ++i)
        if (slots_[i].state == stage::wire &&
            (slots_[i].ack || slots_[i].cancel))
          if (!publish(i))
            return fail();
      if (!current_ && !reset_pending_ && !suspend_pending_ && !resume_pending_)
        return schedule();
      return {};
    });
  }
  [[nodiscard]] system_result
  enter_suspend(bool release_clock = true) noexcept {
    if (!awake() || !suspend_pending_ || current_ ||
        s_.context.execution.in_isr())
      return std::unexpected(system_error::busy);
    if (!s_.write(0x52C, 1))
      return fail();
    suspended_ = true;
    suspend_pending_ = false;
    suspended_at_ = s_.context.execution.now();
    // Retain active-work ownership to exclude System OFF while attached. System
    // ON idle remains available with a caller-supplied wake-safe predicate.
    if (release_clock)
      return s_.release_high_frequency();
    return {};
  }
  [[nodiscard]] system_result resume() noexcept {
    if (!initialized_ || !suspended_ || s_.context.execution.in_isr())
      return std::unexpected(system_error::busy);
    auto clock = s_.require_high_frequency();
    if (!clock)
      return clock;
    if (!errata_power(true, false) || !s_.write(0x52C, 0))
      return fail();
    if (!s_.bounded(
            [&] {
              auto r = s_.read(0x400);
              return r && (*r & 1024);
            },
            timeout_, PollBudget))
      return std::unexpected(system_error::timeout);
    if (!s_.write(0x400, 1024) || !errata_power(false, false))
      return fail();
    suspended_ = resume_pending_ = suspend_pending_ = false;
    return {};
  }
  void remote_wakeup_enabled(bool enabled) noexcept {
    remote_enabled_ = enabled;
  }
  [[nodiscard]] system_result remote_wakeup() noexcept {
    if (!remote_enabled_ || !suspended_ ||
        static_cast<execution::tick>(s_.context.execution.now() -
                                     suspended_at_) <
            std::uint64_t(config_.ticks_per_second) * 5 / 1000)
      return std::unexpected(system_error::busy);
    auto r = resume();
    if (!r)
      return r;
    if (!s_.write(0x508, 1) || !s_.write(0x58, 1))
      return fail();
    return {}; // hardware-timed resume K state
  }
  [[nodiscard]] bool suspended() const noexcept { return suspended_; }
  [[nodiscard]] bool resume_pending() const noexcept { return resume_pending_; }
  [[nodiscard]] bool suspend_pending() const noexcept {
    return suspend_pending_;
  }
  [[nodiscard]] bool active_dma() const noexcept {
    return current_.has_value();
  }
  [[nodiscard]] system_result interrupt_delivery(bool on) noexcept {
    if (!initialized_)
      return std::unexpected(system_error::not_initialized);
    return s_.write(on ? 0x304 : 0x308, irq_mask) ? system_result{} : fail();
  }
  [[nodiscard]] system_result cancel_all() noexcept {
    if (s_.context.execution.in_isr())
      return std::unexpected(system_error::invalid);
    if (!s_.owned())
      return {};
    auto enabled = s_.read(0x500);
    if (!enabled)
      return fail();
    if (*enabled || !disable_requested_) {
      auto r = request_disable();
      if (!r)
        return r;
    }
    if (!s_.bounded(
            [&] {
              auto r = s_.read(0x500);
              return r && !*r;
            },
            timeout_, PollBudget))
      return std::unexpected(system_error::timeout);
    return finish_disable();
  }
  [[nodiscard]] system_result shutdown() noexcept {
    if (!s_.owned())
      return {};
    auto r = cancel_all();
    if (!r)
      return r;
    if (!s_.write(0x308, irq_mask) || !errata_power(false, true))
      return fail();
    r = s_.release();
    if (r)
      fault_ = false;
    return r;
  }
  [[nodiscard]] system_result
  request_bootloader(bootloader_handoff handoff) noexcept {
    if (!handoff.reviewed_bootloader_abi || !handoff.application_quiescent ||
        handoff.begin % 4 || handoff.begin >= handoff.end ||
        handoff.end > 0x100000 || s_.context.execution.in_isr())
      return std::unexpected(system_error::invalid);
    auto stack = s_.context.read(handoff.begin),
         reset = s_.context.read(handoff.begin + 4);
    if (!stack || !reset)
      return fail();
    if ((*stack & 7) || *stack <= 0x20000000 || *stack > 0x20040000 ||
        !(*reset & 1) || (*reset & ~1U) < handoff.begin ||
        (*reset & ~1U) >= handoff.end)
      return std::unexpected(system_error::invalid);
    auto off = shutdown();
    if (!off)
      return off;
    if (s_.context.system_off_blocked() || s_.clocks.has_clients())
      return std::unexpected(system_error::busy);
    auto aircr = s_.context.read(0xE000ED0C);
    if (!aircr)
      return fail();
    if (!s_.context.write(0x4000051C, handoff.gpregret))
      return fail();
    arch::data_synchronization_barrier();
    if (!s_.context.write(0xE000ED0C, 0x05FA0004 | (*aircr & 0x700)))
      return fail();
    arch::data_synchronization_barrier();
    return {};
  }

  [[nodiscard]] bool control_quiescent() const noexcept {
    for (auto i : std::array{0U, 9U})
      if (slots_[i].state == stage::dma || slots_[i].state == stage::wire ||
          slots_[i].state == stage::queued)
        return false;
    return true;
  }
  [[nodiscard]] unsigned ticks_per_second() const noexcept {
    return config_.ticks_per_second;
  }
  [[nodiscard]] execution::tick now() const noexcept {
    return s_.context.execution.now();
  }
  [[nodiscard]] system_result
  deconfigure_endpoints(unsigned mask = 0x1FE) noexcept {
    if (!awake() || mask & ~0x1FEU)
      return std::unexpected(system_error::invalid);
    bool pending = false;
    for (unsigned i = 0; i < 18; ++i)
      if (mask & (1U << endpoint_number(i))) {
        auto &v = slots_[i];
        if (v.state != stage::free && v.state != stage::complete) {
          v.cancel = true;
          if (current_ && *current_ == i)
            pending = true;
          else if (!publish(i))
            return fail();
        }
      }
    if (pending)
      return std::unexpected(system_error::busy);
    auto in = s_.read(0x510), out = s_.read(0x514);
    if (!in || !out || !s_.write(0x510, *in & ~mask) ||
        !s_.write(0x514, *out & ~mask))
      return fail();
    for (unsigned i = 0; i < 18; ++i)
      if (mask & (1U << endpoint_number(i)))
        endpoints_[i].enabled = false;
    return {};
  }

private:
  static constexpr unsigned index(unsigned ep, bool in) {
    return ep + (in ? 0 : 9);
  }
  static constexpr unsigned endpoint_number(unsigned i) { return i % 9; }
  static constexpr bool is_in(unsigned i) { return i < 9; }
  static unsigned registers(unsigned i) {
    return (is_in(i) ? 0x600 : 0x700) + 20 * endpoint_number(i);
  }
  static unsigned task(unsigned i) {
    return (is_in(i) ? 4 : 0x28) + 4 * endpoint_number(i);
  }
  static unsigned end_event(unsigned i) {
    return is_in(i)
               ? (endpoint_number(i) == 8 ? 0x12C
                                          : 0x108 + 4 * endpoint_number(i))
               : (endpoint_number(i) == 8 ? 0x150
                                          : 0x130 + 4 * endpoint_number(i));
  }
  bool awake() const {
    return initialized_ && !suspended_ && !disable_requested_ && !fault_;
  }
  system_result enqueue(unsigned ep, bool in, const void *data,
                        std::size_t capacity, std::size_t count) {
    return s_.context.execution.critical([&]() -> system_result {
      if (!awake() || ep > 8)
        return std::unexpected(system_error::busy);
      auto i = index(ep, in);
      auto &endpoint = endpoints_[i];
      auto &v = slots_[i];
      if (!endpoint.enabled || v.state != stage::free ||
          count > endpoint.maximum)
        return std::unexpected(system_error::busy);
      auto bytes = (std::max(count, std::size_t{1}) + 3) & ~std::size_t{3};
      if (!count) {
        data = zero_[i].data();
        capacity = 4;
      }
      if (capacity < bytes)
        return std::unexpected(system_error::invalid);
      auto lease = s_.prepare(data, capacity, bytes, {1024, 1, 4},
                              in ? dma_access::read : dma_access::write);
      if (!lease)
        return std::unexpected(system_error::invalid);
      v = {};
      v.state = stage::queued;
      v.lease = *lease;
      v.data = data;
      v.count = count;
      v.data_ready = in && ep != 8;
      v.generation = setup_.generation;
      if (!in && ep != 8) {
        if (!s_.write(ep == 0 ? 0x4C : 0x4A0 + 4 * ep, ep == 0 ? 1 : 0))
          return fail();
      }
      return {};
    });
  }
  system_result schedule() {
    // EP0 takes priority, then a rotating scan prevents bulk starvation.
    std::array<unsigned, 18> order{};
    order[0] = 0;
    order[1] = 9;
    unsigned at = 2;
    for (unsigned n = 0; n < 18; ++n) {
      auto i = (round_robin_ + n) % 18;
      if (i != 0 && i != 9)
        order[at++] = i;
    }
    for (auto i : order) {
      auto &v = slots_[i];
      if (v.state != stage::queued || v.cancel || !v.data_ready)
        continue;
      auto ep = endpoint_number(i);
      auto count = v.count;
      if (!is_in(i)) {
        auto size = s_.read(ep == 8 ? 0x4C0 : 0x4A0 + 4 * ep);
        if (!size)
          return fail();
        auto incoming = *size & (ep == 8 ? 1023U : 127U);
        if (incoming > count)
          v.overrun = true;
        count = std::min(count, std::size_t(incoming));
      }
      if (!s_.write(end_event(i), 0) ||
          !s_.write(registers(i),
                    s_.address(v.data, std::max(v.count, std::size_t{1}))) ||
          !s_.write(registers(i) + 4, count))
        return fail();
      if (!s_.buffers.arm(*v.lease))
        return fail();
      v.state = stage::dma;
      current_ = i;
      round_robin_ = (i + 1) % 18;
      if (!s_.write(0xC1C, 0x82) || !s_.write(task(i), 1))
        return fail();
      return {};
    }
    return {};
  }
  bool publish(unsigned i) {
    auto &v = slots_[i];
    if (v.state == stage::queued) {
      s_.discard(v.lease);
      if (v.lease)
        return false;
    } else if (!s_.release_quiet(v.lease))
      return false;
    v.state = stage::complete;
    return true;
  }
  system_result request_disable() {
    disable_requested_ = true;
    for (auto &v : slots_)
      if (v.state != stage::free && v.state != stage::complete)
        v.cancel = true;
    bool disconnected = suspended_ || !initialized_ || s_.write(0x504, 0);
    // Still request full disable after a failed pull-up write; a later explicit
    // cancel retries both writes and preserves leases until readback is zero.
    bool disabled = s_.write(0x500, 0);
    if (!disconnected || !disabled)
      return fail();
    return {};
  }
  system_result finish_disable() {
    // ENABLE reads Enabled until the entire peripheral is disabled (PS 6.35.4).
    // This concrete readback, not the write request, is the cancellation proof.
    for (unsigned i = 0; i < 18; ++i) {
      auto &v = slots_[i];
      if (v.state != stage::free && v.state != stage::complete) {
        v.cancel = true;
        if (!publish(i))
          return fail();
      }
    }
    if (!s_.write(0xC1C, 0))
      return fail();
    current_.reset();
    initialized_ = connected_ = suspended_ = resume_pending_ =
        suspend_pending_ = false;
    return {};
  }
  bool errata_power(bool begin, bool enable) {
    if (!s_.context.write(0x4006EC00, 0x9375) ||
        !s_.context.write(0x4006EC14, begin ? 0xC0 : 0))
      return false;
    if (enable && (!s_.context.write(0x4006EC00, 0x9375) ||
                   !s_.context.write(0x4006ED14, begin ? 3 : 0)))
      return false;
    return s_.context.write(0x4006EC00, 0x9375).has_value();
  }
  system_result fail() {
    fault_ = true;
    return std::unexpected(system_error::mmio);
  }
  static constexpr std::uintptr_t base = 0x40027000;
  static constexpr std::uint32_t irq_mask = 0x1FFFFFF;
  session s_;
  usb_configuration config_{};
  execution::tick timeout_, suspended_at_{};
  std::array<slot, 18> slots_{};
  std::array<endpoint, 18> endpoints_{};
  alignas(4) std::array<std::array<std::uint8_t, 4>, 18> zero_{};
  std::optional<unsigned> current_;
  unsigned round_robin_{}, frame_{};
  std::uint32_t data_pending_{}, cause_pending_{};
  usb_setup setup_{};
  usb_events events_{};
  bool initialized_{}, fault_{}, connected_{}, disable_requested_{},
      reset_pending_{}, setup_pending_{}, suspend_pending_{}, resume_pending_{},
      suspended_{}, remote_enabled_{};
};
} // namespace mcubsp::backend::nrf52840
