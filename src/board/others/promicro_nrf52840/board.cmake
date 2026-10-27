# @req REQ-0053
# Profile identity describes the documented boot layout, not a PCB revision.
set(MCUBSP_BOARD_DEFAULT_REVISION uf2-s140-v6)
set(MCUBSP_BOARD_REVISIONS uf2-s140-v6)
set(MCUBSP_BOARD_ARCHITECTURE cortex-m4)
set(MCUBSP_BOARD_TOOLCHAIN cmake/toolchain-arm-cortex-m4.cmake)
set(MCUBSP_BOARD_PROFILE_HEADER include/mcubsp/board/others/promicro_nrf52840.hpp)
set(MCUBSP_BOARD_STARTUP
    "${CMAKE_CURRENT_LIST_DIR}/startup/startup.S"
    "${CMAKE_CURRENT_LIST_DIR}/startup/early_init.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/startup/restricted_runtime.cpp")
set(MCUBSP_BOARD_LINKER "${CMAKE_CURRENT_LIST_DIR}/linker.ld")
set(MCUBSP_BOARD_UF2_FAMILY 0xADA52840)
set(MCUBSP_BOARD_UF2_START 0x26000)
set(MCUBSP_BOARD_UF2_SIZE 0xC6000)
set(MCUBSP_BOARD_RUNNER [=[{"type":"uf2","family":"0xADA52840","board_id":"nrf52840","flash_regions":[{"start":"0x26000","size":"0xc6000"}],"recovery_doc":"doc/package.md"}]=])
