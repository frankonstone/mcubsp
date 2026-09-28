include_guard(GLOBAL)
# Shared native harness and source-directory suite registration.
# @req REQ-0035 REQ-0038 REQ-0052

function(mcuhw_configure_native_suite target name)
    set(group "${name}")
    if(name MATCHES "^ip:")
        set(group backend)
    elseif(name MATCHES "^device:")
        set(group device)
    elseif(name MATCHES "^mcu:")
        set(group mcu)
    endif()
    if(NOT "all" IN_LIST MCUHW_TEST_SUITES AND
            NOT "${name}" IN_LIST MCUHW_TEST_SUITES AND
            NOT "${group}" IN_LIST MCUHW_TEST_SUITES)
        set_target_properties(${target} PROPERTIES EXCLUDE_FROM_ALL TRUE)
        return()
    endif()
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
        RUNTIME_OUTPUT_DIRECTORY "${MCUHW_TEST_BINARY_DIR}")
    target_link_libraries(${target} PRIVATE mcuhw::io mcutest::mcutest)
    target_include_directories(${target} PRIVATE "${MCUHW_TEST_SOURCE_DIR}")
    if(MCUHW_BUILD_MCUIP_CONTRACTS)
        target_include_directories(${target} PRIVATE "${MCUHW_MCUIP_SOURCE_DIR}/include")
        target_compile_definitions(${target} PRIVATE MCUHW_TEST_MCUIP_CONTRACTS=1)
    endif()
    mcuhw_apply_restricted_runtime(${target})
    mcutest_configure_target(${target} MAX_TESTS 256 SOURCE_LOCATION ON)
    add_test(NAME "${target}" COMMAND "${CMAKE_COMMAND}"
        -DTEST_PROGRAM=$<TARGET_FILE:${target}>
        -DOUTPUT_FILE=${MCUHW_TEST_ARTIFACT_DIR}/${identifier}.json
        -P "${mcuhw_SOURCE_DIR}/test/support/CaptureMcutest.cmake")
    set_tests_properties("${target}" PROPERTIES LABELS "native;${category};${name}")
endfunction()
