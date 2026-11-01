#pragma once
// @req-file REQ-0045 REQ-0046 REQ-0056 REQ-0059 REQ-0060 REQ-0061 REQ-0062
#include <mcubsp/backend/nrf52840/spi.hpp>
#include <mcubsp/backend/nrf52840/twim.hpp>
#include <mcubsp/backend/nrf52840/uart.hpp>
#include <mcubsp/backend/nrf52840/uarte.hpp>
namespace mcubsp::backend::nrf52840 {
// Exactly one service set per facade board context. Selected peripherals borrow
// it; no separate clock controller or DMA pool may compete for the same chip.
template <class M, class E> struct io_services {
  gpio_backend<M> gpio;
  system_context<M, E> context;
  clock_controller<decltype(context)> clocks;
  dma_buffer_pool<E, 16> buffers;
  bool clocks_initialized{};
  io_services(M &mmio, E &execution) noexcept
      : gpio(mmio), context(mmio, execution, gpio), clocks(context),
        buffers(gpio.resources(), execution) {}
  [[nodiscard]] bool ensure() noexcept {
    if (clocks_initialized)
      return true;
    if (!clocks.initialize())
      return false;
    clocks_initialized = true;
    return true;
  }
  [[nodiscard]] bool shutdown() noexcept {
    if (context.active_work())
      return false;
    if (clocks_initialized && !clocks.shutdown())
      return false;
    clocks_initialized = false;
    return true;
  }
};
namespace detail {
template <class Binding, class S,
          bool Cpu = (Binding::instance == peripheral::uart0)>
struct io_serial_native;
template <class Binding, class S> struct io_serial_native<Binding, S, true> {
  using type = uart_controller<Binding, decltype(std::declval<S &>().context)>;
};
template <class Binding, class S> struct io_serial_native<Binding, S, false> {
  using type = uarte_controller<Binding, decltype(std::declval<S &>().context),
                                decltype(std::declval<S &>().buffers)>;
};
template <class Binding> struct io_spi_binding : Binding {
  static constexpr auto cs = spi_disconnected;
  static constexpr std::array pins{Binding::sck, Binding::mosi, Binding::miso,
                                   spi_disconnected};
};
} // namespace detail
template <class Binding, class M, class E> class io_serial_backend {
  using S = io_services<M, E>;
  using native = typename detail::io_serial_native<Binding, S>::type;

public:
  using tx_transfer = typename native::tx_pool::transfer_type;
  using rx_transfer = typename native::rx_pool::transfer_type;
  explicit io_serial_backend(S &s) noexcept : services_(s), native_(make(s)) {}
  [[nodiscard]] std::expected<void, driver::uart_error>
  initialize(uart_baud_timing timing,
             driver::uart_format format = {}) noexcept {
    if (!services_.ensure())
      return std::unexpected(driver::uart_error::transport);
    return native_.initialize(timing, format);
  }
  auto transmit(std::span<const std::uint8_t> b) noexcept {
    return native_.transmit(b);
  }
  auto receive(std::span<std::uint8_t> b) noexcept {
    return native_.receive(b);
  }
  auto receive_stream(std::span<const std::span<std::uint8_t>> b,
                      std::size_t maximum) noexcept {
    return native_.receive_stream(b, maximum);
  }
  auto queue_receive_buffer(std::span<std::uint8_t> b) noexcept {
    return native_.queue_receive_buffer(b);
  }
  auto take_received_block() noexcept { return native_.take_received_block(); }
  auto stop_receive() noexcept { return native_.stop_receive(); }
  auto received_count() const noexcept { return native_.received_count(); }
  auto error_source() const noexcept { return native_.error_source(); }
  auto shutdown() noexcept { return native_.shutdown(); }
  bool needs_cleanup() const noexcept { return native_.owns_resource(); }
  bool poll() noexcept { return native_.poll(); }

private:
  static native make(S &s) noexcept {
    if constexpr (Binding::instance == peripheral::uart0)
      return native{s.context, s.clocks};
    else
      return native{s.context, s.clocks, s.buffers};
  }
  S &services_;
  native native_;
};
template <class Binding, class M, class E> class io_twim_backend {
  using S = io_services<M, E>;
  using native = twim_controller<Binding, decltype(std::declval<S &>().context),
                                 decltype(std::declval<S &>().buffers)>;

public:
  using transfer_type = typename native::transfer_type;
  explicit io_twim_backend(S &s) noexcept
      : services_(s), native_(s.context, s.clocks, s.buffers) {}
  auto initialize(twim_timing timing) noexcept
      -> std::expected<void, driver::i2c_error> {
    if (!services_.ensure())
      return std::unexpected(driver::i2c_error::transport);
    return native_.initialize(timing);
  }
  auto write(driver::i2c_address a, std::span<const std::uint8_t> b) noexcept {
    return native_.write(a, b);
  }
  auto read(driver::i2c_address a, std::span<std::uint8_t> b) noexcept {
    return native_.read(a, b);
  }
  auto write_read(driver::i2c_address a, std::span<const std::uint8_t> t,
                  std::span<std::uint8_t> r) noexcept {
    return native_.write_read(a, t, r);
  }
  auto read_write(driver::i2c_address a, std::span<std::uint8_t> r,
                  std::span<const std::uint8_t> t) noexcept {
    return native_.read_write(a, r, t);
  }
  bool needs_cleanup() const noexcept { return native_.owns_resource(); }
  bool poll() noexcept { return native_.poll(); }
  auto shutdown() noexcept { return native_.shutdown(); }

private:
  S &services_;
  native native_;
};
template <class Binding, class M, class E> class io_spi_backend {
  static constexpr bool dma = Binding::easy_dma;
  static_assert(Binding::peripheral_id == peripheral::spi0 ||
                    Binding::peripheral_id == peripheral::spi1 ||
                    Binding::peripheral_id == peripheral::spi2 ||
                    Binding::peripheral_id == peripheral::spim0 ||
                    Binding::peripheral_id == peripheral::spim1 ||
                    Binding::peripheral_id == peripheral::spim2 ||
                    Binding::peripheral_id == peripheral::spim3,
                "SPI facade requires a master binding");
  using S = io_services<M, E>;
  using B = detail::io_spi_binding<Binding>;
  using native = std::conditional_t<
      dma,
      spim_controller<B, decltype(std::declval<S &>().context),
                      decltype(std::declval<S &>().buffers), native_dma_address,
                      8, 1000000, true>,
      spi_controller<B, decltype(std::declval<S &>().context)>>;

public:
  using transfer_type = typename native::transfer_type;
  explicit io_spi_backend(S &s) noexcept : services_(s), native_(make(s)) {}
  auto initialize(spim_timing timing,
                  driver::spi_configuration config = {}) noexcept
      -> std::expected<void, driver::spi_error> {
    if (!services_.ensure())
      return std::unexpected(driver::spi_error::transport);
    if constexpr (dma)
      return native_.initialize(timing, config, {Binding::dedicated_ahb_slave});
    else
      return native_.initialize(timing, config);
  }
  auto reconfigure(spim_timing t, driver::spi_configuration c = {}) noexcept {
    return native_.reconfigure(t, c);
  }
  auto set_select_handler(bool (*f)(void *, bool) noexcept, void *p) noexcept {
    return native_.set_select_handler(f, p);
  }
  auto transaction(std::span<const driver::spi_segment> s) noexcept {
    return native_.transaction(s);
  }
  bool needs_cleanup() const noexcept { return native_.owns_resource(); }
  auto recover() noexcept { return native_.shutdown(); }
  bool busy() const noexcept { return native_.operation_pending(); }

  bool poll() noexcept { return native_.poll(); }
  auto shutdown() noexcept { return native_.shutdown(); }

private:
  static native make(S &s) noexcept {
    if constexpr (dma)
      return native{s.context, s.clocks, s.buffers};
    else
      return native{s.context, s.clocks};
  }
  S &services_;
  native native_;
};
template <class Binding, class M, class E,
          bool Dma = requires { Binding::easy_dma; }>
struct io_i2c_backend_selector {
  static_assert(
      !requires { Binding::peripheral_id; },
      "I2C master facade requires a TWI or TWIM binding");
  using type = i2c_controller_backend<Binding, M, E>;
};
template <class Binding, class M, class E>
struct io_i2c_backend_selector<Binding, M, E, true> {
  using type = io_twim_backend<Binding, M, E>;
};
} // namespace mcubsp::backend::nrf52840
