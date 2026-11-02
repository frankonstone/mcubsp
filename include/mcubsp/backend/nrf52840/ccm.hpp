#pragma once
// @req-file REQ-0071 REQ-0062
// PS v1.11 6.4: Bluetooth packet CCM, 4-byte MIC, 39-bit packet counter.
#include <mcubsp/backend/nrf52840/detail/peripheral_session.hpp>
namespace mcubsp::backend::nrf52840 {
struct alignas(4) ccm_nonce_configuration {
  std::array<std::uint8_t, 16> key{};
  std::array<std::uint8_t, 8> packet_counter{};
  std::uint8_t direction{};
  std::array<std::uint8_t, 8> iv{};
};
static_assert(offsetof(ccm_nonce_configuration, iv) == 25);
enum class ccm_data_rate : unsigned { mbit1, mbit2, kbit125, kbit500 };
struct ccm_options {
  bool decrypt{}, extended{}, automatic_crypt{true};
  unsigned maximum_payload{27};
  ccm_data_rate data_rate{};
};
struct ccm_result {
  std::size_t bytes{};
  bool mic_valid{};
};
template <class Context, class Buffers, class Address = native_dma_address>
class ccm_controller {
  using session = detail::peripheral_session<Context, Buffers, Address>;

public:
  ccm_controller(Context &c, clock_controller<Context> &clocks, Buffers &b,
                 Address a = {}) noexcept
      : s_(c, clocks, b, a) {}
  ccm_controller(const ccm_controller &) = delete;
  ccm_controller &operator=(const ccm_controller &) = delete;
  ~ccm_controller() {
    if (active_)
      s_.context.execution.active_transfer_destroyed();
  }
  [[nodiscard]] system_result initialize() noexcept {
    auto r = s_.acquire_routed(peripheral::ccm, 77, endpoint_domain::ccm,
                               0x4000F000);
    if (!r)
      return r;
    auto enabled = s_.read(0x500);
    if (!enabled || *enabled) {
      (void)s_.release();
      return std::unexpected(enabled ? system_error::busy : system_error::mmio);
    }
    s_.publish();
    if (!s_.write(0x308, 7) || !clear() || !s_.write(0x200, 0) ||
        !s_.write(0x500, 2))
      return fail();
    initialized_ = true;
    return {};
  }
  [[nodiscard]] system_result
  start(const ccm_nonce_configuration &configuration,
        std::span<const std::uint8_t> packet, std::span<std::uint8_t> output,
        std::span<std::uint8_t> scratch, ccm_options options = {}) noexcept {
    if (!initialized_ || active_ || fault_ || s_.context.execution.in_isr())
      return std::unexpected(system_error::busy);
    if (packet.size() < 3 || unsigned(options.data_rate) > 3 ||
        !options.maximum_payload ||
        options.maximum_payload > (options.extended ? 251U : 27U) ||
        (!options.extended && options.maximum_payload != 27) ||
        (configuration.packet_counter[4] & 128) || configuration.direction > 1)
      return std::unexpected(system_error::invalid);
    auto length = packet[1];
    unsigned plain = options.decrypt ? (length >= 4 ? length - 4 : 0) : length;
    if ((options.decrypt && length > 0 && length < 5) ||
        plain > options.maximum_payload ||
        packet.size() < std::size_t(length) + 3)
      return std::unexpected(system_error::invalid);
    output_bytes_ =
        3 + (length ? (options.decrypt ? length - 4 : length + 4) : 0);
    std::size_t scratch_bytes =
        options.extended ? 16 + options.maximum_payload : 43;
    if (output.size() < output_bytes_ || scratch.size() < scratch_bytes)
      return std::unexpected(system_error::invalid);
    if (!prepare(0, &configuration, 33, dma_access::read) ||
        !prepare(1, packet.data(), length + 3, dma_access::read) ||
        !prepare(2, output.data(), output_bytes_, dma_access::write) ||
        !prepare(3, scratch.data(), scratch_bytes, dma_access::read_write)) {
      discard();
      return std::unexpected(system_error::invalid);
    }
    auto mode = (options.decrypt ? 1U : 0U) |
                (unsigned(options.data_rate) << 16) |
                (options.extended ? 1U << 24 : 0U);
    if (!clear() || !s_.write(0x504, mode) ||
        !s_.write(0x518, options.maximum_payload) ||
        !s_.write(0x200, options.automatic_crypt) ||
        !s_.write(0x508, s_.address(&configuration, 33)) ||
        !s_.write(0x50C, s_.address(packet.data(), length + 3)) ||
        !s_.write(0x510, s_.address(output.data(), output_bytes_)) ||
        !s_.write(0x514, s_.address(scratch.data(), scratch_bytes))) {
      discard();
      return fail();
    }
    options_ = options;
    active_ = true;
    terminal_ = key_ready_ = crypt_started_ = error_ = false;
    for (auto &v : leases_)
      if (!s_.buffers.arm(*v))
        return fail();
    crypt_started_ = options.automatic_crypt;
    if (!s_.write(0, 1))
      return fail();
    return {};
  }
  [[nodiscard]] system_result crypt() noexcept {
    if (!active_ || !key_ready_ || crypt_started_)
      return std::unexpected(system_error::busy);
    crypt_started_ = true;
    if (!s_.write(4, 1))
      return fail();
    return {};
  }
  [[nodiscard]] system_result override_rate(ccm_data_rate rate) noexcept {
    if (!active_ || unsigned(rate) > 3)
      return std::unexpected(system_error::invalid);
    if (!s_.write(0x51C, unsigned(rate)) || !s_.write(12, 1))
      return fail();
    return {};
  }
  [[nodiscard]] system_value<std::optional<ccm_result>> poll() noexcept {
    return s_.context.execution.critical(
        [&]() -> system_value<std::optional<ccm_result>> {
          if (!active_)
            return std::optional<ccm_result>{};
          auto key = s_.read(0x100), end = s_.read(0x104),
               error = s_.read(0x108);
          if (!key || !end || !error)
            return std::unexpected(system_error::mmio);
          key_ready_ = key_ready_ || *key;
          terminal_ = terminal_ || *end;
          error_ = error_ || *error;
          if (error_) {
            auto r = cancel_impl();
            if (!r)
              return std::unexpected(r.error());
            return std::unexpected(system_error::faulted);
          }
          if (!terminal_) {
            if (*key && !s_.write(0x100, 0))
              return std::unexpected(system_error::mmio);
            return std::optional<ccm_result>{};
          }
          auto mic = s_.read(0x400);
          if (!mic || !clear() || !release())
            return std::unexpected(system_error::mmio);
          active_ = false;
          return std::optional{
              ccm_result{output_bytes_, !options_.decrypt || *mic == 1}};
        });
  }
  [[nodiscard]] bool key_ready() const noexcept { return key_ready_; }
  [[nodiscard]] system_result cancel() noexcept {
    if (s_.context.execution.in_isr())
      return std::unexpected(system_error::invalid);
    return cancel_impl();
  }
  [[nodiscard]] system_result interrupt_delivery(bool on) noexcept {
    if (!initialized_)
      return std::unexpected(system_error::not_initialized);
    return s_.write(on ? 0x304 : 0x308, 7) ? system_result{} : fail();
  }
  [[nodiscard]] system_result shutdown() noexcept {
    if (s_.context.execution.in_isr())
      return std::unexpected(system_error::invalid);
    if (!s_.owned())
      return {};
    auto r = cancel_impl();
    if (!r)
      return r;
    if (!s_.write(0x308, 7) || !s_.write(0x200, 0) || !s_.disabled() ||
        !clear())
      return fail();
    r = s_.release();
    if (r)
      initialized_ = fault_ = false;
    return r;
  }

