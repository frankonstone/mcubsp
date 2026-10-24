// @req-file REQ-0031 REQ-0049
#include "esp_rom_sys.h"
#include "hal/efuse_hal.h"
#include "soc/rtc.h"

int mcubsp_application_main();

// Retain SDK cache/libc/constructor initialization, then enter the application
// on HP CPU0 before the scheduler is started. The profile requires 360 MHz and
// the observed v1.0 silicon. Reject another clock/revision rather than applying
// the development register bindings to unqualified hardware.
extern "C" [[noreturn]] void __wrap_esp_startup_start_app() {
    rtc_cpu_freq_config_t config{};
    rtc_clk_cpu_freq_get_config(&config);
    if (config.freq_mhz != 360 || efuse_hal_chip_revision() != 100U) {
        esp_rom_printf("mcubsp: this profile requires ESP32-P4 v1.0 at 360 MHz\n");
        __builtin_trap();
    }
    static_cast<void>(mcubsp_application_main());
    for (;;) __asm__ volatile("wfi");
}

// The IDF default task is never entered in supplied-startup mode.
extern "C" void app_main() {}
