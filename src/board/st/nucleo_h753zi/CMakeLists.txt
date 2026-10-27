# @req REQ-0038 REQ-0049
if(MCUBSP_BUILD_BOARD_TESTS)
    include("${MCUBSP_ROOT}/test/support/TestDependencies.cmake")
    mcubsp_test_dependency(mcutest)
    mcubsp_test_dependency(mcujson)
    find_package(mcucov CONFIG REQUIRED)
    include(tests.cmake)
endif()
