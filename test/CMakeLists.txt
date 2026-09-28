# Test orchestration. Each group owns its sources and registration rules.
# @req REQ-0035 REQ-0038 REQ-0052
if(MCUHW_BUILD_PROBES)
    include(compile/Probes.cmake)
endif()
if(MCUHW_CHECK_HEADERS)
    mcuhw_add_header_self_containment_checks(check-headers
        ROOT "${mcuhw_SOURCE_DIR}/include")
endif()
if(MCUHW_WRITE_TOOLCHAIN_REPORT)
    include(tooling/ToolchainReport.cmake)
    mcuhw_write_toolchain_report("${mcuhw_BINARY_DIR}/toolchain-report.txt")
endif()
if(MCUHW_BUILD_IO_COST_REPORT)
    find_package(Python3 REQUIRED COMPONENTS Interpreter)
    find_program(MCUHW_IO_COST_CXX arm-none-eabi-g++ REQUIRED)
    find_program(MCUHW_IO_COST_SIZE arm-none-eabi-size REQUIRED)
    add_custom_target(mcuhw_io_cost_report
        COMMAND "${Python3_EXECUTABLE}"
            "${mcuhw_SOURCE_DIR}/tool/report-io-cost.py"
            --compiler "${MCUHW_IO_COST_CXX}"
            --size-tool "${MCUHW_IO_COST_SIZE}"
            --output-dir "${mcuhw_BINARY_DIR}/artifact/io-cost"
        WORKING_DIRECTORY "${mcuhw_SOURCE_DIR}"
        COMMENT "Building fresh paired RA4M1 driver/io facade size probes"
        VERBATIM)
endif()

if(MCUHW_BUILD_TESTS)
    set(MCUHW_TEST_SUITES "all" CACHE STRING "Native suites: all, core, drivers, io, descriptions, compile, backend, device, mcu, or kind:vendor/id")
    if(NOT "all" IN_LIST MCUHW_TEST_SUITES)
        foreach(suite IN LISTS MCUHW_TEST_SUITES)
            if(NOT suite MATCHES "^(core|drivers|io|descriptions|compile|backend|device|mcu)$" AND
                    NOT suite MATCHES "^(mcu|device|ip):[a-z0-9][a-z0-9_-]*/[a-z0-9][a-z0-9_.-]*$")
                message(FATAL_ERROR "Unknown native suite ${suite}")
            endif()
        endforeach()
    endif()
    set(MCUHW_TEST_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
    set(MCUHW_TEST_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}")
    set(MCUHW_TEST_ARTIFACT_DIR "${CMAKE_CURRENT_BINARY_DIR}/../artifact/native")
    include(support/NativeSuites.cmake)
    add_custom_target(native-tests)
    set_target_properties(native-tests PROPERTIES FOLDER "Tests")

    if("all" IN_LIST MCUHW_TEST_SUITES OR "core" IN_LIST MCUHW_TEST_SUITES)
        add_subdirectory(core)
    endif()
    if("all" IN_LIST MCUHW_TEST_SUITES OR "drivers" IN_LIST MCUHW_TEST_SUITES)
        add_subdirectory(driver)
    endif()
    if("all" IN_LIST MCUHW_TEST_SUITES OR "backend" IN_LIST MCUHW_TEST_SUITES OR
            MCUHW_TEST_SUITES MATCHES "ip:")
        add_subdirectory(backend)
    endif()
    if("all" IN_LIST MCUHW_TEST_SUITES OR "device" IN_LIST MCUHW_TEST_SUITES OR
            MCUHW_TEST_SUITES MATCHES "device:")
        add_subdirectory(device)
    endif()
    if("all" IN_LIST MCUHW_TEST_SUITES OR "io" IN_LIST MCUHW_TEST_SUITES)
        add_subdirectory(io)
    endif()
    if("all" IN_LIST MCUHW_TEST_SUITES OR "mcu" IN_LIST MCUHW_TEST_SUITES OR
            "descriptions" IN_LIST MCUHW_TEST_SUITES OR MCUHW_TEST_SUITES MATCHES "mcu:")
        add_subdirectory(mcu)
    endif()
    if("all" IN_LIST MCUHW_TEST_SUITES OR "compile" IN_LIST MCUHW_TEST_SUITES)
        add_subdirectory(compile)
    endif()
endif()

if(MCUHW_BUILD_TOOLING_TESTS)
    add_subdirectory(tooling)
endif()
