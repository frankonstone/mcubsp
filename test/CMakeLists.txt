# Host-executable suites and cross-compiler checks.
set(MCUBSP_TEST_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
set(MCUBSP_TEST_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}")
set(MCUBSP_TEST_ARTIFACT_DIR "${CMAKE_CURRENT_BINARY_DIR}/../artifact/native")

include(support/NativeSuites.cmake)
add_custom_target(native-tests)
set_target_properties(native-tests PROPERTIES FOLDER "Tests")

add_subdirectory(compile)
add_executable(mcubsp.test.fixture_failure "${MCUBSP_TEST_SOURCE_DIR}/support/test_fixture_failure.cpp")
add_dependencies(native-tests mcubsp.test.fixture_failure)
set_target_properties(mcubsp.test.fixture_failure PROPERTIES FOLDER "Tests/core/fixtures"
    RUNTIME_OUTPUT_DIRECTORY "${MCUBSP_TEST_BINARY_DIR}")
target_link_libraries(mcubsp.test.fixture_failure PRIVATE mcutest::mcutest)
mcubsp_apply_restricted_runtime(mcubsp.test.fixture_failure)
mcutest_configure_target(mcubsp.test.fixture_failure MAX_TESTS 4 SOURCE_LOCATION ON)
add_test(NAME mcubsp.fixture_failure COMMAND mcubsp.test.fixture_failure)
set_tests_properties(mcubsp.fixture_failure PROPERTIES WILL_FAIL TRUE LABELS "native;core")

# Module suites live beside the implementation tree that mirrors public headers.
foreach(component IN ITEMS
    core
    execution
    testing
    driver
    backend
    backend/esp32p4
    backend/ra4m1
    backend/stm32h753
    backend/nrf52840
    device/audio
    device/sensor
    device/ethernet
    io
    mcu
    mcu/esp32p4
    mcu/ra4m1
    mcu/nrf52840
    mcu/stm32h753
)
    add_subdirectory("${MCUBSP_ROOT}/src/${component}/test"
        "${CMAKE_BINARY_DIR}/src/${component}/test")
endforeach()

find_package(Python3 3.10 REQUIRED COMPONENTS Interpreter)
add_test(NAME mcubsp.test.build COMMAND "${CMAKE_COMMAND}"
    "-DTEST_PROGRAM=${Python3_EXECUTABLE}"
    "-DTEST_ARGUMENTS=${CMAKE_CURRENT_SOURCE_DIR}/workflow/run_checks.py;--compiler;${CMAKE_CXX_COMPILER};--cmake;${CMAKE_COMMAND};--contract-fixture;$<TARGET_FILE:mcubsp.test.contract_fixture>"
    "-DOUTPUT_FILE=${MCUBSP_TEST_ARTIFACT_DIR}/build.json"
    -P "${CMAKE_CURRENT_SOURCE_DIR}/support/CaptureMcutest.cmake")
set_tests_properties(mcubsp.test.build PROPERTIES LABELS "build")

add_test(NAME mcubsp.test.runner COMMAND "${CMAKE_COMMAND}"
    "-DTEST_PROGRAM=${Python3_EXECUTABLE}"
    "-DTEST_ARGUMENTS=${CMAKE_CURRENT_SOURCE_DIR}/workflow/test_hardware.py"
    "-DOUTPUT_FILE=${MCUBSP_TEST_ARTIFACT_DIR}/hardware.json"
    -P "${CMAKE_CURRENT_SOURCE_DIR}/support/CaptureMcutest.cmake")
set_tests_properties(mcubsp.test.runner PROPERTIES LABELS "workflow;runner")
add_test(NAME mcubsp.test.uf2 COMMAND "${CMAKE_COMMAND}"
    "-DTEST_PROGRAM=${Python3_EXECUTABLE}"
    "-DTEST_ARGUMENTS=${CMAKE_CURRENT_SOURCE_DIR}/workflow/test_uf2.py"
    "-DOUTPUT_FILE=${MCUBSP_TEST_ARTIFACT_DIR}/uf2.json"
    -P "${CMAKE_CURRENT_SOURCE_DIR}/support/CaptureMcutest.cmake")
set_tests_properties(mcubsp.test.uf2 PROPERTIES LABELS "workflow;uf2")
set_property(GLOBAL APPEND PROPERTY MCUBSP_NATIVE_REPORTS
    "${MCUBSP_TEST_ARTIFACT_DIR}/uf2.json")
set_property(GLOBAL APPEND PROPERTY MCUBSP_NATIVE_REPORTS
    "${MCUBSP_TEST_ARTIFACT_DIR}/hardware.json")
find_program(MCUBSP_TEST_NINJA ninja REQUIRED)
find_package(Git REQUIRED)
add_test(NAME mcubsp.test.provenance COMMAND "${CMAKE_COMMAND}"
    "-DTEST_PROGRAM=${Python3_EXECUTABLE}"
    "-DTEST_ARGUMENTS=${CMAKE_CURRENT_SOURCE_DIR}/workflow/test_provenance.py;--compiler;${CMAKE_CXX_COMPILER};--cmake;${CMAKE_COMMAND};--ninja;${MCUBSP_TEST_NINJA};--git;${GIT_EXECUTABLE}"
    "-DOUTPUT_FILE=${MCUBSP_TEST_ARTIFACT_DIR}/provenance.json"
    -P "${CMAKE_CURRENT_SOURCE_DIR}/support/CaptureMcutest.cmake")
set_tests_properties(mcubsp.test.provenance PROPERTIES LABELS "workflow;provenance")
set_property(GLOBAL APPEND PROPERTY MCUBSP_NATIVE_REPORTS
    "${MCUBSP_TEST_ARTIFACT_DIR}/provenance.json")
set_property(GLOBAL APPEND PROPERTY MCUBSP_NATIVE_REPORTS
    "${MCUBSP_TEST_ARTIFACT_DIR}/build.json")
add_test(NAME mcubsp.test.nrf_product COMMAND "${CMAKE_COMMAND}"
    "-DTEST_PROGRAM=${Python3_EXECUTABLE}"
    "-DTEST_ARGUMENTS=${CMAKE_CURRENT_SOURCE_DIR}/workflow/test_nrf_product.py"
    "-DOUTPUT_FILE=${MCUBSP_TEST_ARTIFACT_DIR}/nrf-product.json"
    -P "${CMAKE_CURRENT_SOURCE_DIR}/support/CaptureMcutest.cmake")
set_tests_properties(mcubsp.test.nrf_product PROPERTIES LABELS "workflow;metadata")
set_property(GLOBAL APPEND PROPERTY MCUBSP_NATIVE_REPORTS
    "${MCUBSP_TEST_ARTIFACT_DIR}/nrf-product.json")
find_program(MCUBSP_TEST_TCLSH tclsh)
if(MCUBSP_TEST_TCLSH)
    add_test(NAME mcubsp.test.nrf_debug COMMAND "${CMAKE_COMMAND}"
        "-DTEST_PROGRAM=${Python3_EXECUTABLE}"
        "-DTEST_ARGUMENTS=${CMAKE_CURRENT_SOURCE_DIR}/workflow/test_nrf_debug.py"
        "-DOUTPUT_FILE=${MCUBSP_TEST_ARTIFACT_DIR}/nrf-debug.json"
        -P "${CMAKE_CURRENT_SOURCE_DIR}/support/CaptureMcutest.cmake")
    set_tests_properties(mcubsp.test.nrf_debug PROPERTIES LABELS "workflow;debugger")
    set_property(GLOBAL APPEND PROPERTY MCUBSP_NATIVE_REPORTS
        "${MCUBSP_TEST_ARTIFACT_DIR}/nrf-debug.json")
endif()
get_property(native_reports GLOBAL PROPERTY MCUBSP_NATIVE_REPORTS)
list(JOIN native_reports "\n" native_registry)
file(GENERATE OUTPUT "${CMAKE_BINARY_DIR}/artifact/native-results.txt"
    CONTENT "${native_registry}\n")

if(MCUBSP_HOST_COVERAGE)
    get_property(coverage_reports GLOBAL PROPERTY MCUBSP_COVERAGE_REPORTS)
    list(JOIN coverage_reports "\n" coverage_registry)
    file(GENERATE OUTPUT "${CMAKE_BINARY_DIR}/artifact/coverage-results.txt"
        CONTENT "${coverage_registry}\n")
endif()
