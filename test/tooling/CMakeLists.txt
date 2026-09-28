# Tooling can run independently of the native test framework.
find_package(Python3 3.10 REQUIRED COMPONENTS Interpreter)
add_test(NAME mcuhw.test.tooling
    COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/run_tests.py"
        --output "${CMAKE_CURRENT_BINARY_DIR}/../../artifact/mcutest-tooling.json")
set_tests_properties(mcuhw.test.tooling PROPERTIES LABELS "tooling;packaging")
