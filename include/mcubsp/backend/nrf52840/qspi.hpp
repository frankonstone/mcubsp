#pragma once
// @req-file REQ-0066 REQ-0062
// PS v1.11 6.19; Nordic nRF52840 anomalies 122,208,215,216,244.
#include <mcubsp/backend/nrf52840/detail/peripheral_session.hpp>
#include <mcubsp/mcu/nrf52840/product/qspi.hpp>
#include <mcubsp/mcu/nrf52840/serial_revision.hpp>
namespace mcubsp::backend::nrf52840 {
template <core::pin_id Sck, core::pin_id Cs, core::pin_id Io0, core::pin_id Io1,
          core::pin_id Io2 = detail::disconnected,
          core::pin_id Io3 = detail::disconnected>
struct qspi_binding {
  static constexpr std::array pins{Sck, Cs, Io0, Io1, Io2, Io3};
  static_assert(
      [] {
        for (unsigned i = 0; i < pins.size(); ++i) {
          if (pins[i] == detail::disconnected) {
            if (i < 4)
              return false;
            continue;
          }
          if (!mcu::nrf52840::gpio_supported(pins[i]) ||
              mcu::nrf52840::product::silicon_low_frequency_only(pins[i]))
            return false;
          for (unsigned j = 0; j < i; ++j)
            if (pins[j] == pins[i])
              return false;
        }
        return true;
      }(),
      "QSPI requires distinct, high-frequency-capable GPIO pins");
};
enum class qspi_read_protocol : unsigned {
  fast,
  dual_output,
  dual_io,
  quad_output,
  quad_io
};
enum class qspi_write_protocol : unsigned {
  single,
  dual_output,
  quad_output,
  quad_io
};
struct qspi_device {
  std::uint64_t capacity_bytes{};
  unsigned page_bytes{256};
  std::uint32_t maximum_clock_hz{};
  qspi_read_protocol read{qspi_read_protocol::fast};
  qspi_write_protocol write{qspi_write_protocol::single};
  bool address_32bit{}, quad_enabled{}, supports_4k_erase{true},
      supports_64k_erase{true};
  std::uint32_t
      address_configuration{}; // ADDRCONF, attached flash's EN4B command.
  std::uint8_t status_opcode{5}, busy_mask{1}, error_mask{};
  bool deep_power_down{};
  std::uint16_t dpm_enter{}, dpm_exit{}; // 16 MHz ticks, per attached device.
  execution::tick
      operation_timeout{}; // device's worst-case erase/program interval.
};
struct qspi_configuration {
  unsigned frequency_hz{8'000'000};
  std::uint8_t sck_delay{128}, rx_delay{};
  bool mode3{};
};
struct qspi_instruction {
  std::uint8_t opcode{};
  std::array<std::uint8_t, 8> data{};
  unsigned size{}; // bytes after opcode, 0..8
  bool io2_high{true}, io3_high{true}, write_enable{}, wait_busy{};
  bool long_frame{}, last_frame{};
};
struct qspi_completion {
  std::size_t bytes{};
  std::array<std::uint8_t, 8> response{};
  std::uint8_t device_status{};
};
struct qspi_xip_contract {
  // Required for anomaly 216: pointer-bearing read-only data must be placed
  // in internal flash/RAM. Direct external data accesses must avoid dependent
  // consecutive LDRs; copy_xip_words() provides a bounded safe copy path.
  bool reviewed_linker_and_read_accesses{};
  bool executable{};
};
struct qspi_mapping {
  const void *owner{};
  unsigned slot{};
  std::uint32_t generation{};
  std::uintptr_t address{};
  std::size_t bytes{};
};
// Polling API: a failed start/timeout may retain a DMA buffer. The caller must
// keep it alive until poll() returns a completion or cancel()/shutdown
// succeeds. QSPI has no DMA STOP task; cancellation drains the actual
// operation.
template <class Binding, class Context, class Buffers,
          class Address = native_dma_address, unsigned PollBudget = 1000000>
class qspi_controller {
  using session = detail::peripheral_session<Context, Buffers, Address>;
  using region = mcu::nrf52840::memory::region;
  enum class phase { idle, dma, custom, status, status_reply };

public:
  qspi_controller(Context &c, clock_controller<Context> &clocks,
                  Buffers &buffers, execution::tick control_ticks = 1'280'000,
                  Address a = {}) noexcept
      : s_(c, clocks, buffers, a), control_ticks_(control_ticks) {}
  qspi_controller(const qspi_controller &) = delete;
  qspi_controller &operator=(const qspi_controller &) = delete;
  ~qspi_controller() {
    if (dma_ || mapping_count_)
      s_.context.execution.active_transfer_destroyed();
  }
  [[nodiscard]] system_result
  initialize(qspi_device device, qspi_configuration config, region writable,
             std::span<const region> protected_ranges = {}) noexcept {
    if (!valid(device, config, writable, protected_ranges) ||
        !valid_interval(control_ticks_))
      return std::unexpected(system_error::invalid);
    auto p = s_.context.read(0x10000130), r = s_.context.read(0x10000134);
    if (!p || !r)
      return std::unexpected(system_error::mmio);
    if (!mcu::nrf52840::serial_revision{*p, *r}.reviewed_modern())
      return std::unexpected(system_error::unsupported);
    auto claim = s_.acquire(peripheral::qspi, base, Binding::pins);
    if (!claim)
      return claim;
    d_ = device;
    config_ = config;
    writable_ = writable;
    for (unsigned i = 0; i < protected_ranges.size(); ++i)
      protected_[i] = protected_ranges[i];
    protected_count_ = protected_ranges.size();
    s_.publish();
    for (unsigned i = 0; i < 6; ++i)
      if (Binding::pins[i] != detail::disconnected) {
        if (!s_.context.gpio.configure(
                Binding::pins[i],
                {.direction = i == 3 ? driver::gpio_direction::input
                                     : driver::gpio_direction::output,
                 .initial_level = (i == 1 || i >= 4) ? driver::gpio_level::high
                                                     : driver::gpio_level::low,
                 .drive = driver::gpio_drive::middle}) ||
            !s_.write(pin_offsets[i], session::select(Binding::pins[i])))
          return failure();
      } else if (!s_.write(pin_offsets[i], 0x80000000U))
        return failure();
    unsigned if0 =
        static_cast<unsigned>(d_.read) |
        (static_cast<unsigned>(d_.write) << 3) | (d_.address_32bit ? 64U : 0U) |
        (d_.deep_power_down ? 128U : 0U) | (d_.page_bytes == 512 ? 4096U : 0U);
    if (!s_.write(0x308, 1) || !s_.write(0x100, 0) || !s_.write(0x544, if0) ||
        !s_.write(0x600, ifconfig1()) ||
        !s_.write(0x614, d_.dpm_enter | (unsigned(d_.dpm_exit) << 16)) ||
        !s_.write(0x624, d_.address_configuration) ||
        !s_.write(0x640, unsigned(config.rx_delay) << 8) || !s_.write(0x500, 1))
      return failure();
    enabled_ = true;
    auto activated = activate();
    if (!activated)
      return activated;
    initialized_ = true;
    return {};
  }
  [[nodiscard]] system_result activate() noexcept {
    if (!s_.owned() || busy() || mapping_count_ || long_frame_)
      return std::unexpected(system_error::busy);
    if (!s_.write(0x500, 1) || !s_.write(0x100, 0))
      return failure();
    control_pending_ = true;
    if (!s_.write(0, 1))
      return failure();
    if (!wait_ready())
      return std::unexpected(system_error::timeout);
    initialized_ = enabled_ = active_ = true;
    after_xip_ = false;
    sleeping_ = false;
    return {};
  }
  [[nodiscard]] system_result read(std::uint32_t offset,
                                   std::span<std::uint32_t> output) noexcept {
    return start_dma(offset, output.data(), output.size_bytes(), false);
  }
  [[nodiscard]] system_result
  program(std::uint32_t offset, std::span<const std::uint32_t> input) noexcept {
    return start_dma(offset, input.data(), input.size_bytes(), true);
  }
  [[nodiscard]] system_result erase(std::uint32_t offset,
                                    unsigned bytes) noexcept {
    if (!available())
      return std::unexpected(system_error::busy);
    unsigned word = bytes == 4096 ? 0 : bytes == 65536 ? 1 : 2;
    if (!bytes || offset % bytes || !mutable_range(offset, bytes) ||
        (word == 0 && !d_.supports_4k_erase) ||
        (word == 1 && !d_.supports_64k_erase) ||
        (word == 2 && (offset || bytes != d_.capacity_bytes)))
      return std::unexpected(system_error::invalid);
    if (!s_.write(0x51C, offset) || !s_.write(0x520, word) ||
        !s_.write(0x100, 0))
      return failure();
    begin(phase::dma, 0, true);
    if (!s_.write(0xC, 1))
      return failure();
    return {};
  }
  [[nodiscard]] system_result custom(qspi_instruction instruction) noexcept {
    if (!initialized_ || !active_ || busy() || mapping_count_ || sleeping_ ||
        fault_)
      return std::unexpected(system_error::busy);
    if (instruction.size > 8 ||
        (!instruction.long_frame && (long_frame_ || instruction.last_frame)))
      return std::unexpected(system_error::invalid);
    // Commands may mutate any device state; exclude all XIP and use an explicit
    // attached-device command, never an implicit status-register/quad write.
    auto safe = register_access();
    if (!safe)
      return safe;
    long_frame_ = instruction.long_frame;
    begin(phase::custom, instruction.size, false);
    last_frame_ = instruction.last_frame;
    if (!send_instruction(instruction))
      return failure();
    return {};
  }
  [[nodiscard]] system_value<std::optional<qspi_completion>> poll() noexcept {
    return s_.context.execution.critical(
        [&]() -> system_value<std::optional<qspi_completion>> {
          if (control_pending_)
            return std::unexpected(system_error::busy);
          if (phase_ == phase::idle)
            return std::optional<qspi_completion>{};
          auto event = s_.read(0x100);
          if (!event)
            return std::unexpected(system_error::mmio);
          if (!*event && !terminal_seen_) {
            if (execution::reached(s_.context.execution.now(), until_))
              return std::unexpected(system_error::timeout);
            return std::optional<qspi_completion>{};
          }
          terminal_seen_ = true;
          if (!s_.write(0x100, 0))
            return std::unexpected(system_error::mmio);
          if (phase_ == phase::dma) {
            if (!s_.release_quiet(dma_))
              return failure_value();
            if (mutation_) {
              phase_ = phase::status;
              terminal_seen_ = true;
            } else
              return finish();
          }
          if (phase_ == phase::custom || phase_ == phase::status_reply) {
            auto lo = s_.read(0x638), hi = s_.read(0x63C);
            if (!lo || !hi)
              return std::unexpected(system_error::mmio);
            if (phase_ == phase::custom) {
              for (unsigned i = 0; i < 8; ++i)
                completion_.response[i] =
                    ((i < 4 ? *lo : *hi) >> ((i % 4) * 8)) & 255;
              if (last_frame_)
                long_frame_ = false;
              return finish();
            }
            completion_.device_status = *lo & 255;
            if (!(*lo & d_.busy_mask)) {
              bool error = (*lo & d_.error_mask) != 0;
              auto completed = finish();
              if (error)
                return std::unexpected(system_error::faulted);
              return completed;
            }
            phase_ = phase::status;
          }
          if (phase_ == phase::status) {
            if (execution::reached(s_.context.execution.now(), until_))
              return std::unexpected(system_error::timeout);
            if (!send_instruction({.opcode = d_.status_opcode, .size = 1}))
              return failure_value();
            phase_ = phase::status_reply;
            terminal_seen_ = false;
          }
          return std::optional<qspi_completion>{};
        });
  }
  [[nodiscard]] system_result cancel() noexcept {
    if (s_.context.execution.in_isr())
      return std::unexpected(system_error::invalid);
    if (control_pending_) {
      if (!wait_ready())
        return std::unexpected(system_error::timeout);
      active_ = true;
      after_xip_ = false;
    }
    if (dpm_pending_) {
      if (!s_.bounded(
              [&] {
                auto r = s_.read(0x604);
                return r && !(*r & 8) && bool(*r & 4) == dpm_target_;
              },
              control_ticks_, PollBudget))
        return std::unexpected(system_error::timeout);
      sleeping_ = dpm_target_;
      dpm_pending_ = false;
    }
    until_ = {static_cast<execution::tick>(s_.context.execution.now() +
                                           control_ticks_)};
    // A deadline is a report, not permission to free a still-active buffer.
    if (!s_.bounded(
            [&] {
              if (phase_ == phase::idle)
                return true;
              auto p = poll();
              return p && phase_ == phase::idle;
            },
            control_ticks_, PollBudget))
      return std::unexpected(system_error::timeout);
    if (long_frame_) {
      // Commit ownership before LFSTOP; retry keeps the frame until READY.
      begin(phase::custom, 0, false);
      last_frame_ = true;
      if (!send_instruction({.long_frame = true, .last_frame = true}))
        return failure();
      if (!s_.bounded(
              [&] {
                auto r = poll();
                return r && phase_ == phase::idle;
              },
              control_ticks_, PollBudget))
        return std::unexpected(system_error::timeout);
    }
    fault_ = false;
    return {};
  }
  [[nodiscard]] bool busy() const noexcept {
    return phase_ != phase::idle || control_pending_ || dpm_pending_;
  }
  [[nodiscard]] bool owns_resource() const noexcept { return s_.owned(); }
  [[nodiscard]] system_result deep_power_down(bool enter) noexcept {
    if (!initialized_ || !active_ || busy() || mapping_count_ || long_frame_ ||
        fault_ || !d_.deep_power_down)
      return std::unexpected(system_error::busy);
    auto safe = register_access();
    if (!safe)
      return safe;
    dpm_pending_ = true;
    dpm_target_ = enter;
    if (!s_.write(0x600, ifconfig1() | (enter ? 1U << 24 : 0U)))
      return failure();
    // STATUS is valid only after DPMDUR timing. Bound the device's ready and
    // DPM observations; retain ownership if it never reaches the requested
    // state.
    if (!s_.bounded(
            [&] {
              auto r = s_.read(0x604);
              return r && !(*r & 8) && bool(*r & 4) == enter;
            },
            control_ticks_, PollBudget))
      return std::unexpected(system_error::timeout);
    sleeping_ = enter;
    dpm_pending_ = false;
    return {};
  }
  [[nodiscard]] system_value<qspi_mapping>
  borrow_xip(std::uint32_t flash_offset, std::size_t bytes,
             qspi_xip_contract contract) noexcept {
    if (!initialized_ || !active_ || busy() || long_frame_ || sleeping_ ||
        fault_)
      return std::unexpected(system_error::busy);
    if (!contract.reviewed_linker_and_read_accesses || flash_offset % 4 ||
        !bytes || bytes > 0x8000000 || !range(flash_offset, bytes))
      return std::unexpected(system_error::invalid);
    if (mapping_count_ && flash_offset != xip_offset_)
      return std::unexpected(system_error::busy);
    unsigned slot = 8;
    for (unsigned i = 0; i < 8; ++i)
      if (!mappings_[i].live && mappings_[i].generation != 0xFFFFFFFFU) {
        slot = i;
        break;
      }
    if (slot == 8)
      return std::unexpected(system_error::busy);
    if (!mapping_count_ && !s_.write(0x540, flash_offset))
      return std::unexpected(system_error::mmio);
    if (!s_.context.resources.retain(s_.context.execution, *s_.owner()))
      return std::unexpected(system_error::busy);
    auto &m = mappings_[slot];
    ++m.generation;
    m.live = true;
    m.executable = contract.executable;
    m.bytes = bytes;
    ++mapping_count_;
    xip_offset_ = flash_offset;
    after_xip_ = true;
    arch::data_synchronization_barrier();
    arch::instruction_barrier();
    return qspi_mapping{this, slot, m.generation, 0x12000000, bytes};
  }
  [[nodiscard]] system_result
  return_xip(qspi_mapping mapping,
             bool execution_and_interrupts_relocated = false) noexcept {
    if (!valid_mapping(mapping) || s_.context.execution.in_isr())
      return std::unexpected(system_error::invalid);
    auto &m = mappings_[mapping.slot];
    if (m.executable && !execution_and_interrupts_relocated)
      return std::unexpected(system_error::busy);
    arch::data_synchronization_barrier();
    arch::instruction_barrier();
    if (!s_.context.resources.drop(s_.context.execution, *s_.owner()))
      return std::unexpected(system_error::busy);
    m.live = false;
    --mapping_count_;
    return {};
  }
  [[nodiscard]] system_result
  copy_xip_words(qspi_mapping mapping, std::size_t offset,
                 std::span<std::uint32_t> output) noexcept {
    if (!valid_mapping(mapping) || offset % 4 || offset > mapping.bytes ||
        output.size_bytes() > mapping.bytes - offset)
      return std::unexpected(system_error::invalid);
    for (std::size_t i = 0; i < output.size(); ++i) {
      auto v = s_.context.read(mapping.address + offset + 4 * i);
      if (!v)
        return std::unexpected(system_error::mmio);
      output[i] = *v;
      arch::data_memory_barrier(); // separate dependent external loads
    }
    return {};
  }
  [[nodiscard]] system_result deactivate() noexcept {
    if (!s_.owned())
      return {};
    if (mapping_count_)
      return std::unexpected(system_error::busy);
    auto drained = cancel();
    if (!drained)
      return drained;
    // 208: CPU task only; 122: undocumented shutdown latch before ENABLE=0.
    if (!s_.write(0x10, 1) || !s_.write(0x54, 1) || !s_.disabled())
      return failure();
    active_ = initialized_ = enabled_ = false;
    return {};
  }
  [[nodiscard]] system_result shutdown() noexcept {
    if (s_.context.execution.in_isr())
      return std::unexpected(system_error::invalid);
    auto off = deactivate();
    if (!off)
      return off;
    if (!s_.owned())
      return {};
    for (auto off : pin_offsets)
      if (!s_.write(off, 0x80000000U))
        return failure();
    if (!s_.write(0x308, 1) || !s_.write(0x100, 0))
      return failure();
    auto r = s_.release();
    if (r)
      fault_ = false;
    return r;
  }

private:
  static bool valid_interval(execution::tick n) {
    return n && n <= execution::maximum_deadline_interval;
  }
  static bool valid(qspi_device d, qspi_configuration c, region writable,
                    std::span<const region> protected_ranges) {
    if (!d.capacity_bytes || d.capacity_bytes > 0x100000000ULL ||
        (!d.address_32bit && d.capacity_bytes > 0x1000000) ||
        (d.page_bytes != 256 && d.page_bytes != 512) || !d.maximum_clock_hz ||
        c.frequency_hz > d.maximum_clock_hz || c.frequency_hz < 2'000'000 ||
        c.frequency_hz > 32'000'000 || 32'000'000 % c.frequency_hz ||
        !valid_interval(d.operation_timeout) ||
        static_cast<unsigned>(d.read) > 4 ||
        static_cast<unsigned>(d.write) > 3 || c.rx_delay > 7 || !d.busy_mask ||
        (d.error_mask & d.busy_mask) || protected_ranges.size() > 8 ||
        d.address_configuration &
            ~mcu::nrf52840::product::qspi::addrconf::writable_mask ||
        writable.begin > d.capacity_bytes ||
        writable.bytes > d.capacity_bytes - writable.begin)
      return false;
    if ((static_cast<unsigned>(d.read) >= 3 ||
         static_cast<unsigned>(d.write) >= 2) &&
        (!d.quad_enabled || Binding::pins[4] == detail::disconnected ||
         Binding::pins[5] == detail::disconnected))
      return false;
    for (auto p : protected_ranges)
      if (!p.bytes || p.begin > d.capacity_bytes ||
          p.bytes > d.capacity_bytes - p.begin)
        return false;
    return true;
  }
  bool range(std::uint32_t off, std::size_t bytes) const {
    return off <= d_.capacity_bytes && bytes <= d_.capacity_bytes - off;
  }
  bool mutable_range(std::uint32_t off, std::size_t bytes) const {
    if (!range(off, bytes) || !writable_.contains(off, bytes))
      return false;
    for (unsigned i = 0; i < protected_count_; ++i)
      if (off < protected_[i].begin + protected_[i].bytes &&
          protected_[i].begin < off + bytes)
        return false;
    return true;
  }
  bool available() const {
    return initialized_ && active_ && !busy() && !mapping_count_ &&
           !long_frame_ && !sleeping_ && !fault_ &&
           !s_.context.execution.in_isr();
  }
  std::uint32_t ifconfig1() const {
    return ((32'000'000 / config_.frequency_hz - 1) << 28) |
           (config_.mode3 ? 1U << 25 : 0U) | config_.sck_delay;
  }
  bool wait_ready() {
    if (!s_.bounded(
            [&] {
              auto r = s_.read(0x100);
              if (r && *r)
                terminal_seen_ = true;
              return terminal_seen_;
            },
            control_ticks_, PollBudget))
      return false;
    if (!s_.write(0x100, 0))
      return false;
    control_pending_ = terminal_seen_ = false;
    return true;
  }
  system_result register_access() {
    if (!after_xip_)
      return {};
    if (mapping_count_)
      return std::unexpected(system_error::busy);
    // 215: ACTIVATE+READY before any register offset above 0x600.
    return activate();
  }
  void begin(phase p, std::size_t bytes, bool mutation) {
    phase_ = p;
    completion_ = {};
    completion_.bytes = bytes;
    mutation_ = mutation;
    terminal_seen_ = false;
    until_ = {static_cast<execution::tick>(s_.context.execution.now() +
                                           d_.operation_timeout)};
  }
  system_result start_dma(std::uint32_t off, const void *data,
                          std::size_t bytes, bool write) {
    if (!available())
      return std::unexpected(system_error::busy);
    if (!bytes || bytes > 0x3FFFC || off % 4 || !range(off, bytes) ||
        (write && !mutable_range(off, bytes)))
      return std::unexpected(system_error::invalid);
    auto safe = register_access();
    if (!safe)
      return safe;
    auto lease = s_.prepare(data, bytes, bytes / 4, {0xFFFF, 4, 4},
                            write ? dma_access::read : dma_access::write);
    if (!lease)
      return std::unexpected(system_error::invalid);
    dma_ = *lease;
    auto reg = write ? 0x510U : 0x504U;
    if (!s_.write(reg, off) || !s_.write(reg + 4, s_.address(data, bytes)) ||
        !s_.write(reg + 8, bytes) || !s_.write(0x100, 0)) {
      s_.discard(dma_);
      return failure();
    }
    if (!s_.buffers.arm(*dma_)) {
      s_.discard(dma_);
      return failure();
    }
    begin(phase::dma, bytes, write);
    if (!s_.write(write ? 8 : 4, 1))
      return failure();
    return {};
  }
  bool send_instruction(qspi_instruction i) {
    std::uint32_t lo = 0, hi = 0;
    for (unsigned n = 0; n < i.size; ++n)
      (n < 4 ? lo : hi) |= std::uint32_t(i.data[n]) << (8 * (n % 4));
    auto word = i.opcode | ((i.size + 1) << 8) | (i.io2_high ? 1U << 12 : 0U) |
                (i.io3_high ? 1U << 13 : 0U) | (i.wait_busy ? 1U << 14 : 0U) |
                (i.write_enable ? 1U << 15 : 0U) |
                (i.long_frame ? 1U << 16 : 0U) | (i.last_frame ? 1U << 17 : 0U);
    return s_.write(0x100, 0) && s_.write(0x638, lo) && s_.write(0x63C, hi) &&
           s_.write(0x634, word);
  }
  system_value<std::optional<qspi_completion>> finish() {
    phase_ = phase::idle;
    mutation_ = terminal_seen_ = false;
    return std::optional{completion_};
  }
  system_result failure() {
    fault_ = true;
    return std::unexpected(system_error::mmio);
  }
  system_value<std::optional<qspi_completion>> failure_value() {
    fault_ = true;
    return std::unexpected(system_error::mmio);
  }
  bool valid_mapping(qspi_mapping m) const {
    return m.owner == this && m.slot < 8 && mappings_[m.slot].live &&
           mappings_[m.slot].generation == m.generation &&
           m.address == 0x12000000 && m.bytes == mappings_[m.slot].bytes;
  }
  static constexpr std::uintptr_t base = 0x40029000;
  static constexpr std::array<unsigned, 6> pin_offsets{0x524, 0x528, 0x530,
                                                       0x534, 0x538, 0x53C};
  session s_;
  qspi_device d_{};
  qspi_configuration config_{};
  region writable_{};
  std::array<region, 8> protected_{};
  unsigned protected_count_{};
  struct mapping_state {
    std::uint32_t generation{};
    std::size_t bytes{};
    bool live{}, executable{};
  };
  std::array<mapping_state, 8> mappings_{};
  unsigned mapping_count_{};
  std::uint32_t xip_offset_{};
  std::optional<dma_buffer_token> dma_;
  phase phase_{phase::idle};
  qspi_completion completion_{};
  execution::deadline until_{};
  execution::tick control_ticks_;
  bool dpm_pending_{}, dpm_target_{};
  bool enabled_{}, initialized_{}, active_{}, fault_{}, sleeping_{},
      after_xip_{}, control_pending_{}, terminal_seen_{}, mutation_{},
      long_frame_{}, last_frame_{};
};
} // namespace mcubsp::backend::nrf52840
