#pragma once
// @req-file REQ-0055 REQ-0072
#include <concepts>
#include <mcubsp/backend/nrf52840/system_context.hpp>
#include <mcubsp/mcu/nrf52840/product/ficr.hpp>
namespace mcubsp::backend::nrf52840 {
template <class R>
concept factory_register =
    std::same_as<R, mcu::nrf52840::product::ficr::codepagesize> ||
    std::same_as<R, mcu::nrf52840::product::ficr::codesize> ||
    std::same_as<R, mcu::nrf52840::product::ficr::deviceid_0> ||
    std::same_as<R, mcu::nrf52840::product::ficr::deviceid_1> ||
    std::same_as<R, mcu::nrf52840::product::ficr::er_0> ||
    std::same_as<R, mcu::nrf52840::product::ficr::er_1> ||
    std::same_as<R, mcu::nrf52840::product::ficr::er_2> ||
    std::same_as<R, mcu::nrf52840::product::ficr::er_3> ||
    std::same_as<R, mcu::nrf52840::product::ficr::ir_0> ||
    std::same_as<R, mcu::nrf52840::product::ficr::ir_1> ||
    std::same_as<R, mcu::nrf52840::product::ficr::ir_2> ||
    std::same_as<R, mcu::nrf52840::product::ficr::ir_3> ||
    std::same_as<R, mcu::nrf52840::product::ficr::deviceaddrtype> ||
    std::same_as<R, mcu::nrf52840::product::ficr::deviceaddr_0> ||
    std::same_as<R, mcu::nrf52840::product::ficr::deviceaddr_1> ||
    std::same_as<R, mcu::nrf52840::product::ficr::info_part> ||
    std::same_as<R, mcu::nrf52840::product::ficr::info_variant> ||
    std::same_as<R, mcu::nrf52840::product::ficr::info_package> ||
    std::same_as<R, mcu::nrf52840::product::ficr::info_ram> ||
    std::same_as<R, mcu::nrf52840::product::ficr::info_flash> ||
    std::same_as<R, mcu::nrf52840::product::ficr::prodtest_0> ||
    std::same_as<R, mcu::nrf52840::product::ficr::prodtest_1> ||
    std::same_as<R, mcu::nrf52840::product::ficr::prodtest_2> ||
    std::same_as<R, mcu::nrf52840::product::ficr::temp_a0> ||
    std::same_as<R, mcu::nrf52840::product::ficr::temp_a1> ||
    std::same_as<R, mcu::nrf52840::product::ficr::temp_a2> ||
    std::same_as<R, mcu::nrf52840::product::ficr::temp_a3> ||
    std::same_as<R, mcu::nrf52840::product::ficr::temp_a4> ||
    std::same_as<R, mcu::nrf52840::product::ficr::temp_a5> ||
    std::same_as<R, mcu::nrf52840::product::ficr::temp_b0> ||
    std::same_as<R, mcu::nrf52840::product::ficr::temp_b1> ||
    std::same_as<R, mcu::nrf52840::product::ficr::temp_b2> ||
    std::same_as<R, mcu::nrf52840::product::ficr::temp_b3> ||
    std::same_as<R, mcu::nrf52840::product::ficr::temp_b4> ||
    std::same_as<R, mcu::nrf52840::product::ficr::temp_b5> ||
    std::same_as<R, mcu::nrf52840::product::ficr::temp_t0> ||
    std::same_as<R, mcu::nrf52840::product::ficr::temp_t1> ||
    std::same_as<R, mcu::nrf52840::product::ficr::temp_t2> ||
    std::same_as<R, mcu::nrf52840::product::ficr::temp_t3> ||
    std::same_as<R, mcu::nrf52840::product::ficr::temp_t4> ||
    std::same_as<R, mcu::nrf52840::product::ficr::nfc_tagheader0> ||
    std::same_as<R, mcu::nrf52840::product::ficr::nfc_tagheader1> ||
    std::same_as<R, mcu::nrf52840::product::ficr::nfc_tagheader2> ||
    std::same_as<R, mcu::nrf52840::product::ficr::nfc_tagheader3> ||
    std::same_as<R, mcu::nrf52840::product::ficr::trng90b_bytes> ||
    std::same_as<R, mcu::nrf52840::product::ficr::trng90b_rccutoff> ||
    std::same_as<R, mcu::nrf52840::product::ficr::trng90b_apcutoff> ||
    std::same_as<R, mcu::nrf52840::product::ficr::trng90b_startup> ||
    std::same_as<R, mcu::nrf52840::product::ficr::trng90b_rosc1> ||
    std::same_as<R, mcu::nrf52840::product::ficr::trng90b_rosc2> ||
    std::same_as<R, mcu::nrf52840::product::ficr::trng90b_rosc3> ||
    std::same_as<R, mcu::nrf52840::product::ficr::trng90b_rosc4>;
struct factory_geometry {
  std::uint32_t page_bytes{}, pages{}, ram_kib{}, flash_kib{};
};
struct factory_address {
  std::array<std::uint8_t, 6> bytes;
  bool random;
};
template <class Context> class factory_information {
public:
  explicit factory_information(Context &c) noexcept : c_(c) {}
  template <factory_register R>
  [[nodiscard]] system_value<std::uint32_t> read() noexcept {
    return c_.read(mcu::nrf52840::product::ficr::base + R::offset);
  }
  [[nodiscard]] auto identity() noexcept { return c_.identity(); }
  [[nodiscard]] system_value<factory_geometry> geometry() noexcept {
    namespace f = mcu::nrf52840::product::ficr;
    auto page = read<f::codepagesize>(), pages = read<f::codesize>(),
         ram = read<f::info_ram>(), flash = read<f::info_flash>();
    if (!page || !pages || !ram || !flash)
      return std::unexpected(system_error::mmio);
    return factory_geometry{*page, *pages, *ram, *flash};
  }
  [[nodiscard]] system_value<factory_address> device_address() noexcept {
    namespace f = mcu::nrf52840::product::ficr;
    auto type = read<f::deviceaddrtype>(), lo = read<f::deviceaddr_0>(),
         hi = read<f::deviceaddr_1>();
    if (!type || !lo || !hi)
      return std::unexpected(system_error::mmio);
    factory_address a{{}, bool(*type & 1U)};
    for (unsigned i = 0; i < 4; ++i)
      a.bytes[i] = *lo >> (8 * i);
    a.bytes[4] = *hi;
    a.bytes[5] = *hi >> 8;
    return a;
  }
  [[nodiscard]] system_value<std::array<std::uint32_t, 17>>
  temperature_coefficients() noexcept {
    std::array<std::uint32_t, 17> result{};
    for (unsigned i = 0; i < 17; ++i) {
      auto v = c_.read(0x10000404 + 4 * i);
      if (!v)
        return std::unexpected(v.error());
      result[i] = *v;
    }
    return result;
  }
  [[nodiscard]] system_value<std::array<std::uint32_t, 4>>
  nfc_header() noexcept {
    std::array<std::uint32_t, 4> result{};
    for (unsigned i = 0; i < 4; ++i) {
      auto v = c_.read(0x10000450 + 4 * i);
      if (!v)
        return std::unexpected(v.error());
      result[i] = *v;
    }
    return result;
  }

private:
  Context &c_;
};
} // namespace mcubsp::backend::nrf52840
