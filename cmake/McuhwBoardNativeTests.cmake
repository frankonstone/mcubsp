include_guard(GLOBAL)
include("${CMAKE_CURRENT_LIST_DIR}/McuhwCache.cmake")

# Cross-board menus expose host suites without resolving host-test dependencies
# until a group is built. Each group owns a native compiler/cache and evidence.
function(_mcuhw_board_native_group group suites)
    set(binary "${CMAKE_BINARY_DIR}/native-tests/${group}")
    set(MCUHW_BOARD "")
    set(MCUHW_BOARD_TESTS "")
    set(MCUHW_BOARD_REVISION "")
    set(MCUHW_ACTIVE_BOARDS "")
    set(MCUHW_EXAMPLE "")
    set(MCUHW_ARCH native)
    set(MCUHW_WORKSPACE OFF)
    set(MCUHW_TEST_SUITES "${suites}")
    set(MCUHW_BUILD_TESTS ON)
    foreach(flag IN ITEMS BUILD_PROBES CHECK_HEADERS BUILD_MCUIP_CONTRACTS
            BUILD_TOOLING_TESTS BUILD_IO_EXAMPLES BUILD_IO_COST_REPORT INSTALL)
        set(MCUHW_${flag} OFF)
    endforeach()
    set(CMAKE_CXX_COMPILER "${MCUHW_NATIVE_CXX_COMPILER}")
    set(CMAKE_CXX_FLAGS "${MCUHW_NATIVE_CXX_FLAGS}")
    set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
    set(seed "${binary}/initial-cache.cmake")
    mcuhw_write_initial_cache("${seed}"
        CMAKE_CXX_COMPILER CMAKE_CXX_FLAGS CMAKE_BUILD_TYPE CMAKE_MAKE_PROGRAM CMAKE_EXPORT_COMPILE_COMMANDS
        MCUHW_BOARD MCUHW_BOARD_TESTS MCUHW_BOARD_REVISION MCUHW_ACTIVE_BOARDS MCUHW_EXAMPLE
        MCUHW_ARCH MCUHW_WORKSPACE MCUHW_TEST_SUITES MCUHW_BUILD_TESTS
        MCUHW_BUILD_PROBES MCUHW_CHECK_HEADERS MCUHW_BUILD_MCUIP_CONTRACTS
        MCUHW_BUILD_TOOLING_TESTS MCUHW_BUILD_IO_EXAMPLES MCUHW_BUILD_IO_COST_REPORT MCUHW_INSTALL
        MCUHW_MCUTEST_SOURCE_DIR Python3_EXECUTABLE)
    add_custom_target(test.${group}
        COMMAND "${CMAKE_COMMAND}" -E env --unset=CXX --unset=CC --
            "${CMAKE_COMMAND}" -S "${MCUHW_ROOT}" -B "${binary}" -G "${CMAKE_GENERATOR}" -C "${seed}"
        COMMAND "${CMAKE_COMMAND}" --build "${binary}" --config "$<CONFIG>" --target native-tests
        COMMAND "${CMAKE_CTEST_COMMAND}" --test-dir "${binary}" -C "$<CONFIG>" --output-on-failure
        USES_TERMINAL VERBATIM)
    set_target_properties(test.${group} PROPERTIES FOLDER "Board/${MCUHW_BOARD_SELECTION}/Tests/Native/${group}")
    add_dependencies(tests test.${group})
endfunction()

function(mcuhw_add_board_native_tests)
    set(MCUHW_BOARD_SELECTION "${MCUHW_BOARD}")
    set(MCUHW_MCUTEST_SOURCE_DIR "${MCUHW_ROOT}/../mcutest" CACHE PATH "Optional local mcutest checkout")
    set(MCUHW_NATIVE_CXX_COMPILER "" CACHE FILEPATH "Host C++23 compiler for selectable board-menu native tests")
    set(MCUHW_NATIVE_CXX_FLAGS "" CACHE STRING "Host compiler flags for selectable native tests")
    if(NOT MCUHW_NATIVE_CXX_COMPILER)
        find_program(native_compiler NAMES g++-15 g++-14 g++-13 g++ c++ NO_CACHE)
        if(native_compiler)
            set(MCUHW_NATIVE_CXX_COMPILER "${native_compiler}" CACHE FILEPATH
                "Host C++23 compiler for selectable board-menu native tests" FORCE)
        endif()
    endif()
    _mcuhw_board_native_group(core core)
    _mcuhw_board_native_group(driver drivers)
    _mcuhw_board_native_group(backend backend)
    _mcuhw_board_native_group(device device)
    _mcuhw_board_native_group(io io)
    _mcuhw_board_native_group(mcu "mcu;descriptions")
    _mcuhw_board_native_group(compile compile)
endfunction()
