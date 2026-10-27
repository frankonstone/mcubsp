# @req REQ-0049
set(MCUBSP_BOARD_DEFAULT_REVISION r4)
set(MCUBSP_BOARD_REVISIONS r4)
set(MCUBSP_BOARD_ARCHITECTURE cortex-m4)
set(MCUBSP_BOARD_TOOLCHAIN cmake/toolchain-arm-cortex-m4.cmake)
set(MCUBSP_BOARD_PROFILE_HEADER include/mcubsp/board/arduino/uno_r4_minima.hpp)
set(MCUBSP_BOARD_STARTUP
    "${CMAKE_CURRENT_LIST_DIR}/startup/startup.S"
    "${CMAKE_CURRENT_LIST_DIR}/startup/early_init.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/startup/restricted_runtime.cpp")
set(MCUBSP_BOARD_LINKER "${CMAKE_CURRENT_LIST_DIR}/linker.ld")
set(MCUBSP_BOARD_RUNNER [=[{"type":"jlink","device":"R7FA4M1AB","interface":"SWD","speed":4000,"flash_regions":[{"start":"0x4000","size":"0x3c000"}],"recovery_doc":"doc/package.md"}]=])
