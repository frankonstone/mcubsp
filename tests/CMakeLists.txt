# Component-focused native suites. # @req REQ-0035 REQ-0038 REQ-0052
set(MCUHW_TEST_SUITES "all" CACHE STRING "Native suites: all, core, drivers, io, descriptions, compile, or kind:vendor/id")
add_custom_target(native-tests)
set_target_properties(native-tests PROPERTIES FOLDER "Verification")
function(mcuhw_native_suite name)
    if(NOT "all" IN_LIST MCUHW_TEST_SUITES AND NOT "${name}" IN_LIST MCUHW_TEST_SUITES)
        return()
    endif()
    string(MAKE_C_IDENTIFIER "${name}" identifier)
    set(target "mcuhw_test_${identifier}")
    # Every translation unit has one executable owner. Shared contracts belong
    # in a dedicated integration suite instead of selection-dependent deduplication.
    set(unique_sources "")
    foreach(source IN LISTS ARGN)
        get_filename_component(absolute "${source}" ABSOLUTE BASE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
        string(SHA256 key "${absolute}")
        get_property(owner GLOBAL PROPERTY "mcuhw_test_owner_${key}")
        if(owner)
            message(FATAL_ERROR "Native test ${source} belongs to both ${owner} and ${name}; give it one suite owner.")
        endif()
        set_property(GLOBAL PROPERTY "mcuhw_test_owner_${key}" "${name}")
        list(APPEND unique_sources "${absolute}")
    endforeach()
    if(NOT unique_sources)
        return()
    endif()
    add_executable(${target} ${unique_sources} main.cpp)
    add_dependencies(native-tests ${target})
    set_target_properties(${target} PROPERTIES FOLDER "Verification/Native suites")
    target_link_libraries(${target} PRIVATE mcuhw::io mcutest::mcutest)
    target_include_directories(${target} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
    if(MCUHW_BUILD_MCUIP_CONTRACTS)
        target_include_directories(${target} PRIVATE "${MCUHW_MCUIP_SOURCE_DIR}/include")
        target_compile_definitions(${target} PRIVATE MCUHW_TEST_MCUIP_CONTRACTS=1)
    endif()
    mcuhw_apply_restricted_runtime(${target})
    mcutest_configure_target(${target} MAX_TESTS 256 SOURCE_LOCATION ON)
    add_test(NAME "mcuhw.${name}" COMMAND "${CMAKE_COMMAND}"
        -DTEST_PROGRAM=$<TARGET_FILE:${target}>
        -DOUTPUT_FILE=${CMAKE_CURRENT_BINARY_DIR}/../artifacts/native/${identifier}.json
        -P "${mcuhw_SOURCE_DIR}/cmake/CaptureMcutest.cmake")
    set_tests_properties("mcuhw.${name}" PROPERTIES LABELS "native;${name}")
endfunction()

mcuhw_native_suite(core test_metadata.cpp test_ownership.cpp test_protected_access.cpp
    test_register.cpp test_recording_mmio.cpp test_execution_policy.cpp test_transfer.cpp)
mcuhw_native_suite(drivers test_sdmmc.cpp test_sdio.cpp test_timing_contracts.cpp)
mcuhw_native_suite(io test_io.cpp test_io_boards.cpp test_io_peripherals.cpp
    test_io_spi_devices.cpp test_spi_device_backends.cpp test_io_i2c_devices.cpp)
mcuhw_native_suite(descriptions test_description_suite.cpp test_three_description_gate.cpp)

# Only full maintainer runs enumerate the catalog; selected suites use direct paths.
set(component_manifests "")
if("all" IN_LIST MCUHW_TEST_SUITES)
    file(GLOB component_manifests CONFIGURE_DEPENDS
        "${mcuhw_SOURCE_DIR}/mcu/*/*/component.json"
        "${mcuhw_SOURCE_DIR}/ip/*/*/component.json"
        "${mcuhw_SOURCE_DIR}/device/*/*/component.json")
else()
    foreach(suite IN LISTS MCUHW_TEST_SUITES)
        if(suite MATCHES "^(mcu|device|ip):([a-z0-9][a-z0-9_-]*/[a-z0-9][a-z0-9_.-]*)$")
            list(APPEND component_manifests "${mcuhw_SOURCE_DIR}/${CMAKE_MATCH_1}/${CMAKE_MATCH_2}/component.json")
        elseif(NOT suite MATCHES "^(core|drivers|io|descriptions|compile)$")
            message(FATAL_ERROR "Unknown native suite ${suite}")
        endif()
    endforeach()
endif()
foreach(manifest IN LISTS component_manifests)
    if(NOT EXISTS "${manifest}")
        message(FATAL_ERROR "Missing component suite manifest: ${manifest}")
    endif()
    file(READ "${manifest}" json)
    set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${manifest}")
    string(JSON kind GET "${json}" kind)
    string(JSON id GET "${json}" id)
    string(JSON count LENGTH "${json}" tests)
    set(sources "")
    if(count GREATER 0)
        math(EXPR last "${count} - 1")
        foreach(index RANGE 0 ${last})
            string(JSON source GET "${json}" tests ${index})
            if(source MATCHES "\\.cpp$")
                list(APPEND sources "${mcuhw_SOURCE_DIR}/${source}")
            endif()
        endforeach()
    endif()
    if(sources)
        mcuhw_native_suite("${kind}:${id}" ${sources})
    endif()
endforeach()

if("all" IN_LIST MCUHW_TEST_SUITES OR "core" IN_LIST MCUHW_TEST_SUITES)
    add_executable(mcuhw_fixture_failure test_fixture_failure.cpp)
    add_dependencies(native-tests mcuhw_fixture_failure)
    set_target_properties(mcuhw_fixture_failure PROPERTIES FOLDER "Internal/Test fixtures")
    target_link_libraries(mcuhw_fixture_failure PRIVATE mcutest::mcutest)
    mcuhw_apply_restricted_runtime(mcuhw_fixture_failure)
    mcutest_configure_target(mcuhw_fixture_failure MAX_TESTS 4 SOURCE_LOCATION ON)
    add_test(NAME mcuhw.fixture_failure COMMAND mcuhw_fixture_failure)
    set_tests_properties(mcuhw.fixture_failure PROPERTIES WILL_FAIL TRUE LABELS "native;core")
endif()

if("all" IN_LIST MCUHW_TEST_SUITES OR "compile" IN_LIST MCUHW_TEST_SUITES)
function(mcuhw_add_compile_case name source expect_success)
    add_test(
        NAME "compile.${name}"
        COMMAND "${CMAKE_COMMAND}"
            -DCXX_COMPILER=${CMAKE_CXX_COMPILER}
            "-DCOMPILE_OPTIONS=${MCUHW_RESTRICTED_COMPILE_OPTIONS}"
            -DINCLUDE_DIR=${mcuhw_SOURCE_DIR}/include
            -DSOURCE=${CMAKE_CURRENT_SOURCE_DIR}/compile/${source}
            -DEXPECT_SUCCESS=${expect_success}
            -P "${mcuhw_SOURCE_DIR}/cmake/CompileCase.cmake"
    )
endfunction()

mcuhw_add_compile_case(cxx23 cxx23_features.cpp TRUE)
mcuhw_add_compile_case(io_example io_example.cpp TRUE)
mcuhw_add_compile_case(io_devices_example io_devices_example.cpp TRUE)
mcuhw_add_compile_case(io_disabled_peripheral io_disabled_peripheral_forbidden.cpp FALSE)
mcuhw_add_compile_case(io_unsupported_dma io_unsupported_dma_forbidden.cpp FALSE)
mcuhw_add_compile_case(io_move_runtime io_move_runtime_forbidden.cpp FALSE)
mcuhw_add_compile_case(exceptions_rejected exceptions_forbidden.cpp FALSE)
mcuhw_add_compile_case(rtti_rejected rtti_forbidden.cpp FALSE)
mcuhw_add_compile_case(command_read_rejected command_read_forbidden.cpp FALSE)
mcuhw_add_compile_case(unsafe_rmw_rejected unsafe_rmw_forbidden.cpp FALSE)
mcuhw_add_compile_case(invalid_field_rejected invalid_field_forbidden.cpp FALSE)
mcuhw_add_compile_case(invalid_enum_rejected invalid_enum_forbidden.cpp FALSE)
mcuhw_add_compile_case(duplicate_pin_rejected duplicate_pin_forbidden.cpp FALSE)
mcuhw_add_compile_case(unroutable_pin_rejected unroutable_pin_forbidden.cpp FALSE)
mcuhw_add_compile_case(unsupported_capability_rejected unsupported_capability_forbidden.cpp FALSE)
mcuhw_add_compile_case(invalid_clock_rejected invalid_clock_forbidden.cpp FALSE)
mcuhw_add_compile_case(interrupt_conflict_rejected interrupt_conflict_forbidden.cpp FALSE)
mcuhw_add_compile_case(invalid_dma_rejected invalid_dma_forbidden.cpp FALSE)
mcuhw_add_compile_case(reserved_write_rejected reserved_write_forbidden.cpp FALSE)
mcuhw_add_compile_case(ra4m1_unavailable_pin_rejected ra4m1_unavailable_pin_forbidden.cpp FALSE)
mcuhw_add_compile_case(ra4m1_third_gpt_output_rejected ra4m1_third_gpt_output_forbidden.cpp FALSE)
mcuhw_add_compile_case(ra4m1_invalid_gpt_output_rejected ra4m1_invalid_gpt_output_forbidden.cpp FALSE)
mcuhw_add_compile_case(stm32h753_unavailable_pin_rejected stm32h753_unavailable_pin_forbidden.cpp FALSE)
mcuhw_add_compile_case(stm32h753_advanced_output_rejected stm32h753_advanced_output_forbidden.cpp FALSE)
mcuhw_add_compile_case(esp32p4_unavailable_pin_rejected esp32p4_unavailable_pin_forbidden.cpp FALSE)
mcuhw_add_compile_case(esp32p4_i3c_capability_rejected esp32p4_i3c_capability_forbidden.cpp FALSE)

# The all-target gate exists now but is only built after all three real,
# source-backed description headers exist. No placeholder MCU types are used.
# @req REQ-0040
set(description_headers
    "${mcuhw_SOURCE_DIR}/include/mcuhw/mcu/ra4m1/description.hpp"
    "${mcuhw_SOURCE_DIR}/include/mcuhw/mcu/stm32h753/description.hpp"
    "${mcuhw_SOURCE_DIR}/include/mcuhw/mcu/esp32p4/description.hpp"
)
set(all_descriptions_available TRUE)
foreach(header IN LISTS description_headers)
    if(NOT EXISTS "${header}")
        set(all_descriptions_available FALSE)
    endif()
endforeach()
if(all_descriptions_available)
    add_library(mcuhw_all_descriptions OBJECT description_all.cpp)
    add_dependencies(native-tests mcuhw_all_descriptions)
    set_target_properties(mcuhw_all_descriptions PROPERTIES FOLDER "Verification/Compile")
    target_link_libraries(mcuhw_all_descriptions PRIVATE mcuhw::mcuhw)
    mcuhw_apply_restricted_runtime(mcuhw_all_descriptions)
else()
    message(STATUS
        "All-three description gate deferred until the three real description headers exist")
endif()

endif()
