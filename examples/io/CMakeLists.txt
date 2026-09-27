cmake_minimum_required(VERSION 3.25)
# These examples use UNO connector names and the RA4M1 clock-status contract.
if(NOT MCUHW_BOARD_PROFILE STREQUAL "mcuhw::io::boards::uno_r4_minima")
    message(FATAL_ERROR "The examples/io firmware requires arduino/uno_r4_minima")
endif()
if(NOT MCUHW_IO_EXAMPLE MATCHES "^[a-z0-9_]+$" OR
        NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${MCUHW_IO_EXAMPLE}.cpp")
    message(FATAL_ERROR "Unknown MCUHW_IO_EXAMPLE '${MCUHW_IO_EXAMPLE}'")
endif()
mcuhw_add_firmware(${MCUHW_FIRMWARE_TARGET}
    SOURCES "${MCUHW_IO_EXAMPLE}.cpp" STARTUP supplied)
