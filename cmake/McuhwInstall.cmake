# @req REQ-0047 REQ-0049
include(CMakePackageConfigHelpers)
set(mcuhw_package_dir "lib/cmake/mcuhw")
install(TARGETS mcuhw mcuhw_io mcuhw_device EXPORT mcuhwTargets)
install(EXPORT mcuhwTargets NAMESPACE mcuhw:: DESTINATION "${mcuhw_package_dir}")
configure_package_config_file("${CMAKE_CURRENT_LIST_DIR}/mcuhwConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/mcuhwConfig.cmake"
    INSTALL_DESTINATION "${mcuhw_package_dir}")
write_basic_package_version_file("${CMAKE_CURRENT_BINARY_DIR}/mcuhwConfigVersion.cmake"
    VERSION "${PROJECT_VERSION}" COMPATIBILITY SameMinorVersion ARCH_INDEPENDENT)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/mcuhwConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/mcuhwConfigVersion.cmake"
    DESTINATION "${mcuhw_package_dir}")
# Preserve one relocatable layout for headers, startup and tools.
# No build products, private references, vendor SDK or development dependencies.
install(DIRECTORY include cmake board device example tool doc test
    DESTINATION share/mcuhw
    PATTERN build EXCLUDE PATTERN __pycache__ EXCLUDE
    PATTERN _vendor EXCLUDE PATTERN _generated EXCLUDE
    PATTERN ".DS_Store" EXCLUDE PATTERN "*.pyc" EXCLUDE
    PATTERN "sdkconfig" EXCLUDE PATTERN "sdkconfig.old" EXCLUDE)
install(FILES README.md requirements.md implementation-plan.md DESTINATION share/mcuhw)
