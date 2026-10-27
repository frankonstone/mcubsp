# @req REQ-0049
set(MCUBSP_BOARD_SDK "esp-idf")
set(MCUBSP_BOARD_IDF_DEFAULTS "${CMAKE_CURRENT_LIST_DIR}/startup/sdkconfig.defaults")
set(MCUBSP_BOARD_IDF_BOOT "${CMAKE_CURRENT_LIST_DIR}/startup/bootloader_components")
set(MCUBSP_BOARD_IDF_START "${CMAKE_CURRENT_LIST_DIR}/startup/app_start.cpp")

# Board-local build selection and runner settings.
set(MCUBSP_BOARD_DEFAULT_REVISION "development-v1.0")
set(MCUBSP_BOARD_REVISIONS "development-v1.0")
set(MCUBSP_BOARD_ARCHITECTURE "esp32p4-riscv")
set(MCUBSP_BOARD_TOOLCHAIN "cmake/toolchain-esp32p4.cmake")
set(MCUBSP_BOARD_PROFILE_HEADER "include/mcubsp/board/waveshare/esp32p4_nano.hpp")
set(MCUBSP_BOARD_TOOLS "cmake;ninja;riscv-gcc;esp-idf;idf-python;idf-tools")
set(MCUBSP_BOARD_RUNNER [=[{"type":"esp-idf","tool":"idf-python","chip":"esp32p4","build_dir":"esp-idf-build","flash_regions":[{"start":"0x2000","size":"0x6000"},{"start":"0x8000","size":"0x1000"},{"start":"0x10000","size":"0x100000"}],"recovery_doc":"doc/package.md"}]=])
