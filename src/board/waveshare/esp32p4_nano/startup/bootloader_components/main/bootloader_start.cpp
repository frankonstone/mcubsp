// SPDX-License-Identifier: Apache-2.0 OR MIT

#include "bootloader_init.h"
#include "bootloader_utility.h"
#include "esp_rom_sys.h"

// The immutable ROM loader transfers control here. This project owns the
// second-stage policy: validate the partition table and boot only the factory
// application. Tests, coverage and functional drivers belong to that app.
// @req REQ-0003 REQ-0031 REQ-0041
extern "C" [[noreturn]] void call_start_cpu0(void) {
    if (bootloader_init() != ESP_OK) {
        esp_rom_printf("MCUBSP_BOOT status=fail stage=hardware_init\n");
        bootloader_reset();
    }

    bootloader_state_t state{};
    if (!bootloader_utility_load_partition_table(&state)) {
        esp_rom_printf("MCUBSP_BOOT status=fail stage=partition_table\n");
        bootloader_reset();
    }
    if (state.factory.offset == 0U || state.factory.size == 0U) {
        esp_rom_printf("MCUBSP_BOOT status=fail stage=factory_partition\n");
        bootloader_reset();
    }

    esp_rom_printf(
        "MCUBSP_BOOT status=load policy=factory offset=0x%08x size=0x%08x\n",
        static_cast<unsigned>(state.factory.offset),
        static_cast<unsigned>(state.factory.size));
    bootloader_utility_load_boot_image(&state, FACTORY_INDEX);
}
