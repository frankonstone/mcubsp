#pragma once

#include <cstddef>

namespace mcubsp::board_test::esp32p4_nano {

struct run_result {
    std::size_t tests{};
    int failures{};
    std::size_t registry_overflow{};
    bool coverage_written{};
};

[[nodiscard]] run_result run_tests_and_dump_coverage() noexcept;

} // namespace mcubsp::board_test::esp32p4_nano
