include_guard(GLOBAL)
# Shared native harness and source-directory suite registration.
# @req REQ-0035 REQ-0038 REQ-0052

function(mcubsp_configure_native_suite target name)
    string(MAKE_C_IDENTIFIER "${name}" identifier)
    add_dependencies(native-tests ${target})
    set(group "${name}")
    if(name MATCHES "^(ip|device|mcu):(.+)$")
        set(group "${CMAKE_MATCH_1}/${CMAKE_MATCH_2}")
        if(group MATCHES "^ip/")
            string(REGEX REPLACE "^ip/" "backend/" group "${group}")
        endif()
    elseif(name STREQUAL "drivers")
        set(group driver)
    elseif(name STREQUAL "descriptions")
        set(group "mcu/shared")
    endif()
    string(REGEX REPLACE "/.*$" "" category "${group}")
    set_target_properties(${target} PROPERTIES FOLDER "Tests/${group}"
        RUNTIME_OUTPUT_DIRECTORY "${MCUBSP_TEST_BINARY_DIR}")
    target_link_libraries(${target} PRIVATE mcubsp::io mcutest::mcutest)
    target_include_directories(${target} PRIVATE "${MCUBSP_TEST_SOURCE_DIR}")
    mcubsp_apply_restricted_runtime(${target})
    mcutest_configure_target(${target} MAX_TESTS 256 SOURCE_LOCATION ON)
    if(MCUBSP_HOST_COVERAGE)
        target_compile_definitions(${target} PRIVATE MCUBSP_HOST_COVERAGE=1)
        if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
            # MCUCOV can emit a switch helper used only by constant-evaluated
            # functions. Preserve strict warnings on ordinary native builds.
            target_compile_options(${target} PRIVATE -Wno-unneeded-internal-declaration)
        endif()
        target_compile_options(${target} PRIVATE -include "${MCUBSP_ROOT}/test/support/mcucov_compat.hpp")
        mcucov_instrument_target(${target} PROFILE FULL BUILD_ID 0x4d420001
            SOURCE_ROOT "${MCUBSP_ROOT}" OUTPUT_DIR "${MCUBSP_TEST_BINARY_DIR}/mcucov/${identifier}")
        set_property(GLOBAL APPEND PROPERTY MCUBSP_COVERAGE_REPORTS
            "${identifier}|${MCUBSP_TEST_ARTIFACT_DIR}/${identifier}.mcuv|${MCUBSP_TEST_BINARY_DIR}/mcucov/${identifier}")
    endif()
    add_test(NAME "${target}" COMMAND "${CMAKE_COMMAND}"
        -DTEST_PROGRAM=$<TARGET_FILE:${target}>
        -DOUTPUT_FILE=${MCUBSP_TEST_ARTIFACT_DIR}/${identifier}.json
        "-DCOVERAGE_FILE=$<$<BOOL:${MCUBSP_HOST_COVERAGE}>:${MCUBSP_TEST_ARTIFACT_DIR}/${identifier}.mcuv>"
        -P "${mcubsp_SOURCE_DIR}/test/support/CaptureMcutest.cmake")
    set_property(GLOBAL APPEND PROPERTY MCUBSP_NATIVE_REPORTS
        "${MCUBSP_TEST_ARTIFACT_DIR}/${identifier}.json")
    set_tests_properties("${target}" PROPERTIES LABELS "native;${category};${name}")
endfunction()
