#include <mcutest/mcutest.hpp>

namespace {

struct ObservableFailureFixture {
    void SetUp() { initialized = true; }
    void TearDown() { initialized = false; }

    bool initialized{};
};

TEST_F(ObservableFailureFixture, reports_a_failed_assertion, "fixture-contract") {
    ASSERT_TRUE(f->initialized);
    EXPECT_EQ(1, 2);
}

} // namespace

int main() {
    mcutest::Runner<mcutest::StdoutOutput> runner;
    runner.run_all_registered();
    return runner.summary();
}
