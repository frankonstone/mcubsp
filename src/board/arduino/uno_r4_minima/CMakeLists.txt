# @req REQ-0049
function(mcubsp_uno_firmware target source)
    mcubsp_add_firmware(${target} BOARD arduino/uno_r4_minima
        REVISION "${MCUBSP_BOARD_REVISION}" STARTUP "${MCUBSP_STARTUP}" SOURCES "${source}" ${ARGN})
endfunction()
if(MCUBSP_BUILD_BOARD_TESTS)
    include("${MCUBSP_ROOT}/test/support/TestDependencies.cmake")
    mcubsp_test_dependency(mcutest)
    mcubsp_test_dependency(mcujson)
    find_package(mcucov CONFIG REQUIRED)
    add_subdirectory(test)
endif()
