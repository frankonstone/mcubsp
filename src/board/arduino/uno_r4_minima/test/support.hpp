#pragma once

#include <cstddef>
#include <cstdint>
#include <span>
#include <string_view>

#if MCUBSP_COVERAGE
#include <mcucov/mcucov.hpp>
#include <mcucov/sinks/buffered.hpp>
#endif
#include <mcujson/mcujson.hpp>
#include <mcutest/mcutest.hpp>

#include <mcubsp/backend/ra4m1/gpio.hpp>
#include <mcubsp/core/direct_mmio.hpp>
#include <mcubsp/execution/bare_metal_policy.hpp>
#include <mcubsp/testing/rtt_output.hpp>

namespace mcubsp::board_test {

inline core::direct_mmio mmio;
inline backend::ra4m1::gpio_backend gpio{mmio};

struct execution_hooks {
    using critical_token = std::uint32_t;

    static critical_token enter_critical() noexcept {
        std::uint32_t previous;
        __asm__ volatile("mrs %0, primask\n\tcpsid i"
                         : "=r"(previous) :: "memory");
        return previous;
    }

    static void leave_critical(critical_token previous) noexcept {
        if ((previous & 1U) == 0U) {
            __asm__ volatile("cpsie i" ::: "memory");
        }
    }

    static execution::tick now() noexcept {
        return *reinterpret_cast<volatile const std::uint32_t*>(
            0xE000'1004U);
    }

    static bool in_isr() noexcept {
        std::uint32_t exception;
        __asm__ volatile("mrs %0, ipsr" : "=r"(exception));
        return exception != 0U;
    }

    static void poll() noexcept { __asm__ volatile("nop"); }
    static void notify() noexcept { __asm__ volatile("sev" ::: "memory"); }

    [[noreturn]] static void active_transfer_destroyed() noexcept {
        for (;;) {
            __asm__ volatile("bkpt #0");
        }
    }
};

using execution_policy = execution::bare_metal_policy<execution_hooks>;

inline void enable_cycle_counter() noexcept {
    auto& demcr = *reinterpret_cast<volatile std::uint32_t*>(0xE000'EDFCU);
    auto& dwt_control =
        *reinterpret_cast<volatile std::uint32_t*>(0xE000'1000U);
    auto& cycle_counter =
        *reinterpret_cast<volatile std::uint32_t*>(0xE000'1004U);
    demcr |= 1U << 24U;
    cycle_counter = 0U;
    dwt_control |= 1U;
}

inline void clear_icu_line(std::uint32_t line) noexcept {
    auto& ielsr = *reinterpret_cast<volatile std::uint32_t*>(
        0x4000'6300U + line * 4U);
    const auto value = ielsr;
    ielsr = value & ~(1U << 16U);
}

struct test_observation {
    mcutest::StringView name{};
    bool passed{};
};

struct run_observations {
    test_observation tests[MCUTEST_MAX_TESTS]{};
    std::size_t count{};
    std::size_t failures{};

    void append(mcutest::StringView name, bool passed) noexcept {
        if (count < MCUTEST_MAX_TESTS) {
            tests[count++] = {name, passed};
        }
        if (!passed) {
            ++failures;
        }
    }
};

struct rtt_output {
    run_observations* observations{};

    void write(mcutest::StringView value) noexcept {
        testing::rtt::write(value.data(), value.size());
    }

    void on_test_pass(mcutest::StringView name) noexcept {
        observations->append(name, true);
    }

    void on_test_fail(mcutest::StringView name) noexcept {
        observations->append(name, false);
    }
};

struct evidence_metadata {
    std::string_view suite;
    std::string_view firmware;
    std::string_view fixture;
    std::uint32_t coverage_build_id;
    bool coverage_enabled{true};
};

inline char evidence_buffer[8192]{};

[[nodiscard]] inline std::string_view as_string_view(
    mcutest::StringView value) noexcept {
    return {value.data(), value.size()};
}

[[nodiscard]] inline const mcutest::TestRecord* find_test(
    mcutest::StringView name) noexcept {
    const auto& registry = mcutest::registry();
    for (std::size_t index = 0; index < registry.count; ++index) {
        const auto& candidate = registry.records[index];
        if (candidate.name.size() != name.size()) {
            continue;
        }
        bool same = true;
        for (std::size_t character = 0; character < name.size(); ++character) {
            if (candidate.name[character] != name[character]) {
                same = false;
                break;
            }
        }
        if (same) {
            return &candidate;
        }
    }
    return nullptr;
}

template<typename DetailWriter>
bool write_evidence_report(const evidence_metadata& metadata,
                           const run_observations& observations,
                           DetailWriter&& write_details) noexcept {
    mcujson::JsonWriter json{std::span<char>{evidence_buffer}};
    json.object([&](auto& root) {
        root("format", "mcutest-results");
        root("version", 1U);
        root.object("run", [&](auto& run) {
            run("board", "Arduino UNO R4 Minima");
            run("mcu", "Renesas RA4M1 R7FA4M1AB");
            run("suite", metadata.suite);
            run("firmware", metadata.firmware);
            run("fixture", metadata.fixture);
            run("debug_probe", "SEGGER J-Link");
            run("interface", "SWD + RTT channel 0");
            run("toolchain", "arm-none-eabi-gcc 15.2.1");
            run.object("clocks_hz", [&](auto& clocks) {
                clocks("iclk", 48'000'000U);
                clocks("pclka", 48'000'000U);
                clocks("pclkb", 24'000'000U);
                clocks("pclkd", 48'000'000U);
            });
            if (metadata.coverage_enabled) {
                run.object("coverage", [&](auto& coverage) {
                    coverage("producer", "mcucov");
                    coverage("profile", "full");
                    coverage("build_id", metadata.coverage_build_id);
                    coverage("transport_encoding", "SLIP frame as RTT hex");
                });
            }
        });
        root.object("summary", [&](auto& summary) {
            summary("tests", observations.count);
            summary("failures", observations.failures);
            summary("passed", observations.count - observations.failures);
            summary("registry_overflow", mcutest::registry().overflow_count);
        });
        root.array("tests", [&](auto& tests) {
            for (std::size_t index = 0; index < observations.count; ++index) {
                const auto& observation = observations.tests[index];
                const auto* record = find_test(observation.name);
                tests.object([&](auto& test) {
                    test("name", as_string_view(observation.name));
                    test("status", observation.passed ? "passed" : "failed");
                    if (record != nullptr) {
#ifdef MCUTEST_SOURCE_LOCATION
                        test("file", as_string_view(record->file));
                        test("line", record->line);
#endif
                        test.array("requirements", [&](auto& requirements) {
                            for (std::size_t tag = 0; tag < record->tag_count;
                                 ++tag) {
                                requirements(as_string_view(record->tags[tag]));
                            }
                        });
                    }
                });
            }
        });
        root.object("hardware_observations", write_details);
    });
    const auto written = json.finish();
    if (!written) {
        return false;
    }
    constexpr char prefix[] = "MCUBSP_JSON:";
    testing::rtt::write_blocking(prefix, sizeof(prefix) - 1U);
    testing::rtt::write_blocking(evidence_buffer, *written);
    constexpr char newline[] = "\n";
    testing::rtt::write_blocking(newline, 1U);
    return true;
}

#if MCUBSP_COVERAGE
struct coverage_hex_sink {
#if defined(__MCUCOV_ANALYZER__)
    [[clang::annotate("mcucov:no_instrument")]]
#elif defined(__GNUC__)
    [[gnu::no_instrument_function]]
#endif
    bool write(const std::uint8_t* bytes,
               std::size_t size) noexcept {
        constexpr char hexadecimal[] = "0123456789abcdef";
        char encoded[128];
        std::size_t offset{};
        while (offset < size) {
            const auto chunk = size - offset < sizeof(encoded) / 2U
                ? size - offset
                : sizeof(encoded) / 2U;
            for (std::size_t index = 0; index < chunk; ++index) {
                const auto byte = bytes[offset + index];
                encoded[index * 2U] = hexadecimal[byte >> 4U];
                encoded[index * 2U + 1U] = hexadecimal[byte & 0x0fU];
            }
            testing::rtt::write_blocking(encoded, chunk * 2U);
            offset += chunk;
        }
        return true;
    }
};

#if defined(__MCUCOV_ANALYZER__)
[[clang::annotate("mcucov:no_instrument")]]
#elif defined(__GNUC__)
[[gnu::no_instrument_function]]
#endif
inline bool dump_coverage() noexcept {
    constexpr char prefix[] = "MCUBSP_COVERAGE_HEX:";
    testing::rtt::write_blocking(prefix, sizeof(prefix) - 1U);
    coverage_hex_sink backend;
    mcucov::BufferedSink<coverage_hex_sink, 64> sink{backend};
    const bool written = mcucov::dump(sink);
    constexpr char newline[] = "\n";
    testing::rtt::write_blocking(newline, 1U);
    return written;
}
#else
inline bool dump_coverage() noexcept { return true; }
#endif

template<std::size_t MetadataSize>
void start(const char (&metadata)[MetadataSize]) noexcept {
    testing::rtt::initialize();
    enable_cycle_counter();
    testing::rtt::write(metadata, MetadataSize - 1U);
}

template<typename DetailWriter>
[[noreturn]] void finish(const evidence_metadata& metadata,
                         DetailWriter&& write_details) noexcept {
    run_observations observations;
    mcutest::Runner<rtt_output> runner{rtt_output{&observations}};
    runner.run_all_registered();
    const bool passed = runner.summary() == 0;
    const bool report_written = write_evidence_report(
        metadata, observations, static_cast<DetailWriter&&>(write_details));
    const bool coverage_written = !metadata.coverage_enabled || dump_coverage();
    const bool complete = passed && report_written && coverage_written;
    constexpr char pass_result[] = "MCUBSP_RESULT:PASS\n";
    constexpr char fail_result[] = "MCUBSP_RESULT:FAIL\n";
    testing::rtt::write_blocking(
        complete ? pass_result : fail_result,
        complete ? sizeof(pass_result) - 1U : sizeof(fail_result) - 1U);
    for (;;) {
        __asm__ volatile("wfi");
    }
}

} // namespace mcubsp::board_test
