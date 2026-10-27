#include <array>
#include <cstdio>
#include <cstdlib>
#include <mcubsp/io/board.hpp>
#include <mcubsp/board/arduino/uno_r4_minima.hpp>
#include <mcubsp/execution/fake_policy.hpp>
#include <mcubsp/testing/recording_mmio.hpp>
struct diagnostic_policy : mcubsp::execution::fake_policy {
    [[noreturn]] void contract_violation(mcubsp::io::contract_failure info) noexcept {
        std::printf("%s:%s:%u:%u\n", info.component, info.reason,
                    info.operation.slot, info.operation.generation);
        std::fflush(stdout);
        std::_Exit(77); // Observe the diagnostic without executing a host trap.
    }
};
int main() {
    diagnostic_policy policy;
    mcubsp::testing::recording_mmio<0x80000, 4096> mmio{0x4000'0000U};
    auto hw = mcubsp::io::make_board<mcubsp::board::arduino::uno_r4_minima>(policy, mmio, 1000U);
    mcubsp::io::require(hw.begin());
    mcubsp::io::require(hw.serial.begin());
    std::array<std::uint8_t, 1> bytes{0x55};
    auto pending = mcubsp::io::require(hw.serial.write_async(bytes));
} // Pending operation destruction must invoke the diagnostic hook.
