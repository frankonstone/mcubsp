if(NOT DEFINED TEST_PROGRAM OR NOT DEFINED OUTPUT_FILE)
    message(FATAL_ERROR "TEST_PROGRAM and OUTPUT_FILE are required")
endif()

get_filename_component(output_directory "${OUTPUT_FILE}" DIRECTORY)
file(MAKE_DIRECTORY "${output_directory}")
set(test_command "${TEST_PROGRAM}" ${TEST_ARGUMENTS})
if(COVERAGE_FILE)
    file(REMOVE "${COVERAGE_FILE}")
    set(test_command "${CMAKE_COMMAND}" -E env
        "MCUBSP_COVERAGE_FILE=${COVERAGE_FILE}" ${test_command})
endif()
execute_process(
    COMMAND ${test_command}
    RESULT_VARIABLE test_result
    OUTPUT_FILE "${OUTPUT_FILE}"
    ERROR_VARIABLE test_error
)

if(NOT test_result EQUAL 0)
    message(FATAL_ERROR
        "mcutest failed with exit code ${test_result}: ${test_error}")
endif()

file(READ "${OUTPUT_FILE}" test_json)
string(JSON result_format ERROR_VARIABLE json_error GET "${test_json}" format)
if(json_error OR NOT result_format STREQUAL "mcutest-results")
    message(FATAL_ERROR "mcutest emitted invalid JSON: ${json_error}")
endif()
