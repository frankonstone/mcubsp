# Explicit build-only cost comparison. The resulting ELFs have no reset
# vector/startup and MUST NOT be flashed as firmware.
include_guard(GLOBAL)

option(MCUHW_BUILD_IO_COST_REPORT
    "Build paired Cortex-M4 driver/io facade RAM and flash probes" OFF)
if(MCUHW_BUILD_IO_COST_REPORT)
    find_package(Python3 REQUIRED COMPONENTS Interpreter)
    find_program(MCUHW_IO_COST_CXX arm-none-eabi-g++ REQUIRED)
    find_program(MCUHW_IO_COST_SIZE arm-none-eabi-size REQUIRED)
    add_custom_target(mcuhw_io_cost_report
        COMMAND "${Python3_EXECUTABLE}"
            "${CMAKE_SOURCE_DIR}/tools/report-io-cost.py"
            --compiler "${MCUHW_IO_COST_CXX}"
            --size-tool "${MCUHW_IO_COST_SIZE}"
            --output-dir "${CMAKE_BINARY_DIR}/artifacts/io-cost"
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
        COMMENT "Building fresh paired RA4M1 driver/io facade size probes"
        VERBATIM)
endif()
