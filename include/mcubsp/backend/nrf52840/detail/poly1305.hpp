#pragma once
// @req-file REQ-0071
// Fixed-storage RFC 8439 authenticator. 26-bit limbs, no secret-indexed tables.
#include <array>
#include <cstdint>
#include <span>
namespace mcubsp::backend::nrf52840::detail {
class poly1305 {
public:
  explicit poly1305(std::span<const std::uint8_t, 32> key) {
    r_[0] = load(key.data()) & 0x3ffffff;
    r_[1] = (load(key.data() + 3) >> 2) & 0x3ffff03;
    r_[2] = (load(key.data() + 6) >> 4) & 0x3ffc0ff;
    r_[3] = (load(key.data() + 9) >> 6) & 0x3f03fff;
    r_[4] = (load(key.data() + 12) >> 8) & 0xfffff;
    for (unsigned i = 0; i < 4; ++i)
      pad_[i] = load(key.data() + 16 + 4 * i);
  }
  void update(std::span<const std::uint8_t> bytes) {
    for (auto b : bytes) {
      block_[used_++] = b;
      if (used_ == 16) {
        process(1U << 24);
        used_ = 0;
      }
    }
  }
  std::array<std::uint8_t, 16> finish() {
    if (used_) {
      block_[used_++] = 1;
      while (used_ < 16)
        block_[used_++] = 0;
      process(0);
    }
    std::uint32_t c = h_[1] >> 26;
    h_[1] &= 0x3ffffff;
    h_[2] += c;
    c = h_[2] >> 26;
    h_[2] &= 0x3ffffff;
    h_[3] += c;
    c = h_[3] >> 26;
    h_[3] &= 0x3ffffff;
    h_[4] += c;
    c = h_[4] >> 26;
    h_[4] &= 0x3ffffff;
    h_[0] += c * 5;
    c = h_[0] >> 26;
    h_[0] &= 0x3ffffff;
    h_[1] += c;
    std::array<std::uint32_t, 5> g{};
    g[0] = h_[0] + 5;
    c = g[0] >> 26;
    g[0] &= 0x3ffffff;
    for (unsigned i = 1; i < 4; ++i) {
      g[i] = h_[i] + c;
      c = g[i] >> 26;
      g[i] &= 0x3ffffff;
    }
    g[4] = h_[4] + c - (1U << 26);
    auto mask = (g[4] >> 31) - 1;
    for (unsigned i = 0; i < 5; ++i)
      h_[i] = (h_[i] & ~mask) | (g[i] & mask);
    std::array<std::uint32_t, 4> words{
        h_[0] | h_[1] << 26, (h_[1] >> 6) | (h_[2] << 20),
        (h_[2] >> 12) | (h_[3] << 14), (h_[3] >> 18) | (h_[4] << 8)};
    std::array<std::uint8_t, 16> tag{};
    std::uint64_t carry = 0;
    for (unsigned i = 0; i < 4; ++i) {
      carry = std::uint64_t(words[i]) + pad_[i] + (carry >> 32);
      for (unsigned j = 0; j < 4; ++j)
        tag[4 * i + j] = carry >> (8 * j);
    }
    return tag;
  }

private:
  static std::uint32_t load(const std::uint8_t *p) {
    return std::uint32_t(p[0]) | std::uint32_t(p[1]) << 8 |
           std::uint32_t(p[2]) << 16 | std::uint32_t(p[3]) << 24;
  }
  void process(std::uint32_t high) {
    h_[0] += load(block_.data()) & 0x3ffffff;
    h_[1] += (load(block_.data() + 3) >> 2) & 0x3ffffff;
    h_[2] += (load(block_.data() + 6) >> 4) & 0x3ffffff;
    h_[3] += (load(block_.data() + 9) >> 6) & 0x3ffffff;
    h_[4] += (load(block_.data() + 12) >> 8) | high;
    std::array<std::uint64_t, 5> d{};
    for (unsigned i = 0; i < 5; ++i)
      for (unsigned j = 0; j < 5; ++j) {
        auto k = (i + 5 - j) % 5;
        d[i] += std::uint64_t(h_[j]) * r_[k] * (j > i ? 5 : 1);
      }
    for (unsigned i = 0; i < 4; ++i) {
      d[i + 1] += d[i] >> 26;
      h_[i] = d[i] & 0x3ffffff;
    }
    h_[4] = d[4] & 0x3ffffff;
    h_[0] += (d[4] >> 26) * 5;
    auto c = h_[0] >> 26;
    h_[0] &= 0x3ffffff;
    h_[1] += c;
  }
  std::array<std::uint32_t, 5> r_{}, h_{};
  std::array<std::uint32_t, 4> pad_{};
  std::array<std::uint8_t, 16> block_{};
  unsigned used_{};
};
} // namespace mcubsp::backend::nrf52840::detail
