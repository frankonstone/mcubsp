#include "crypto_reference.hpp"
#include "fake_analog.hpp"
#include <cstring>
#include <mcubsp/backend/nrf52840/aar.hpp>
#include <mcubsp/backend/nrf52840/ccm.hpp>
#include <mcubsp/backend/nrf52840/cryptocell.hpp>
#include <mcubsp/backend/nrf52840/ecb.hpp>
#include <mcutest/mcutest.hpp>
namespace {
using namespace mcubsp;
using namespace backend::nrf52840;
class fake_crypto;
struct dma_address {
  fake_crypto *model;
  std::uintptr_t operator()(const void *, std::size_t) const;
};
class fake_crypto : public nrf52840_test::fake_analog {
public:
  bool responds{true}, hold_busy{}, ahb_error{};
  unsigned trng_faults{}, trng_starts{};
  std::optional<std::uintptr_t> fail_write;
  struct mapping {
    const void *ptr{};
    std::size_t bytes{};
  };
  std::array<mapping, 32> mappings{};
  unsigned mapped{};
  std::array<std::uint32_t, 1024> pka_sram{};
  std::array<std::uint32_t, 512> rng_sram{};
  unsigned pka_write{}, pka_read{}, rng_address{};
  std::uint32_t previous_rng_read{};
  const void *pointer(std::uintptr_t a) {
    auto i = (a - 0x20000000) / 0x1000;
    return i < mapped ? mappings[i].ptr : nullptr;
  }
  std::uintptr_t address(const void *p, std::size_t n) {
    for (unsigned i = 0; i < mapped; ++i)
      if (mappings[i].ptr == p)
        return 0x20000000 + 0x1000 * i;
    mappings[mapped] = {p, n};
    return 0x20000000 + 0x1000 * mapped++;
  }
  template <class T>
  std::expected<T, core::mmio_error> read(std::uintptr_t a) noexcept {
    if (a == 0x5002B0DC) {
      auto v = pka_sram[pka_read++];
      return T(v);
    }
    if (a == 0x5002BF00) {
      auto v = previous_rng_read;
      previous_rng_read = rng_sram[rng_address];
      if (rng_address < 511)
        ++rng_address;
      return T(v);
    }
    return fake_analog::read<T>(a);
  }
  template <class T>
  std::expected<void, core::mmio_error> write(std::uintptr_t a, T v) noexcept {
    auto r = fake_analog::write<T>(a, v);
    if (!r)
      return r;
    if (a == 0x4000E000 && v && responds) {
      auto *block = const_cast<ecb_data *>(
          static_cast<const ecb_data *>(pointer(peek(0x4000E504))));
      if (block)
        block->ciphertext =
            nrf52840_test::aes_encrypt(block->key, block->cleartext);
      poke(0x4000E100, 1);
    }
    if (a == 0x4000E004 && v && responds)
      poke(0x4000E104, 1);
    if (a == 0x4000F000 && v && responds) {
      if (peek(0x4000F500) == 3) {
        poke(0x4000F100, 1);
        poke(0x4000F104, 1);
        poke(0x4000F400, 1);
      } else {
        poke(0x4000F100, 1);
        if (peek(0x4000F200))
          poke(0x4000F104, 1);
      }
    }
    if (a == 0x4000F004 && v && responds)
      poke(0x4000F104, 1);
    if (a == 0x5002BA08)
      poke(0x5002BA00, peek(0x5002BA00) & ~v);
    if (a == 0x5002B108)
      poke(0x5002B104, peek(0x5002B104) & ~v);
    if (a == 0x5002B0D4)
      pka_write = v;
    if (a == 0x5002B0D8 && pka_write < 1024)
      pka_sram[pka_write++] = v;
    if (a == 0x5002B0E4)
      pka_read = v;
    if (a == 0x5002BF04)
      rng_address = v / 4;
    if (a == 0x5002BF00 && rng_address < 512) {
      rng_sram[rng_address] = v;
      if (rng_address < 511)
        ++rng_address;
    }
    if (a == 0x5002B0B4)
      poke(a, 1);
    if (a == 0x5002B08C) {
      poke(0x5002B0B4, hold_busy ? 0 : 1);
      poke(0x5002B0B0, 1);
    }
    if (a == 0x5002B080 && responds) {
      auto op = v >> 27;
      auto ai = (v >> 18) & 31, bi = (v >> 12) & 31, ri = (v >> 6) & 31;
      auto A = pka_sram[peek(0x5002B000 + 4 * ai)],
           B = pka_sram[peek(0x5002B000 + 4 * bi)];
      auto value = op == 4 ? A + B : op == 5 ? A - B : op == 16 ? A * B : 0;
      pka_sram[peek(0x5002B000 + 4 * ri)] = value;
      poke(0x5002B0B4, 1);
      poke(0x5002B088, op << 16);
    }
    if (a == 0x5002BC44 || a == 0x5002BD58) {
      if (!hold_busy) {
        poke(0x5002BC20, 0);
        poke(0x5002BD20, 0);
      }
    }
    if (a == 0x5002BC2C && responds) {
      if (hold_busy) {
        poke(0x5002BC20, 1);
        poke(0x5002BD20, 1);
      } else {
        run_dma(v);
        poke(0x5002BA00, ahb_error ? 256 : 192);
      }
    }
    if (a == 0x5002B12C && v && responds) {
      ++trng_starts;
      poke(0x5002B104, trng_faults ? 3 : 1);
      if (trng_faults)
        --trng_faults;
      for (unsigned i = 0; i < 6; ++i)
        poke(0x5002B114 + 4 * i, 0x12340000 + i);
    }
    if (fail_write == a) {
      fail_write.reset();
      return std::unexpected(core::mmio_error::invalid_value);
    }
    return {};
  }
  void run_dma(unsigned count) {
    auto *src = static_cast<const std::uint8_t *>(pointer(peek(0x5002BC28)));
    auto *dst = const_cast<std::uint8_t *>(
        static_cast<const std::uint8_t *>(pointer(peek(0x5002BD28))));
    auto mode = peek(0x5002B900);
    if (mode == 0 && src && dst)
      std::memcpy(dst, src, count);
    if ((mode == 1 || mode == 9) && src) {
      std::array<std::uint8_t, 16> key{}, iv{}, in{};
      for (unsigned i = 0; i < 16; ++i) {
        key[i] = peek(0x5002B400 + 4 * (i / 4)) >> (8 * (i % 4));
        iv[i] = peek((unsigned(peek(0x5002B4C0) >> 2) == 2 ? 0x5002B460
                                                           : 0x5002B440) +
                     4 * (i / 4)) >>
                (8 * (i % 4));
      }
      auto aesmode = (peek(0x5002B4C0) >> 2) & 15;
      auto double_block = [](auto value) {
        bool carry = value[0] & 128;
        for (unsigned i = 0; i < 16; ++i)
          value[i] = (value[i] << 1) | (i < 15 ? value[i + 1] >> 7 : 0);
        if (carry)
          value[15] ^= 0x87;
        return value;
      };
      for (unsigned p = 0; p < count; p += 16) {
        in = {};
        auto n = std::min(16U, count - p);
        std::copy_n(src + p, n, in.begin());
        if (aesmode == 2) {
          auto stream = nrf52840_test::aes_encrypt(key, iv);
          for (unsigned j = 0; j < n; ++j)
            dst[p + j] = in[j] ^ stream[j];
          for (int j = 15; j >= 0; --j)
            if (++iv[j])
              break;
        } else {
          if (aesmode == 7 && p + n == count) {
            auto k1 = double_block(nrf52840_test::aes_encrypt(key, {}));
            if (n < 16) {
              k1 = double_block(k1);
              in[n] = 128;
            }
            for (unsigned j = 0; j < 16; ++j)
              in[j] ^= k1[j];
          }
          if (aesmode)
            for (unsigned j = 0; j < 16; ++j)
              in[j] ^= iv[j];
          iv = nrf52840_test::aes_encrypt(key, in);
          if (dst)
            std::copy_n(iv.begin(), n, dst + p);
        }
      }
      if (mode == 9)
        for (unsigned i = 0; i < 4; ++i)
          poke(0x5002B440 + 4 * i, unsigned(iv[4 * i]) |
                                       unsigned(iv[4 * i + 1]) << 8 |
                                       unsigned(iv[4 * i + 2]) << 16 |
                                       unsigned(iv[4 * i + 3]) << 24);
    }
    if (mode == 16 && src && dst) {
      std::array<std::uint32_t, 16> state{0x61707865, 0x3320646e, 0x79622d32,
                                          0x6b206574};
      for (unsigned i = 0; i < 8; ++i)
        state[i + 4] = peek(0x5002B388 + 4 * i);
      state[12] = peek(0x5002B3B8);
      state[13] = peek(0x5002B3BC);
      state[14] = peek(0x5002B3A8);
      state[15] = peek(0x5002B3AC);
      // CC NONCE[2] shares COUNT_HIGH; 96-bit mode orders it before NONCE[0:1].
      for (unsigned p = 0; p < count; p += 64) {
        auto out = nrf52840_test::chacha_block(state);
        for (unsigned j = 0; j < std::min(64U, count - p); ++j)
          dst[p + j] = src[p + j] ^ out[j];
        ++state[12];
      }
    }
    if (mode == 7 && src) {
      auto digest = nrf52840_test::sha({src, peek(0x5002B7C4) ? count : 0},
                                       peek(0x5002B7C0));
      for (unsigned i = 0; i < 8; ++i)
        poke(0x5002B640 + 4 * i, digest[i]);
    }
  }
};
std::uintptr_t dma_address::operator()(const void *p, std::size_t n) const {
  return model->address(p, n);
}
struct fixture {
  fake_crypto mmio;
  execution::fake_policy execution;
  resource_registry resources;
  gpio_backend<fake_crypto> gpio{mmio, resources};
  system_context<fake_crypto, execution::fake_policy> context{mmio, execution,
                                                              gpio};
  clock_controller<decltype(context)> clocks{context};
  dma_buffer_pool<execution::fake_policy, 16> buffers{resources, execution};
  ecb_controller<decltype(context), decltype(buffers), dma_address, 64> ecb{
      context, clocks, buffers, 100, {&mmio}};
  aar_controller<decltype(context), decltype(buffers), dma_address> aar{
      context, clocks, buffers, {&mmio}};
  ccm_controller<decltype(context), decltype(buffers), dma_address> ccm{
      context, clocks, buffers, {&mmio}};
  cryptocell_controller<decltype(context), decltype(buffers), dma_address, 64>
      cc{context, clocks, buffers, 100, {&mmio}};
  fixture() {
    mmio.execution = &execution;
    mmio.poke(0x5002BA24, 0x20E00000);
    mmio.poke(0x5002B0B4, 1);
    mmio.poke(0x5002B0B0, 1);
    mmio.poke(0x5002BF08, 1);
    for (unsigned i = 0; i < 4; ++i)
      mmio.poke(0x10000C10 + 4 * i, 100 + i);
  }
  ~fixture() {
    mmio.hold_busy = false;
    mmio.responds = true;
    (void)ecb.shutdown();
    (void)aar.shutdown();
    (void)ccm.shutdown();
    (void)cc.shutdown();
    (void)clocks.shutdown();
  }
};
constexpr std::array<std::uint8_t, 16> cipher{
    0x69, 0xc4, 0xe0, 0xd8, 0x6a, 0x7b, 0x04, 0x30,
    0xd8, 0xcd, 0xb7, 0x80, 0x70, 0xb4, 0xc5, 0x5a};
TEST(nrf_crypto,
     ecb_nist_known_answer_and_abort_retains_lease_until_error_event,
     "REQ-0071", "REQ-0062") {
  fixture f;
  ASSERT_TRUE(f.clocks.initialize().has_value());
  ASSERT_TRUE(f.ecb.initialize().has_value());
  ecb_data block{};
  for (unsigned i = 0; i < 16; ++i) {
    block.key[i] = i;
    block.cleartext[i] = i * 17;
  }
  ASSERT_TRUE(f.ecb.start(block).has_value());
  auto done = f.ecb.poll();
  ASSERT_TRUE(done.has_value());
  EXPECT_TRUE(*done);
  EXPECT_TRUE(block.ciphertext == cipher);
  f.mmio.responds = false;
  f.mmio.fail_write = 0x4000E000;
  EXPECT_FALSE(f.ecb.start(block).has_value());
  EXPECT_FALSE(f.ecb.cancel().has_value());
  EXPECT_TRUE(f.ecb.active());
  f.mmio.poke(0x4000E104, 1);
  ASSERT_TRUE(f.ecb.cancel().has_value());
  EXPECT_FALSE(f.ecb.active());
}
TEST(nrf_crypto, aar_bounds_alias_exclusion_and_resolved_index, "REQ-0071",
     "REQ-0062") {
  fixture f;
  ASSERT_TRUE(f.clocks.initialize().has_value());
  ASSERT_TRUE(f.aar.initialize().has_value());
  EXPECT_FALSE(f.ccm.initialize().has_value());
  std::array<identity_resolving_key, 2> keys{};
  aar_packet packet{};
  packet.address[5] = 0x40;
  std::array<std::uint8_t, 3> scratch{};
  EXPECT_FALSE(f.aar.resolve({}, packet, scratch).has_value());
  ASSERT_TRUE(f.aar.resolve(keys, packet, scratch).has_value());
  auto result = f.aar.poll();
  ASSERT_TRUE(result && result->has_value());
  EXPECT_TRUE((**result).resolved);
  EXPECT_EQ((**result).key_index, 1U);
  ASSERT_TRUE(f.aar.shutdown().has_value());
  ASSERT_TRUE(f.ccm.initialize().has_value());
}
TEST(nrf_crypto, ccm_nonce_layout_mic_and_manual_keystream_handoff, "REQ-0071",
     "REQ-0062") {
  fixture f;
  ASSERT_TRUE(f.clocks.initialize().has_value());
  ASSERT_TRUE(f.ccm.initialize().has_value());
  ccm_nonce_configuration nonce{};
  std::array<std::uint8_t, 8> packet{0, 5, 0, 1, 2, 3, 4, 5};
  std::array<std::uint8_t, 16> output{};
  std::array<std::uint8_t, 43> scratch{};
  ASSERT_TRUE(f.ccm
                  .start(nonce, packet, output, scratch,
                         {.decrypt = true, .automatic_crypt = false})
                  .has_value());
  auto pending = f.ccm.poll();
  ASSERT_TRUE(pending.has_value());
  EXPECT_FALSE(pending->has_value());
  EXPECT_TRUE(f.ccm.key_ready());
  ASSERT_TRUE(f.ccm.override_rate(ccm_data_rate::mbit2).has_value());
  ASSERT_TRUE(f.ccm.crypt().has_value());
  auto done = f.ccm.poll();
  ASSERT_TRUE(done && done->has_value());
  EXPECT_FALSE((**done).mic_valid);
  EXPECT_EQ((**done).bytes, 4U);
  nonce.direction = 2;
  EXPECT_FALSE(f.ccm.start(nonce, packet, output, scratch).has_value());
}
TEST(nrf_crypto, cryptocell_aes_nist_key_order_and_real_dma_limits, "REQ-0071",
     "REQ-0062") {
  fixture f;
  ASSERT_TRUE(f.clocks.initialize().has_value());
  ASSERT_TRUE(f.cc.initialize().has_value());
  cc_aes_configuration config{};
  for (unsigned i = 0; i < 4; ++i)
    config.key[i] = (4 * i) | ((4 * i + 1) << 8) | ((4 * i + 2) << 16) |
                    ((4 * i + 3) << 24);
  alignas(4) std::array<std::uint8_t, 16> input{}, output{};
  for (unsigned i = 0; i < 16; ++i)
    input[i] = i * 17;
  ASSERT_TRUE(f.cc.aes(config, input, output).has_value());
  EXPECT_TRUE(output == cipher);
  EXPECT_FALSE(
      f.cc.aes(config,
               {reinterpret_cast<const std::uint8_t *>(0x20000000), 65536},
               {reinterpret_cast<std::uint8_t *>(0x20010000), 65536})
          .has_value());
  EXPECT_EQ(f.mmio.peek(0x5002BA38), 2U);
  EXPECT_EQ(f.mmio.peek(0x5002BC2C), 16U);
}
TEST(nrf_crypto, cryptocell_hash_uses_descending_initial_state_and_fips_digest,
     "REQ-0071") {
  fixture f;
  ASSERT_TRUE(f.clocks.initialize().has_value());
  ASSERT_TRUE(f.cc.initialize().has_value());
  alignas(4) std::array<std::uint8_t, 3> input{'a', 'b', 'c'};
  auto digest = f.cc.hash(cc_hash_mode::sha256, input);
  ASSERT_TRUE(digest.has_value());
  EXPECT_EQ(digest->size, 32U);
  EXPECT_EQ(digest->bytes[0], 0xbaU);
  EXPECT_EQ(digest->bytes[31], 0xadU);
  unsigned next = 7;
  for (unsigned i = 0; i < f.mmio.trace_count; ++i) {
    auto &w = f.mmio.trace[i];
    if (w.address >= 0x5002B640 && w.address <= 0x5002B65C) {
      EXPECT_EQ(w.address, 0x5002B640 + 4 * next);
      if (next)
        --next;
    }
  }
}
TEST(nrf_crypto,
     cryptocell_timeout_requires_reset_and_busy_proof_before_dma_release,
     "REQ-0071", "REQ-0062") {
  fixture f;
  ASSERT_TRUE(f.clocks.initialize().has_value());
  ASSERT_TRUE(f.cc.initialize().has_value());
  alignas(4) std::array<std::uint8_t, 16> input{}, output{};
  f.mmio.hold_busy = true;
  EXPECT_FALSE(f.cc.copy(input, output).has_value());
  EXPECT_TRUE(f.cc.pending());
  EXPECT_FALSE(f.cc.recover().has_value());
  f.mmio.hold_busy = false;
  ASSERT_TRUE(f.cc.recover().has_value());
  EXPECT_FALSE(f.cc.pending());
  ASSERT_TRUE(f.cc.copy(input, output).has_value());
  f.mmio.ahb_error = true;
  EXPECT_FALSE(f.cc.copy(input, output).has_value());
  EXPECT_TRUE(f.cc.pending());
  ASSERT_TRUE(f.cc.recover().has_value());
}
TEST(nrf_crypto, cryptocell_pka_maps_padding_executes_arithmetic_and_reads_sram,
     "REQ-0071") {
  fixture f;
  ASSERT_TRUE(f.clocks.initialize().has_value());
  ASSERT_TRUE(f.cc.initialize().has_value());
  ASSERT_TRUE(f.cc.pka_configure(2048).has_value());
  std::array<std::uint32_t, 1> a{5}, b{2}, result{};
  ASSERT_TRUE(f.cc.pka_load(2, a).has_value());
  ASSERT_TRUE(f.cc.pka_load(3, b).has_value());
  auto status =
      f.cc.pka_execute({mcu::nrf52840::product::cc_pka::opcode_fields::
                            opcode_encoding::value_subdecneg});
  ASSERT_TRUE(status.has_value());
  ASSERT_TRUE(f.cc.pka_read(4, result).has_value());
  EXPECT_EQ(result[0], 3U);
  EXPECT_FALSE(f.cc.pka_configure(3137).has_value());
  EXPECT_FALSE(f.cc.pka_read(5, result).has_value());
}
TEST(nrf_crypto,
     cryptocell_entropy_retries_oscillators_and_rejects_health_failures,
     "REQ-0071") {
  fixture f;
  ASSERT_TRUE(f.clocks.initialize().has_value());
  ASSERT_TRUE(f.cc.initialize().has_value());
  f.mmio.trng_faults = 2;
  auto entropy = f.cc.entropy();
  ASSERT_TRUE(entropy.has_value());
  EXPECT_EQ(f.mmio.trng_starts, 3U);
  EXPECT_EQ((*entropy)[5], 0x12340005U);
  EXPECT_EQ(f.mmio.peek(0x5002B130), 102U);
  f.mmio.trng_faults = 4;
  EXPECT_FALSE(f.cc.entropy().has_value());
  EXPECT_EQ(f.mmio.peek(0x5002B12C), 0U);
}
TEST(
    nrf_crypto,
    cryptocell_rng_sram_discards_pipeline_read_and_shutdown_preserves_root_state,
    "REQ-0071") {
  fixture f;
  ASSERT_TRUE(f.clocks.initialize().has_value());
  ASSERT_TRUE(f.cc.initialize().has_value());
  std::array<std::uint32_t, 3> input{1, 2, 3}, output{};
  ASSERT_TRUE(f.cc.rng_sram_write(4, input).has_value());
  ASSERT_TRUE(f.cc.rng_sram_read(4, output).has_value());
  EXPECT_TRUE(input == output);
  EXPECT_FALSE(f.cc.rng_sram_write(2044, input).has_value());
  ASSERT_TRUE(f.cc.shutdown().has_value());
  EXPECT_EQ(f.mmio.peek(0x5002A500), 0U);
  EXPECT_FALSE(f.mmio.wrote(0x5002BA60, 0));
  for (unsigned i = 0; i < 512; ++i)
    EXPECT_EQ(f.mmio.rng_sram[i], 0U);
}

template <std::size_t N> constexpr auto hex(const char (&text)[N]) {
  std::array<std::uint8_t, (N - 1) / 2> bytes{};
  auto digit = [](char c) { return c <= '9' ? c - '0' : c - 'a' + 10; };
  for (unsigned i = 0; i < bytes.size(); ++i)
    bytes[i] = 16 * digit(text[2 * i]) + digit(text[2 * i + 1]);
  return bytes;
}
template <std::size_t N> auto words(const std::array<std::uint8_t, N> &bytes) {
  std::array<std::uint32_t, N / 4> result{};
  for (unsigned i = 0; i < N; ++i)
    result[i / 4] |= unsigned(bytes[i]) << (8 * (i % 4));
  return result;
}
TEST(nrf_crypto, cryptocell_cbc_ctr_and_cmac_nist_vectors, "REQ-0071") {
  fixture f;
  ASSERT_TRUE(f.clocks.initialize().has_value());
  ASSERT_TRUE(f.cc.initialize().has_value());
  cc_aes_configuration c{};
  c.key = words(hex("2b7e151628aed2a6abf7158809cf4f3c"));
  alignas(4) auto input =
      hex("6bc1bee22e409f96e93d7e117393172aae2d8a571e03ac9c9eb76fac45af8e51");
  alignas(4) std::array<std::uint8_t, 32> output{};
  c.mode = cc_aes_mode::cbc;
  c.iv = words(hex("000102030405060708090a0b0c0d0e0f"));
  ASSERT_TRUE(f.cc.aes(c, input, output).has_value());
  EXPECT_TRUE(
      output ==
      hex("7649abac8119b246cee98e9b12e9197d5086cb9b507219ee95db113a917678b2"));
  c.mode = cc_aes_mode::ctr;
  c.iv = words(hex("f0f1f2f3f4f5f6f7f8f9fafbfcfdfeff"));
  ASSERT_TRUE(f.cc.aes(c, input, output).has_value());
  EXPECT_TRUE(
      output ==
      hex("874d6191b620e3261bef6864990db6ce9806f66b7970fdff8617187bb9fffdff"));
  c.mode = cc_aes_mode::cmac;
  c.iv = {};
  auto mac = f.cc.mac(c, std::span{input}.first(16));
  ASSERT_TRUE(mac);
  EXPECT_TRUE(*mac == hex("070a16b46b4d4144f79bdd9dd04a287c"));
  c.mode = cc_aes_mode::cbc_mac;
  auto raw = f.cc.mac(c, std::span{input}.first(16));
  ASSERT_TRUE(raw);
  EXPECT_TRUE(*raw == hex("3ad77bb40d7a3660a89ecaf32466ef97"));
}
TEST(nrf_crypto, cryptocell_ccm_rfc3610_and_authentication_failure,
     "REQ-0071") {
  fixture f;
  ASSERT_TRUE(f.clocks.initialize().has_value());
  ASSERT_TRUE(f.cc.initialize().has_value());
  auto key = words(hex("c0c1c2c3c4c5c6c7c8c9cacbcccdcecf"));
  auto nonce = hex("00000003020100a0a1a2a3a4a5");
  auto aad = hex("0001020304050607");
  auto input = hex("08090a0b0c0d0e0f101112131415161718191a1b1c1d1e");
  std::array<std::uint8_t, 23> output{}, plain{};
  std::array<std::uint8_t, 8> tag{};
  ASSERT_TRUE(f.cc.ccm(key, nonce, aad, input, output, tag).has_value());
  EXPECT_TRUE(output == hex("588c979a61c663d2f066d0c2c0f989806d5f6b61dac384"));
  EXPECT_TRUE(tag == hex("17e8d12cfdf926e0"));
  ASSERT_TRUE(f.cc.ccm(key, nonce, aad, output, plain, tag, true).has_value());
  EXPECT_TRUE(plain == input);
  tag[0] ^= 1;
  EXPECT_FALSE(f.cc.ccm(key, nonce, aad, output, plain, tag, true).has_value());
  EXPECT_TRUE((plain == std::array<std::uint8_t, 23>{}));
}
TEST(nrf_crypto, poly1305_rfc8439_and_chacha_aead_vector, "REQ-0071") {
  auto key =
      hex("85d6be7857556d337f4452fe42d506a80103808afb0db2fd4abff6af4149f51b");
  detail::poly1305 poly{key};
  const char text[] = "Cryptographic Forum Research Group";
  poly.update({reinterpret_cast<const std::uint8_t *>(text), sizeof(text) - 1});
  EXPECT_TRUE(poly.finish() == hex("a8061dc1305136c6c22b8baf0c0127a9"));
  fixture f;
  ASSERT_TRUE(f.clocks.initialize().has_value());
  ASSERT_TRUE(f.cc.initialize().has_value());
  cc_chacha_configuration c{};
  c.key = words(
      hex("808182838485868788898a8b8c8d8e8f909192939495969798999a9b9c9d9e9f"));
  c.nonce = words(hex("070000004041424344454647"));
  const char message[] =
      "Ladies and Gentlemen of the class of '99: If I could offer you only one "
      "tip for the future, sunscreen would be it.";
  auto input = std::span{reinterpret_cast<const std::uint8_t *>(message),
                         sizeof(message) - 1};
  std::array<std::uint8_t, 114> output{}, plain{};
  std::array<std::uint8_t, 16> tag{};
  auto aad = hex("50515253c0c1c2c3c4c5c6c7");
  ASSERT_TRUE(f.cc.chacha_poly1305(c, aad, input, output, tag).has_value());
  EXPECT_TRUE(output ==
              hex("d31a8d34648e60db7b86afbc53ef7ec2a4aded51296e08fea9e2b5a736ee"
                  "62d63dbea45e8ca9671282fafb69da92728b1a71de0a9e060b2905d6a5b6"
                  "7ecd3b3692ddbd7f2d778b8c9803aee328091b58fab324e4fad675945585"
                  "808b4831d7bc3ff4def08e4b7a9de576d26586cec64b6116"));
  EXPECT_TRUE(tag == hex("1ae10b594f09e26a7e902ecbd0600691"));
  ASSERT_TRUE(
      f.cc.chacha_poly1305(c, aad, output, plain, tag, true).has_value());
  EXPECT_TRUE(std::equal(input.begin(), input.end(), plain.begin()));
  tag[0] ^= 1;
  plain.fill(0xAA);
  EXPECT_FALSE(
      f.cc.chacha_poly1305(c, aad, output, plain, tag, true).has_value());
  EXPECT_EQ(plain[0], 0xAAU);
}
TEST(nrf_crypto, sha_variants_empty_hash_and_hmac_rfc4231, "REQ-0071") {
  fixture f;
  ASSERT_TRUE(f.clocks.initialize().has_value());
  ASSERT_TRUE(f.cc.initialize().has_value());
  alignas(4) auto input = hex("616263");
  auto sha1 = f.cc.hash(cc_hash_mode::sha1, input);
  ASSERT_TRUE(sha1);
  EXPECT_TRUE(
      std::equal(sha1->bytes.begin(), sha1->bytes.begin() + 20,
                 hex("a9993e364706816aba3e25717850c26c9cd0d89d").begin()));
  auto sha224 = f.cc.hash(cc_hash_mode::sha224, input);
  ASSERT_TRUE(sha224);
  EXPECT_TRUE(std::equal(
      sha224->bytes.begin(), sha224->bytes.begin() + 28,
      hex("23097d223405d8228642a477bda255b32aadbce4bda0b3f7e36c9da7").begin()));
  auto empty = f.cc.hash(cc_hash_mode::sha256, {});
  ASSERT_TRUE(empty);
  EXPECT_TRUE(
      empty->bytes ==
      hex("e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855"));
  auto key = hex("0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b");
  auto message = hex("4869205468657265");
  alignas(4) std::array<std::uint8_t, 96> work{};
  auto hmac = f.cc.hmac(cc_hash_mode::sha256, key, message, work);
  ASSERT_TRUE(hmac);
  EXPECT_TRUE(
      hmac->bytes ==
      hex("b0344c61d8db38535ca8afceaf0bf12b881dc200c9833da726e9376c2e32cff7"));
}

TEST(nrf_crypto, staged_aead_storage_is_wiped_after_dma_quiesces, "REQ-0071",
     "REQ-0062") {
  fixture f;
  ASSERT_TRUE(f.clocks.initialize().has_value());
  ASSERT_TRUE(f.cc.initialize().has_value());
  std::array<std::uint8_t, 16> tag{};
  std::array<std::uint8_t, 4> input{}, output{};
  ASSERT_TRUE(f.cc.chacha_poly1305({}, {}, input, output, tag).has_value());
  auto *staged = static_cast<const std::uint8_t *>(
      f.mmio.pointer(f.mmio.peek(0x5002BD28)));
  ASSERT_TRUE(staged);
  bool any = false;
  for (unsigned i = 0; i < 64; ++i)
    any = any || staged[i];
  EXPECT_TRUE(any);
  ASSERT_TRUE(f.cc.shutdown().has_value());
  for (unsigned i = 0; i < 64; ++i)
    EXPECT_EQ(staged[i], 0U);
}
TEST(nrf_crypto, aead_rejects_partial_overlap_before_modifying_storage,
     "REQ-0071") {
  fixture f;
  ASSERT_TRUE(f.clocks.initialize().has_value());
  ASSERT_TRUE(f.cc.initialize().has_value());
  std::array<std::uint8_t, 32> bytes{};
  std::array<std::uint8_t, 16> tag{};
  auto input = std::span{bytes}.first(16),
       output = std::span{bytes}.subspan(1, 16);
  EXPECT_FALSE(f.cc.chacha_poly1305({}, {}, input, output, tag).has_value());
  auto nonce = hex("00000003020100a0a1a2a3a4a5");
  EXPECT_FALSE(f.cc.ccm({}, nonce, {}, input, output, tag).has_value());
  EXPECT_FALSE(f.cc.pending());
}
} // namespace
