#pragma once

// @req-file REQ-0036 REQ-0037

#include <cstddef>
#include <cstdint>
#include <span>
#include <string_view>

#include <mcucov/mcucov.hpp>
#include <mcujson/mcujson.hpp>
#include <mcutest/mcutest.hpp>

namespace mcubsp::board_test::nucleo_h753zi {

struct test_observation {
    mcutest::StringView name{};
    bool passed{};
};

struct run_observations {
    test_observation tests[MCUTEST_MAX_TESTS]{};
    std::size_t count{};
    std::size_t failures{};

    void append(mcutest::StringView name, bool passed) noexcept {
        if (count < MCUTEST_MAX_TESTS) tests[count++] = {name, passed};
        if (!passed) ++failures;
    }
};

struct silent_output {
    run_observations* observations{};

    void write(mcutest::StringView) noexcept {}
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
};

struct capture_result {
    std::size_t evidence_size{};
    std::size_t coverage_size{};
    std::size_t test_count{};
    std::size_t failures{};
    bool complete{};
};

[[nodiscard]] inline std::string_view as_string_view(
    mcutest::StringView value) noexcept {
    return {value.data(), value.size()};
}

[[nodiscard]] inline const mcutest::TestRecord* find_test(
    mcutest::StringView name) noexcept {
    const auto& registry = mcutest::registry();
    for (std::size_t index = 0; index < registry.count; ++index) {
        const auto& candidate = registry.records[index];
        if (candidate.name.size() != name.size()) continue;
        bool same = true;
        for (std::size_t character = 0; character < name.size(); ++character) {
            if (candidate.name[character] != name[character]) {
                same = false;
                break;
            }
        }
        if (same) return &candidate;
    }
    return nullptr;
}

template<typename DetailWriter>
std::size_t write_evidence_report(std::span<char> destination,
                                  const evidence_metadata& metadata,
                                  const run_observations& observations,
                                  DetailWriter&& write_details) noexcept {
    mcujson::JsonWriter json{destination};
    json.object([&](auto& root) {
        root("format", "mcutest-results");
        root("version", 1U);
        root.object("run", [&](auto& run) {
            run("board", "NUCLEO-H753ZI MB1364 Rev E");
            run("mcu", "STMicroelectronics STM32H753ZI");
            run("suite", metadata.suite);
            run("firmware", metadata.firmware);
            run("fixture", metadata.fixture);
            run("debug_probe", "ST-LINK V3");
            run("interface", "SWD RAM capture + USART3 VCP");
            run("toolchain", "arm-none-eabi-gcc");
            run.object("clocks_hz", [&](auto& clocks) {
                clocks("sysclk", 64'000'000U);
                clocks("hclk", 64'000'000U);
                clocks("pclk", 64'000'000U);
            });
            run.object("coverage", [&](auto& coverage) {
                coverage("producer", "mcucov");
                coverage("profile", "full");
                coverage("build_id", metadata.coverage_build_id);
                coverage("transport_encoding", "binary capture from AXI SRAM");
            });
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
    return written ? *written : 0U;
}

struct memory_sink {
    std::span<std::uint8_t> destination;
    std::size_t size{};

    bool MCUCOV_NO_INSTRUMENT write(const std::uint8_t* bytes,
                                    std::size_t count) noexcept {
        if (count > destination.size() - size) return false;
        for (std::size_t index = 0; index < count; ++index) {
            destination[size + index] = bytes[index];
        }
        size += count;
        return true;
    }
};

template<typename DetailWriter>
capture_result capture(std::span<char> evidence_destination,
                       std::span<std::uint8_t> coverage_destination,
                       const evidence_metadata& metadata,
                       DetailWriter&& write_details) noexcept {
    run_observations observations;
    mcutest::Runner<silent_output> runner{silent_output{&observations}};
    runner.run_all_registered();
    const bool tests_passed = runner.summary() == 0;
    const auto evidence_size = write_evidence_report(
        evidence_destination, metadata, observations,
        static_cast<DetailWriter&&>(write_details));
    memory_sink sink{coverage_destination};
    const bool coverage_written = mcucov::dump(sink);
    return {
        .evidence_size = evidence_size,
        .coverage_size = sink.size,
        .test_count = observations.count,
        .failures = observations.failures,
        .complete = tests_passed && evidence_size != 0U && coverage_written,
    };
}

} // namespace mcubsp::board_test::nucleo_h753zi
