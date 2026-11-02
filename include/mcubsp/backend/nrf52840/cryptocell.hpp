#pragma once
// @req-file REQ-0071 REQ-0062
// Direct PS v1.11 6.6 engine flows. No SDK, root-key provisioning or lifecycle
// change is implicit. Caller DMA storage survives failures until recover().
#include <algorithm>
#include <mcubsp/backend/nrf52840/detail/peripheral_session.hpp>
#include <mcubsp/backend/nrf52840/detail/poly1305.hpp>
#include <mcubsp/mcu/nrf52840/product/cc_pka.hpp>
namespace mcubsp::backend::nrf52840 {
enum class cc_aes_mode : unsigned {
  ecb = 0,
  cbc = 1,
  ctr = 2,
  cbc_mac = 3,
  cmac = 7
};
enum class cc_hash_mode : unsigned { sha1 = 1, sha256 = 2, sha224 = 10 };
struct cc_aes_configuration {
  cc_aes_mode mode{cc_aes_mode::ecb};
  bool decrypt{};
  std::array<std::uint32_t, 4> key{}, iv{}; // little-endian byte words
};
struct cc_chacha_configuration {
  std::array<std::uint32_t, 8> key{};
  std::array<std::uint32_t, 3> nonce{};
  std::uint64_t counter{};
  bool key128{}, nonce96{true}, poly1305_key{};
};
struct cc_digest {
  std::array<std::uint8_t, 32> bytes{};
  unsigned size{};
};
struct cc_pka_operation {
  mcu::nrf52840::product::cc_pka::opcode_fields::opcode_encoding operation;
  unsigned a{2}, b{3}, result{4}, tag{};
  bool constant_a{}, constant_b{}, discard_result{};
};
// Bounded foreground operations, with DMA and engine completion both checked.
// DIN/DOUT are not EasyDMA. Only Data RAM is accepted; 65535 bytes is the
// documented physical transfer limit despite the 30-bit SIZE register fields.
template <class Context, class Buffers, class Address = native_dma_address,
          unsigned PollBudget = 1000000>
class cryptocell_controller {
  using session = detail::peripheral_session<Context, Buffers, Address>;

public:
  cryptocell_controller(Context &c, clock_controller<Context> &clocks,
                        Buffers &b, execution::tick timeout = 64'000'000,
                        Address a = {}) noexcept
      : s_(c, clocks, b, a), timeout_(timeout) {}
  cryptocell_controller(const cryptocell_controller &) = delete;
  cryptocell_controller &operator=(const cryptocell_controller &) = delete;
  ~cryptocell_controller() {
    if (input_ || output_)
      s_.context.execution.active_transfer_destroyed();
  }
  [[nodiscard]] system_result initialize() noexcept {
    if (!timeout_ || timeout_ > execution::maximum_deadline_interval)
      return std::unexpected(system_error::invalid);
    auto claim = s_.acquire(peripheral::cryptocell, base, {}, std::nullopt);
    if (!claim)
      return claim;
    auto enabled = s_.context.read(enable_address);
    if (!enabled || *enabled) {
      (void)s_.release();
      return std::unexpected(enabled ? system_error::busy : system_error::mmio);
    }
    s_.publish();
    enable_attempted_ = true;
    if (!s_.context.write(enable_address, 1))
      return fail();
    for (auto off : clock_offsets)
      if (!write(off, 1))
        return fail();
    if (!write(0x1C4, 1) || !write(0xA04, 0x7F0) || !write(0x100, 0x3F) ||
        !write(0x108, 0x3F) || !write(0xA08, 0x7F0) || !write(0xA0C, 0))
      return fail();
    auto signature = read(0xA24);
    if (!signature || *signature != 0x20E00000)
      return fail();
    initialized_ = true;
    return {};
  }
  [[nodiscard]] system_result aes(cc_aes_configuration configuration,
                                  std::span<const std::uint8_t> input,
                                  std::span<std::uint8_t> output) noexcept {
    auto check = available();
    if (!check)
      return check;
    auto mode = unsigned(configuration.mode);
    if ((mode != 0 && mode != 1 && mode != 2) || input.empty() ||
        input.size() != output.size() || (mode != 2 && input.size() % 16))
      return std::unexpected(system_error::invalid);
    if (!configure_aes(configuration))
      return fail();
    return dma(input, output, 0xC0);
  }
  [[nodiscard]] system_value<std::array<std::uint8_t, 16>>
  mac(cc_aes_configuration configuration,
      std::span<const std::uint8_t> input) noexcept {
    auto check = available();
    if (!check)
      return std::unexpected(check.error());
    auto mode = unsigned(configuration.mode);
    if ((mode != 3 && mode != 7) || configuration.decrypt ||
        (mode == 3 && (input.empty() || input.size() % 16)))
      return std::unexpected(system_error::invalid);
    if (!configure_aes(configuration) || !write(0x4BC, input.size()) ||
        !write(0x900, 9))
      return std::unexpected(system_error::mmio);
    if (mode == 7) {
      if (!write(0x47C, 1) || !wait_zero(0x470))
        return std::unexpected(system_error::timeout);
    }
    if (input.empty()) {
      if (!write(0x524, 1) || !wait_zero(0x470))
        return std::unexpected(system_error::timeout);
    } else {
      auto done = dma(input, {}, 0x40);
      if (!done)
        return std::unexpected(done.error());
    }
    std::array<std::uint8_t, 16> result{};
    for (unsigned i = 0; i < 4; ++i) {
      auto v = read(0x440 + 4 * i);
      if (!v)
        return std::unexpected(system_error::mmio);
      for (unsigned j = 0; j < 4; ++j)
        result[4 * i + j] = *v >> (8 * j);
    }
    return result;
  }
  [[nodiscard]] system_result chacha(cc_chacha_configuration configuration,
                                     std::span<const std::uint8_t> input,
                                     std::span<std::uint8_t> output) noexcept {
    auto check = available();
    if (!check)
      return check;
    if (input.empty() || input.size() != output.size() ||
        (configuration.nonce96 && configuration.counter > 0xFFFFFFFFULL) ||
        (configuration.nonce96 &&
         ((input.size() + 63) / 64) > 0x100000000ULL - configuration.counter))
      return std::unexpected(system_error::invalid);
    if (!write(0x3C0, 1) || !write(0x900, 16) || !write(0x3E4, 0))
      return fail();
    for (unsigned i = 0; i < 8; ++i)
      if (!write(0x388 + 4 * i,
                 configuration.key[configuration.key128 ? i % 4 : i]))
        return fail();
    if (!write(0x3A8, configuration.nonce[configuration.nonce96 ? 1 : 0]) ||
        !write(0x3AC, configuration.nonce[configuration.nonce96 ? 2 : 1]) ||
        !write(0x3B8, configuration.counter) ||
        !write(0x3BC, configuration.nonce96 ? configuration.nonce[0]
                                            : configuration.counter >> 32) ||
        !write(0x380, 2U | (configuration.key128 ? 8U : 0U) |
                          (configuration.nonce96 ? 128U : 0U) |
                          (configuration.poly1305_key ? 4U : 0U)))
      return fail();
    return dma(input, output, 0xC0);
  }
  [[nodiscard]] system_value<std::array<std::uint32_t, 8>>
  poly1305_key() noexcept {
    auto check = available();
    if (!check)
      return std::unexpected(check.error());
    std::array<std::uint32_t, 8> key{};
    for (unsigned i = 0; i < 8; ++i) {
      auto v = read(0x3C4 + 4 * i);
      if (!v)
        return std::unexpected(system_error::mmio);
      key[i] = *v;
    }
    return key;
  }
  [[nodiscard]] system_value<cc_digest>
  hash(cc_hash_mode mode, std::span<const std::uint8_t> input) noexcept {
    auto check = available();
    if (!check)
      return std::unexpected(check.error());
    if (mode != cc_hash_mode::sha1 && mode != cc_hash_mode::sha224 &&
        mode != cc_hash_mode::sha256)
      return std::unexpected(system_error::invalid);
    constexpr std::array<std::uint32_t, 8> sha256{
        0x6A09E667, 0xBB67AE85, 0x3C6EF372, 0xA54FF53A,
        0x510E527F, 0x9B05688C, 0x1F83D9AB, 0x5BE0CD19};
    constexpr std::array<std::uint32_t, 8> sha224{
        0xC1059ED8, 0x367CD507, 0x3070DD17, 0xF70E5939,
        0xFFC00B31, 0x68581511, 0x64F98FA7, 0xBEFA4FA4};
    constexpr std::array<std::uint32_t, 8> sha1{
        0x67452301, 0xEFCDAB89, 0x98BADCFE, 0x10325476, 0xC3D2E1F0, 0, 0, 0};
    auto &initial = mode == cc_hash_mode::sha1     ? sha1
                    : mode == cc_hash_mode::sha224 ? sha224
                                                   : sha256;
    bool empty = input.empty();
    if (empty) {
      empty_hash_block_ = {};
      empty_hash_block_[0] = 0x80;
      input = empty_hash_block_;
    }
    if (!write(0x7E4, 1) || !write(0x900, 7) || !write(0x7C0, unsigned(mode)) ||
        !write(0x7E8, 1) || !write(0x7C4, !empty) || !write(0x684, !empty) ||
        !write(0x7CC, 0) || !write(0x7D0, 0))
      return std::unexpected(system_error::mmio);
    for (int i = mode == cc_hash_mode::sha1 ? 4 : 7; i >= 0; --i)
      if (!write(0x640 + 4 * i, initial[i]))
        return std::unexpected(system_error::mmio);
    auto done = dma(input, {}, 0x40);
    if (!done)
      return std::unexpected(done.error());
    cc_digest digest{};
    digest.size = mode == cc_hash_mode::sha1     ? 20
                  : mode == cc_hash_mode::sha224 ? 28
                                                 : 32;
    for (unsigned i = 0; i < digest.size / 4; ++i) {
      auto v = read(0x640 + 4 * i);
      if (!v)
        return std::unexpected(system_error::mmio);
      for (unsigned j = 0; j < 4; ++j)
        digest.bytes[4 * i + j] = *v >> (24 - 8 * j);
    }
    return digest;
  }
  [[nodiscard]] system_result copy(std::span<const std::uint8_t> input,
                                   std::span<std::uint8_t> output) noexcept {
    auto check = available();
    if (!check)
      return check;
    if (input.empty() || input.size() != output.size())
      return std::unexpected(system_error::invalid);
    if (!write(0x900, 0))
      return fail();
    return dma(input, output, 0xC0);
  }
  // PKA SRAM registers use physical WORD addresses in MEMORY_MAP and the
  // SRAM ports, following PS 6.6.12.4.5's worked example. All virtual registers
  // include 64 zero padding bits. Seven slots support operands up to 3136 bits.
  [[nodiscard]] system_result pka_configure(unsigned operand_bits) noexcept {
    auto check = available();
    if (!check)
      return check;
    if (operand_bits < 128 || operand_bits > 3136)
      return std::unexpected(system_error::invalid);
    if (!write(0x8C, 1) || !wait_set(0xB4, 1))
      return std::unexpected(system_error::timeout);
    pka_bits_ = operand_bits;
    pka_stride_ = ((std::max(5U, (operand_bits + 31) / 32) + 3) & ~1U);
    for (unsigned i = 0; i < 7; ++i)
      if (!write(4 * pka_registers[i], i * pka_stride_))
        return fail();
    if (!write(0x84, 0xFF820) || !write(0x90, operand_bits) ||
        !write(0xE0, 1) || !write(0xD4, 0))
      return fail();
    for (unsigned i = 0; i < 7 * pka_stride_; ++i)
      if (!write(0xD8, 0))
        return fail();
    return {};
  }
  [[nodiscard]] system_result
  pka_load(unsigned reg, std::span<const std::uint32_t> words) noexcept {
    auto check = available();
    if (!check)
      return check;
    auto slot = pka_slot(reg);
    if (!pka_bits_ || slot == 7 ||
        words.size() > (reg == 1 ? std::max(5U, (pka_bits_ + 31) / 32)
                                 : (pka_bits_ + 31) / 32))
      return std::unexpected(system_error::invalid);
    if (!write(0xE0, 1) || !write(0xD4, slot * pka_stride_))
      return fail();
    for (unsigned i = 0; i < pka_stride_; ++i) {
      auto value = i < words.size() ? words[i] : 0U;
      if (reg != 1 && i == (pka_bits_ - 1) / 32 && pka_bits_ % 32 &&
          value >> (pka_bits_ % 32))
        return std::unexpected(system_error::invalid);
      if (!write(0xD8, value))
        return fail();
    }
    return {};
  }
  [[nodiscard]] system_value<std::uint32_t>
  pka_execute(cc_pka_operation operation) noexcept {
    auto check = available();
    if (!check)
      return std::unexpected(check.error());
    auto op = unsigned(operation.operation);
    if (!pka_bits_ || !valid_pka_opcode(op) || operation.a > 31 ||
        operation.b > 31 || operation.result > 31 || operation.tag > 63 ||
        (!operation.constant_a && pka_slot(operation.a) == 7) ||
        (!operation.constant_b && pka_slot(operation.b) == 7) ||
        (!operation.discard_result && pka_slot(operation.result) == 7))
      return std::unexpected(system_error::invalid);
    if (((op >= 12 && op <= 15) &&
         (!operation.constant_b || operation.constant_a)) ||
        ((op > 15) && (operation.constant_a || operation.constant_b)) ||
        ((op == 24 || op == 25) && pka_slot(operation.tag) == 7))
      return std::unexpected(system_error::invalid);
    if (!wait_set(0xB0, 1) || !write(0xA08, 512))
      return std::unexpected(system_error::timeout);
    pka_pending_ = true;
    auto word = (op << 27) | (operation.constant_a ? 1U << 23 : 0U) |
                (operation.a << 18) | (operation.constant_b ? 1U << 17 : 0U) |
                (operation.b << 12) |
                (operation.discard_result ? 1U << 11 : 0U) |
                (operation.result << 6) | operation.tag;
    if (!write(0x80, word))
      return std::unexpected(system_error::mmio);
    if (!wait_set(0xB4, 1))
      return std::unexpected(system_error::timeout);
    pka_pending_ = false;
    auto status = read(0x88);
    if (!status)
      return std::unexpected(system_error::mmio);
    if (*status & 0xE000)
      return std::unexpected(system_error::faulted);
    if (!write(0xA08, 512))
      return std::unexpected(system_error::mmio);
    return *status;
  }
  [[nodiscard]] system_result
  pka_read(unsigned reg, std::span<std::uint32_t> words) noexcept {
    auto check = available();
    if (!check)
      return check;
    auto slot = pka_slot(reg);
    if (!pka_bits_ || slot == 7 ||
        words.size() > (reg == 1 ? std::max(5U, (pka_bits_ + 31) / 32)
                                 : (pka_bits_ + 31) / 32))
      return std::unexpected(system_error::invalid);
    if (!write(0xE0, 1) || !write(0xE4, slot * pka_stride_))
      return fail();
    for (auto &word : words) {
      auto v = read(0xDC);
      if (!v)
        return fail();
      word = *v;
    }
    return {};
  }
  [[nodiscard]] system_value<std::array<std::uint32_t, 6>> entropy() noexcept {
    auto check = available();
    if (!check)
      return std::unexpected(check.error());
    for (unsigned oscillator = 0; oscillator < 4; ++oscillator) {
      auto samples = s_.context.read(0x10000C10 + 4 * oscillator);
      if (!samples)
        return std::unexpected(system_error::mmio);
      if (!*samples || *samples == 0xFFFFFFFF)
        return std::unexpected(system_error::unsupported);
      if (!write(0x1C4, 1) || !write(0x140, 1) || !write(0x1C4, 1) ||
          !write(0x100, 0x3F) || !write(0x108, 0x3F) || !write(0xA08, 1024) ||
          !write(0x138, 0) || !write(0x130, *samples))
        return std::unexpected(system_error::mmio);
      if (!s_.bounded(
              [&] {
                auto r = read(0x130);
                return r && *r == *samples;
              },
              timeout_, PollBudget))
        return std::unexpected(system_error::timeout);
      rng_active_ = true;
      if (!write(0x10C, oscillator) || !write(0x12C, 1))
        return std::unexpected(system_error::mmio);
      std::uint32_t status = 0;
      if (!s_.bounded(
              [&] {
                auto r = read(0x104);
                if (r)
                  status = *r;
                return status & 31U;
              },
              timeout_, PollBudget))
        return std::unexpected(system_error::timeout);
      if (!write(0x12C, 0) || !wait_zero(0x1B8))
        return std::unexpected(system_error::timeout);
      rng_active_ = false;
      // Reject all health/watchdog faults, even when EHR_VALID is also set.
      if (status & 30U) {
        if (!write(0x108, 0x3F) || !write(0xA08, 1024))
          return std::unexpected(system_error::mmio);
        continue;
      }
      std::array<std::uint32_t, 6> result{};
      for (unsigned i = 0; i < 6; ++i) {
        auto v = read(0x114 + 4 * i);
        if (!v)
          return std::unexpected(system_error::mmio);
        result[i] = *v;
      }
      if (!write(0x108, 0x3F) || !write(0xA08, 1024))
        return std::unexpected(system_error::mmio);
      return result;
    }
    return std::unexpected(system_error::faulted);
  }
  [[nodiscard]] system_result
  rng_sram_write(unsigned offset,
                 std::span<const std::uint32_t> words) noexcept {
    auto check = available();
    if (!check)
      return check;
    if (offset % 4 || offset > 2048 || words.size() > (2048 - offset) / 4)
      return std::unexpected(system_error::invalid);
    sram_pending_ = true;
    if (!write(0xF04, offset))
      return fail();
    for (auto word : words) {
      if (!wait_set(0xF08, 1) || !write(0xF00, word))
        return std::unexpected(system_error::timeout);
    }
    if (!wait_set(0xF08, 1))
      return std::unexpected(system_error::timeout);
    if (!wait_set(0xF08, 1))
      return std::unexpected(system_error::timeout);
    sram_pending_ = false;
    return {};
  }
  [[nodiscard]] system_result
  rng_sram_read(unsigned offset, std::span<std::uint32_t> words) noexcept {
    auto check = available();
    if (!check)
      return check;
    if (offset % 4 || offset > 2048 || words.size() > (2048 - offset) / 4)
      return std::unexpected(system_error::invalid);
    sram_pending_ = true;
    if (!write(0xF04, offset) || !wait_set(0xF08, 1))
      return fail();
    auto discarded = read(0xF00);
    if (!discarded)
      return fail();
    for (auto &word : words) {
      if (!wait_set(0xF08, 1))
        return std::unexpected(system_error::timeout);
      auto v = read(0xF00);
      if (!v)
        return fail();
      word = *v;
    }
    if (!wait_set(0xF08, 1))
      return std::unexpected(system_error::timeout);
    sram_pending_ = false;
    return {};
  }
  [[nodiscard]] system_result recover() noexcept {
    if (s_.context.execution.in_isr())
      return std::unexpected(system_error::invalid);
    if (!s_.owned())
      return {};
    if (enable_attempted_) {
      if (!s_.context.write(enable_address, 1))
        return fail();
      for (auto off : clock_offsets)
        if (!write(off, 1))
          return fail();
      if (!write(0x1C4, 1))
        return fail();
    }
    // A reset request alone does not free buffers. Poll both DMA busy words,
    // all engines, the TRNG and private SRAM interface before returning RAM.
    for (auto off : std::array<unsigned, 8>{0xC44, 0xD58, 0xC58, 0x4F4, 0x7E4,
                                            0x3C0, 0x8C, 0x140})
      if (!write(off, 1))
        return fail();
    if (!write(0x1C4, 1) || !write(0x12C, 0))
      return fail();
    if (!s_.bounded([&] { return quiet(); }, timeout_, PollBudget))
      return std::unexpected(system_error::timeout);
    if (!s_.release_quiet(input_) || !s_.release_quiet(output_))
      return fail();
    // CPU staging can contain plaintext or a Poly1305 one-time key. Wipe only
    // after both engines have stopped accessing it, including failed DMA flows.
    wipe(mode_in_);
    wipe(mode_out_);
    wipe(empty_hash_block_);
    dma_pending_ = pka_pending_ = rng_active_ = sram_pending_ = fault_ = false;
    pka_bits_ = 0;
    if (!write(0x108, 0x3F) || !write(0xA08, 0x7F0))
      return fail();
    return {};
  }
  [[nodiscard]] system_result shutdown() noexcept {
    if (s_.context.execution.in_isr())
      return std::unexpected(system_error::invalid);
    if (!s_.owned())
      return {};
    if (enable_attempted_) {
      auto recovered = recover();
      if (!recovered)
        return recovered;
      // Wipe session material and engine SRAM without changing retained
      // KDR/LCS.
      for (unsigned i = 0; i < 8; ++i)
        if (!write(0x388 + 4 * i, 0) || !write(0x400 + 4 * i, 0))
          return fail();
      if (!write(0xE0, 1) || !write(0xD4, 0))
        return fail();
      for (unsigned i = 0; i < 1024; ++i)
        if (!write(0xD8, 0))
          return fail();
      if (!write(0xF04, 0))
        return fail();
      for (unsigned i = 0; i < 512; ++i)
        if (!wait_set(0xF08, 1) || !write(0xF00, 0))
          return fail();
      if (!wait_set(0xF08, 1))
        return fail();
      if (!write(0x100, 0x3F) || !write(0xA04, 0x7F0) || !write(0x108, 0x3F) ||
          !write(0xA08, 0x7F0))
        return fail();
      for (auto off : clock_offsets)
        if (!write(off, 0))
          return fail();
      if (!write(0x1C4, 0) || !s_.context.write(enable_address, 0))
        return fail();
      auto enabled = s_.context.read(enable_address);
      if (!enabled || *enabled)
        return fail();
    }
    auto r = s_.release();
    if (r)
      initialized_ = enable_attempted_ = fault_ = false;
    return r;
  }
  // RFC 3610 CCM and IEEE 802.15.4 CCM*: empty tag selects no authentication;
  // encrypt_payload=false selects authentication-only. Nonces are caller-owned
  // uniqueness domains. The CPU stages blocks in controller-owned RAM, so a DMA
  // timeout never retains a pointer into a returned stack frame.
  [[nodiscard]] system_result
  ccm(std::array<std::uint32_t, 4> key, std::span<const std::uint8_t> nonce,
      std::span<const std::uint8_t> aad, std::span<const std::uint8_t> input,
      std::span<std::uint8_t> output, std::span<std::uint8_t> tag,
      bool decrypt = false, bool encrypt_payload = true) noexcept {
    auto check = available();
    if (!check)
      return check;
    if (nonce.size() < 7 || nonce.size() > 13 ||
        input.size() != output.size() || input.size() > 65535 ||
        aad.size() > 65535 ||
        (!tag.empty() &&
         (tag.size() < 4 || tag.size() > 16 || tag.size() % 2)) ||
        partial_overlap(input, output) || overlaps(aad, output) ||
        overlaps(tag, input) || overlaps(tag, output))
      return std::unexpected(system_error::invalid);
    auto run = [&]() -> system_result {
      const unsigned L = 15 - nonce.size();
      cc_aes_configuration c{};
      c.key = key;
      std::array<std::uint8_t, 16> state{}, block{}, counter{}, mask{};
      counter[0] = L - 1;
      std::copy(nonce.begin(), nonce.end(), counter.begin() + 1);
      auto enc = [&](const auto &source, auto &destination) -> system_result {
        std::copy(source.begin(), source.end(), mode_in_.begin());
        auto r = aes(c, std::span{mode_in_}.first(16),
                     std::span{mode_out_}.first(16));
        if (r)
          std::copy_n(mode_out_.begin(), 16, destination.begin());
        return r;
      };
      auto authenticate = [&]() -> system_result {
        for (unsigned i = 0; i < 16; ++i)
          block[i] ^= state[i];
        auto r = enc(block, state);
        block = {};
        return r;
      };
      block = counter;
      block[0] |= (aad.empty() ? 0 : 64) |
                  (tag.empty() ? 0 : ((tag.size() - 2) / 2) << 3);
      for (unsigned i = 0; i < L; ++i)
        block[15 - i] = std::uint64_t(input.size()) >> (8 * i);
      auto r = authenticate();
      if (!r)
        return r;
      if (!aad.empty()) {
        unsigned pos = 0;
        if (aad.size() < 0xFF00) {
          block[0] = aad.size() >> 8;
          block[1] = aad.size();
          pos = 2;
        } else {
          block[0] = 255;
          block[1] = 254;
          block[4] = aad.size() >> 8;
          block[5] = aad.size();
          pos = 6;
        }
        for (auto b : aad) {
          block[pos++] = b;
          if (pos == 16) {
            r = authenticate();
            if (!r)
              return r;
            pos = 0;
          }
        }
        if (pos) {
          r = authenticate();
          if (!r)
            return r;
        }
      }
      for (std::size_t at = 0; at < input.size(); at += 16) {
        auto number = at / 16 + 1;
        for (unsigned i = 0; i < L; ++i)
          counter[15 - i] = std::uint64_t(number) >> (8 * i);
        r = enc(counter, mask);
        if (!r)
          return r;
        auto n = std::min(std::size_t{16}, input.size() - at);
        for (unsigned i = 0; i < n; ++i) {
          auto plain = std::uint8_t(input[at + i] ^
                                    (decrypt && encrypt_payload ? mask[i] : 0));
          block[i] = plain;
          output[at + i] = plain ^ (!decrypt && encrypt_payload ? mask[i] : 0);
        }
        r = authenticate();
        if (!r)
          return r;
      }
      for (unsigned i = 0; i < L; ++i)
        counter[15 - i] = 0;
      r = enc(counter, mask);
      if (!r)
        return r;
      unsigned difference = 0;
      for (unsigned i = 0; i < tag.size(); ++i) {
        auto expected = std::uint8_t(state[i] ^ mask[i]);
        if (decrypt)
          difference |= tag[i] ^ expected;
        else
          tag[i] = expected;
      }
      if (difference)
        return std::unexpected(system_error::faulted);
      return {};
    };
    auto result = run();
    if (!result)
      std::fill(output.begin(), output.end(), 0);
    return result;
  }
  // RFC 8439 AEAD. The Poly1305 authenticator uses fixed integer storage;
  // stream blocks are produced by CC_CHACHA. Decryption authenticates before
  // publishing any plaintext. This API uses a 256-bit key and 96-bit nonce,
  // counter 0/1.
  [[nodiscard]] system_result chacha_poly1305(
      cc_chacha_configuration config, std::span<const std::uint8_t> aad,
      std::span<const std::uint8_t> input, std::span<std::uint8_t> output,
      std::span<std::uint8_t, 16> tag, bool decrypt = false) noexcept {
    auto check = available();
    if (!check)
      return check;
    if (config.key128 || !config.nonce96 || input.size() != output.size() ||
        input.size() > 65535 || aad.size() > 65535 ||
        partial_overlap(input, output) || overlaps(aad, output) ||
        overlaps(tag, input) || overlaps(tag, output))
      return std::unexpected(system_error::invalid);
    config.counter = 0;
    config.poly1305_key = false;
    mode_in_ = {};
    auto r = chacha(config, mode_in_, mode_out_);
    if (!r)
      return r;
    std::array<std::uint8_t, 32> key{};
    std::copy_n(mode_out_.begin(), 32, key.begin());
    auto authenticate = [&](std::span<const std::uint8_t> ciphertext) {
      detail::poly1305 poly{key};
      constexpr std::array<std::uint8_t, 16> zero{};
      poly.update(aad);
      if (aad.size() % 16)
        poly.update(std::span{zero}.first(16 - aad.size() % 16));
      poly.update(ciphertext);
      if (ciphertext.size() % 16)
        poly.update(std::span{zero}.first(16 - ciphertext.size() % 16));
      std::array<std::uint8_t, 16> lengths{};
      for (unsigned i = 0; i < 8; ++i) {
        lengths[i] = std::uint64_t(aad.size()) >> (8 * i);
        lengths[i + 8] = std::uint64_t(ciphertext.size()) >> (8 * i);
      }
      poly.update(lengths);
      return poly.finish();
    };
    if (decrypt) {
      auto expected = authenticate(input);
      unsigned difference = 0;
      for (unsigned i = 0; i < 16; ++i)
        difference |= expected[i] ^ tag[i];
      if (difference)
        return std::unexpected(system_error::faulted);
    }
    for (std::size_t at = 0; at < input.size(); at += 64) {
      mode_in_ = {};
      auto n = std::min(std::size_t{64}, input.size() - at);
      std::copy_n(input.begin() + at, n, mode_in_.begin());
      config.counter = at / 64 + 1;
      r = chacha(config, mode_in_, mode_out_);
      if (!r) {
        std::fill(output.begin(), output.end(), 0);
        return r;
      }
      std::copy_n(mode_out_.begin(), n, output.begin() + at);
    }
    if (!decrypt) {
      auto expected = authenticate(output);
      std::copy(expected.begin(), expected.end(), tag.begin());
    }
    return {};
  }
  // RFC 2104 HMAC. Caller-owned, word-aligned workspace is DMA storage and must
  // survive recover() after failure. Capacity >= max(key length,64+message,96).
  [[nodiscard]] system_value<cc_digest>
  hmac(cc_hash_mode mode, std::span<const std::uint8_t> key,
       std::span<const std::uint8_t> message,
       std::span<std::uint8_t> workspace) noexcept {
    auto check = available();
    if (!check)
      return std::unexpected(check.error());
    if (key.size() > 65535 || message.size() > 65471 ||
        workspace.size() <
            std::max({key.size(), 64 + message.size(), std::size_t{96}}))
      return std::unexpected(system_error::invalid);
    std::array<std::uint8_t, 64> normalized{};
    if (key.size() > 64) {
      std::copy(key.begin(), key.end(), workspace.begin());
      auto digest = hash(mode, workspace.first(key.size()));
      if (!digest)
        return std::unexpected(digest.error());
      std::copy_n(digest->bytes.begin(), digest->size, normalized.begin());
    } else
      std::copy(key.begin(), key.end(), normalized.begin());
    for (unsigned i = 0; i < 64; ++i)
      workspace[i] = normalized[i] ^ 0x36;
    std::copy(message.begin(), message.end(), workspace.begin() + 64);
    auto inner = hash(mode, workspace.first(64 + message.size()));
    if (!inner)
      return std::unexpected(inner.error());
    for (unsigned i = 0; i < 64; ++i)
      workspace[i] = normalized[i] ^ 0x5C;
    std::copy_n(inner->bytes.begin(), inner->size, workspace.begin() + 64);
    auto result = hash(mode, workspace.first(64 + inner->size));
    if (result)
      std::fill(workspace.begin(), workspace.end(), 0);
    return result;
  }

