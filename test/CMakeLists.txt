# Host-executable suites and cross-compiler checks.
set(MCUBSP_TEST_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
set(MCUBSP_TEST_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}")
set(MCUBSP_TEST_ARTIFACT_DIR "${CMAKE_CURRENT_BINARY_DIR}/../artifact/native")

include(support/NativeSuites.cmake)
add_custom_target(native-tests)
set_target_properties(native-tests PROPERTIES FOLDER "Tests")

add_subdirectory(compile)
add_subdirectory(core)
add_subdirectory(driver)
add_subdirectory(backend)
add_subdirectory(device)
add_subdirectory(io)
add_subdirectory(mcu)