  [[nodiscard]] system_endpoint event_endpoint(unsigned offset) const noexcept {
    return s_.endpoint(offset, endpoint_kind::event);
  }
  [[nodiscard]] system_endpoint task_endpoint(unsigned offset) const noexcept {
    return active_ ? s_.endpoint(offset, endpoint_kind::task)
                   : system_endpoint{};
  }

private:
  system_result cancel_impl() {
    if (s_.routes_referenced())
      return std::unexpected(system_error::busy);
    if (!active_)
      return {};
    if (!s_.write(0x200, 0) || !s_.write(8, 1) || !s_.disabled() || !release())
      return fail();
    active_ = initialized_ = false;
    return {};
  }
  bool prepare(unsigned i, const void *p, std::size_t bytes,
               dma_access access) {
    auto r = s_.prepare(p, bytes, bytes, {267, 1, 1}, access);
    if (!r)
      return false;
    leases_[i] = *r;
    return true;
  }
  void discard() {
    for (auto &v : leases_)
      s_.discard(v);
  }
  bool release() {
    for (auto &v : leases_)
      if (!s_.release_quiet(v))
        return false;
    return true;
  }
  bool clear() {
    return s_.write(0x100, 0) && s_.write(0x104, 0) && s_.write(0x108, 0);
  }
  system_result fail() {
    fault_ = true;
    return std::unexpected(system_error::mmio);
  }
  session s_;
  std::array<std::optional<dma_buffer_token>, 4> leases_{};
  ccm_options options_{};
  std::size_t output_bytes_{};
  bool initialized_{}, active_{}, fault_{}, terminal_{}, key_ready_{},
      crypt_started_{}, error_{};
};
} // namespace mcubsp::backend::nrf52840
