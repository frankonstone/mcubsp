# @req REQ-0047 REQ-0049
include(CMakePackageConfigHelpers)
set(mcubsp_package_dir "lib/cmake/mcubsp")
install(TARGETS mcubsp mcubsp_io mcubsp_device EXPORT mcubspTargets)
install(EXPORT mcubspTargets NAMESPACE mcubsp:: DESTINATION "${mcubsp_package_dir}")
configure_package_config_file("${CMAKE_CURRENT_LIST_DIR}/cmake/mcubspConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/mcubspConfig.cmake"
    INSTALL_DESTINATION "${mcubsp_package_dir}")
write_basic_package_version_file("${CMAKE_CURRENT_BINARY_DIR}/mcubspConfigVersion.cmake"
    VERSION "${PROJECT_VERSION}" COMPATIBILITY SameMinorVersion ARCH_INDEPENDENT)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/mcubspConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/mcubspConfigVersion.cmake"
    DESTINATION "${mcubsp_package_dir}")
# Preserve one relocatable layout for headers and board metadata.
# No build products, private references, vendor SDK or development dependencies.
install(DIRECTORY include cmake board doc test
    DESTINATION share/mcubsp
    PATTERN build EXCLUDE PATTERN __pycache__ EXCLUDE
    PATTERN _vendor EXCLUDE PATTERN _generated EXCLUDE
    PATTERN ".DS_Store" EXCLUDE PATTERN "*.pyc" EXCLUDE
    PATTERN "sdkconfig" EXCLUDE PATTERN "sdkconfig.old" EXCLUDE)
install(FILES README.md requirements.md implementation-plan.md DESTINATION share/mcubsp)