  [[nodiscard]] system_result interrupt_delivery(bool on) noexcept {
    if (!initialized_)
      return std::unexpected(system_error::not_initialized);
    return write(0xA04, on ? 0 : 0x7F0) && write(0x100, on ? 0 : 0x3F)
               ? system_result{}
               : fail();
  }
  [[nodiscard]] system_value<std::uint32_t> interrupt_status() noexcept {
    auto value = read(0xA00);
    if (!value)
      return std::unexpected(system_error::mmio);
    return *value & 0x7F0;
  }
  [[nodiscard]] bool pending() const noexcept {
    return dma_pending_ || pka_pending_ || rng_active_ || sram_pending_;
  }

private:
  static bool overlaps(std::span<const std::uint8_t> a,
                       std::span<const std::uint8_t> b) {
    if (a.empty() || b.empty())
      return false;
    auto first = reinterpret_cast<std::uintptr_t>(a.data()),
         second = reinterpret_cast<std::uintptr_t>(b.data());
    return first < second + b.size() && second < first + a.size();
  }
  static bool partial_overlap(std::span<const std::uint8_t> a,
                              std::span<const std::uint8_t> b) {
    return a.data() != b.data() && overlaps(a, b);
  }
  static void wipe(std::span<std::uint8_t> storage) {
    volatile std::uint8_t *p = storage.data();
    for (std::size_t i = 0; i < storage.size(); ++i)
      p[i] = 0;
  }
  auto read(unsigned off) { return s_.context.read(base + off); }
  bool write(unsigned off, std::uint32_t word) {
    auto r = s_.context.write(base + off, word);
    arch::data_synchronization_barrier();
    return r.has_value();
  }
  bool wait_zero(unsigned off) {
    return s_.bounded(
        [&] {
          auto r = read(off);
          return r && !*r;
        },
        timeout_, PollBudget);
  }
  bool wait_set(unsigned off, std::uint32_t mask) {
    return s_.bounded(
        [&] {
          auto r = read(off);
          return r && (*r & mask) == mask;
        },
        timeout_, PollBudget);
  }
  system_result available() {
    if (!initialized_ || fault_ || pending() || s_.context.execution.in_isr())
      return std::unexpected(system_error::busy);
    return {};
  }
  bool quiet() {
    for (auto off : std::array<unsigned, 9>{0xC20, 0xC38, 0xD20, 0xD38, 0x910,
                                            0x91C, 0x3B0, 0x1B8, 0x1DC}) {
      auto r = read(off);
      if (!r || *r)
        return false;
    }
    auto p = read(0xB4), s = read(0xF08);
    return p && (*p & 1) && s && (*s & 1);
  }
  bool configure_aes(cc_aes_configuration c) {
    if (!write(0x4F4, 1) || !write(0x900, 1) || !write(0xA38, 2) ||
        !write(0x4C0, (unsigned(c.mode) << 2) | (c.decrypt ? 1U : 0U)) ||
        !write(0x4D8, 0))
      return false;
    for (unsigned i = 0; i < 4; ++i)
      if (!write(0x400 + 4 * i, c.key[i]))
        return false;
    for (unsigned i = 0; i < 4; ++i)
      if (!write((c.mode == cc_aes_mode::ctr ? 0x460 : 0x440) + 4 * i, c.iv[i]))
        return false;
    return true;
  }
  system_result dma(std::span<const std::uint8_t> input,
                    std::span<std::uint8_t> output, unsigned events) {
    if (input.empty() || input.size() > 65535 ||
        (!output.empty() && output.size() != input.size()))
      return std::unexpected(system_error::invalid);
    auto tx = s_.prepare(input.data(), input.size(), input.size(),
                         {65535, 1, 4}, dma_access::read);
    if (!tx)
      return std::unexpected(system_error::invalid);
    input_ = *tx;
    if (!output.empty()) {
      auto rx = s_.prepare(output.data(), output.size(), output.size(),
                           {65535, 1, 4}, dma_access::write);
      if (!rx) {
        s_.discard(input_);
        return std::unexpected(system_error::invalid);
      }
      output_ = *rx;
    }
    if (!write(0xA08, 0x1F0) ||
        !write(0xC28, s_.address(input.data(), input.size())) ||
        (!output.empty() &&
         !write(0xD28, s_.address(output.data(), output.size())))) {
      s_.discard(input_);
      s_.discard(output_);
      return fail();
    }
    if (!s_.buffers.arm(*input_) || (output_ && !s_.buffers.arm(*output_)))
      return fail();
    dma_pending_ = true;
    // DOUT size enables a destination and can start writes; both leases are
    // already armed before this first potentially committing register write.
    if ((!output.empty() && !write(0xD2C, output.size())) ||
        !write(0xC2C, input.size()))
      return fail();
    bool ahb_error = false;
    bool done = s_.bounded(
        [&] {
          auto irq = read(0xA00);
          if (!irq)
            return false;
          if (*irq & 256) {
            ahb_error = true;
            return true;
          }
          if ((*irq & events) != events)
            return false;
          auto in = read(0xC20), out = read(0xD20), core = read(0x910),
               hash = read(0x91C), chacha = read(0x3B0);
          return in && !*in && out && !*out && core && !*core && hash &&
                 !*hash && chacha && !*chacha;
        },
        timeout_, PollBudget);
    if (!done)
      return std::unexpected(system_error::timeout);
    if (ahb_error) {
      fault_ = true;
      return std::unexpected(system_error::faulted);
    }
    if (!s_.release_quiet(input_) || !s_.release_quiet(output_))
      return fail();
    dma_pending_ = false;
    if (!write(0xA08, events))
      return fail();
    return {};
  }
  static unsigned pka_slot(unsigned reg) {
    for (unsigned i = 0; i < 7; ++i)
      if (pka_registers[i] == reg)
        return i;
    return 7;
  }
  static bool valid_pka_opcode(unsigned op) {
    return (op >= 4 && op <= 10) || (op >= 12 && op <= 25) || op == 27;
  }
  system_result fail() {
    fault_ = true;
    return std::unexpected(system_error::mmio);
  }
  static constexpr std::uintptr_t base = 0x5002B000,
                                  enable_address = 0x5002A500;
  static constexpr std::array<unsigned, 5> clock_offsets{0x810, 0x818, 0x81C,
                                                         0x820, 0x858};
  static constexpr std::array<unsigned, 7> pka_registers{0, 1, 2, 3, 4, 30, 31};
  session s_;
  execution::tick timeout_;
  std::optional<dma_buffer_token> input_, output_;
  alignas(4) std::array<std::uint8_t, 64> empty_hash_block_{};
  alignas(4) std::array<std::uint8_t, 64> mode_in_{}, mode_out_{};
  unsigned pka_bits_{}, pka_stride_{};
  bool initialized_{}, enable_attempted_{}, fault_{}, dma_pending_{},
      pka_pending_{}, rng_active_{}, sram_pending_{};
};
} // namespace mcubsp::backend::nrf52840
