# @req REQ-0049
set(MCUHW_BOARD_EARLY_INIT "${CMAKE_CURRENT_LIST_DIR}/bsp/early_init.cpp")
set(MCUHW_BOARD_STARTUP
    "${CMAKE_CURRENT_LIST_DIR}/bsp/startup.S"
    "${MCUHW_BOARD_EARLY_INIT}"
    "${CMAKE_CURRENT_LIST_DIR}/bsp/restricted_runtime.cpp")
set(MCUHW_BOARD_LINKER "${CMAKE_CURRENT_LIST_DIR}/bsp/linker.ld")

# Board-local build selection and runner settings.
set(MCUHW_BOARD_DEFAULT_REVISION "mb1364-rev-e")
set(MCUHW_BOARD_REVISIONS "mb1364-rev-e")
set(MCUHW_BOARD_ARCHITECTURE "cortex-m7")
set(MCUHW_BOARD_TOOLCHAIN "cmake/toolchain-arm-cortex-m7.cmake")
set(MCUHW_BOARD_PROFILE_HEADER "include/mcuhw/io/board/nucleo_h753zi.hpp")
set(MCUHW_BOARD_PROFILE_TYPE "mcuhw::io::boards::nucleo_h753zi")
set(MCUHW_BOARD_TOOLS "cmake;ninja;arm-gcc")
set(MCUHW_BOARD_EXAMPLES "")
set(MCUHW_BOARD_TEST_PROJECT "")
set(MCUHW_BOARD_RUNNER [=[{"type":"openocd","tool":"openocd","interface":"interface/stlink.cfg","target":"target/stm32h7x.cfg","artifact":"firmware.elf","flash_regions":[{"start":"0x08000000","size":"0x200000"}],"recovery_doc":"board/st/nucleo_h753zi/doc/README.md"}]=])
