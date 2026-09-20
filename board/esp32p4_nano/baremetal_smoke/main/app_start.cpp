// SPDX-License-Identifier: Apache-2.0 OR MIT

#include "esp_rom_sys.h"
#include "hal/efuse_hal.h"
#include "soc/rtc.h"

#include "support.hpp"

// ESP-IDF's default startup calls this symbol immediately before it would
// create the main task and start FreeRTOS. The link wrapper keeps all required
// image/cache/libc/constructor initialization but transfers control to the
// allocation-free board-test path without starting a scheduler.
// @req REQ-0003 REQ-0031 REQ-0041
extern "C" [[noreturn]] void __wrap_esp_startup_start_app(void) {
    rtc_cpu_freq_config_t cpu_config{};
    rtc_clk_cpu_freq_get_config(&cpu_config);
    const auto chip_revision = efuse_hal_chip_revision();
    esp_rom_printf(
        "MCUHW_CONTEXT board=ESP32-P4-NANO part=ESP32-P4NRW32 "
        "chip_revision=%u firmware=pln-0019-native-app-v12 "
        "compiler=%s core=hp_cpu0 scheduler=none cpu_mhz=%u apb_hz=%u "
        "gpio=2 observation=mixed_board_native qualification=development\n",
        static_cast<unsigned>(chip_revision), __VERSION__,
        static_cast<unsigned>(cpu_config.freq_mhz),
        static_cast<unsigned>(rtc_clk_apb_freq_get()));
    esp_rom_delay_us(20'000U);

    const auto result =
        mcuhw::board_test::esp32p4_nano::run_tests_and_dump_coverage();
    const bool passed = result.tests != 0U && result.failures == 0
        && result.registry_overflow == 0U && result.coverage_written;
    esp_rom_printf(
        "MCUHW_RESULT status=%s suite=esp32p4_native tests=%u failures=%u "
        "registry_overflow=%u coverage=%s\n",
        passed ? "pass" : "fail", static_cast<unsigned>(result.tests),
        static_cast<unsigned>(result.failures),
        static_cast<unsigned>(result.registry_overflow),
        result.coverage_written ? "captured" : "failed");

    for (;;) {
        __asm__ volatile("wfi");
    }
}

// Kept only to satisfy ESP-IDF's otherwise unreachable default main-task
// object. The wrapped startup path above never invokes this function.
extern "C" void app_main(void) {}
