// @req-file REQ-0052
#include <mcutest/mcutest.hpp>

int main() {
    mcutest::Runner<mcutest::JsonOutput> runner;
    return mcutest::run_json_output(runner);
}
