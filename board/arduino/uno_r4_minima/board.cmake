# @req REQ-0049
set(MCUHW_BOARD_EARLY_INIT "${CMAKE_CURRENT_LIST_DIR}/bsp/early_init.cpp")
set(MCUHW_BOARD_STARTUP
    "${CMAKE_CURRENT_LIST_DIR}/bsp/startup.S"
    "${MCUHW_BOARD_EARLY_INIT}"
    "${CMAKE_CURRENT_LIST_DIR}/bsp/restricted_runtime.cpp")
set(MCUHW_BOARD_LINKER "${CMAKE_CURRENT_LIST_DIR}/bsp/linker.ld")

# Standalone facade examples compatible with this board binding.
set(MCUHW_BOARD_IO_EXAMPLES blink serial_echo spi_dma tmp102 w5100)

# Board-local build selection and runner settings.
set(MCUHW_BOARD_DEFAULT_REVISION "r4")
set(MCUHW_BOARD_REVISIONS "r4")
set(MCUHW_BOARD_ARCHITECTURE "cortex-m4")
set(MCUHW_BOARD_TOOLCHAIN "cmake/toolchain-arm-cortex-m4.cmake")
set(MCUHW_BOARD_PROFILE_HEADER "include/mcuhw/io/boards/uno_r4_minima.hpp")
set(MCUHW_BOARD_PROFILE_TYPE "mcuhw::io::boards::uno_r4_minima")
set(MCUHW_BOARD_TOOLS "cmake;ninja;arm-gcc")
set(MCUHW_BOARD_EXAMPLES "examples/apps/blink/CMakeLists.txt;examples/apps/tmp102/CMakeLists.txt")
set(MCUHW_BOARD_TEST_PROJECT "")
set(MCUHW_BOARD_RUNNER [=[{"type":"jlink","tool":"jlink","device":"R7FA4M1AB","interface":"SWD","speed_khz":4000,"artifact":"firmware.elf","flash_regions":[{"start":"0x4000","size":"0x3c000"}],"recovery_doc":"board/arduino/uno_r4_minima/docs/README.md"}]=])
