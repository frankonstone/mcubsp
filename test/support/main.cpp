// @req-file REQ-0035 REQ-0052
#include <mcutest/mcutest.hpp>
#ifdef MCUBSP_HOST_COVERAGE
#include <cstdio>
#include <cstdlib>
#include <mcucov/mcucov.hpp>
namespace {
struct coverage_sink {
    std::FILE* file;
    bool MCUCOV_NO_INSTRUMENT write(const std::uint8_t* bytes, std::size_t size) noexcept {
        return std::fwrite(bytes, 1, size, file) == size;
    }
};
}
int MCUCOV_NO_INSTRUMENT main() {
#else
int main() {
#endif
    mcutest::Runner<mcutest::JsonOutput> runner;
    const auto result = mcutest::run_json_output(runner);
#ifdef MCUBSP_HOST_COVERAGE
    const auto path = std::getenv("MCUBSP_COVERAGE_FILE");
    auto* file = path ? std::fopen(path, "wb") : nullptr;
    if (!file) { std::fputs("coverage output file unavailable\n", stderr); return 2; }
    coverage_sink sink{file};
    const bool captured = mcucov::dump(sink);
    const bool closed = std::fclose(file) == 0;
    if (!captured || !closed) { std::fputs("coverage capture failed\n", stderr); return 2; }
#endif
    return result;
}
