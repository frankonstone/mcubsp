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
    if(MCUBSP_BUILD_MCUIP_CONTRACTS)
        target_include_directories(${target} PRIVATE "${MCUBSP_MCUIP_SOURCE_DIR}/include")
        target_compile_definitions(${target} PRIVATE MCUBSP_TEST_MCUIP_CONTRACTS=1)
    endif()
    mcubsp_apply_restricted_runtime(${target})
    mcutest_configure_target(${target} MAX_TESTS 256 SOURCE_LOCATION ON)
    add_test(NAME "${target}" COMMAND "${CMAKE_COMMAND}"
        -DTEST_PROGRAM=$<TARGET_FILE:${target}>
        -DOUTPUT_FILE=${MCUBSP_TEST_ARTIFACT_DIR}/${identifier}.json
        -P "${mcubsp_SOURCE_DIR}/test/support/CaptureMcutest.cmake")
    set_tests_properties("${target}" PROPERTIES LABELS "native;${category};${name}")
endfunction()
