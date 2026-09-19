if(NOT DEFINED CXX_COMPILER OR NOT DEFINED SOURCE OR NOT DEFINED EXPECT_SUCCESS)
    message(FATAL_ERROR "CXX_COMPILER, SOURCE and EXPECT_SUCCESS are required")
endif()

execute_process(
    COMMAND "${CXX_COMPILER}" ${COMPILE_OPTIONS} -std=c++23
            -I "${INCLUDE_DIR}" -fsyntax-only
            "${SOURCE}"
    RESULT_VARIABLE compile_result
    OUTPUT_VARIABLE compile_output
    ERROR_VARIABLE compile_error
)

if(EXPECT_SUCCESS AND NOT compile_result EQUAL 0)
    message(FATAL_ERROR
        "Expected ${SOURCE} to compile, but it failed:\n${compile_output}${compile_error}")
endif()

if(NOT EXPECT_SUCCESS AND compile_result EQUAL 0)
    message(FATAL_ERROR "Expected ${SOURCE} to be rejected, but it compiled")
endif()

if(NOT EXPECT_SUCCESS)
    message(STATUS "Observed expected compiler rejection for ${SOURCE}")
endif()
