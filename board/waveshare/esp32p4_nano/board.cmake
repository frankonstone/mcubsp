# @req REQ-0049
set(MCUHW_BOARD_SDK "esp-idf")
set(MCUHW_BOARD_IDF_DEFAULTS "${CMAKE_CURRENT_LIST_DIR}/bsp/sdkconfig.defaults")
set(MCUHW_BOARD_IDF_BOOT "${CMAKE_CURRENT_LIST_DIR}/bsp/bootloader_components")
set(MCUHW_BOARD_IDF_START "${CMAKE_CURRENT_LIST_DIR}/bsp/app_start.cpp")

# Board-local build selection and runner settings.
set(MCUHW_BOARD_DEFAULT_REVISION "development-v1.0")
set(MCUHW_BOARD_REVISIONS "development-v1.0")
set(MCUHW_BOARD_ARCHITECTURE "esp32p4-riscv")
set(MCUHW_BOARD_TOOLCHAIN "cmake/toolchain-esp32p4.cmake")
set(MCUHW_BOARD_PROFILE_HEADER "include/mcuhw/io/board/esp32p4_nano.hpp")
set(MCUHW_BOARD_PROFILE_TYPE "mcuhw::io::boards::esp32p4_nano")
set(MCUHW_BOARD_TOOLS "cmake;ninja;riscv-gcc;esp-idf;idf-python;idf-tools")
set(MCUHW_BOARD_EXAMPLES "")
set(MCUHW_BOARD_TEST_PROJECT "board/waveshare/esp32p4_nano/test/firmware")
set(MCUHW_BOARD_RUNNER [=[{"type":"esp-idf","tool":"idf-python","chip":"esp32p4","build_dir":"esp-idf-build","flash_regions":[{"start":"0x2000","size":"0x6000"},{"start":"0x8000","size":"0x1000"},{"start":"0x10000","size":"0x100000"}],"recovery_doc":"board/waveshare/esp32p4_nano/doc/README.md"}]=])
