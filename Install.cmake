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
# Consumer package: explicit application resources, with the same relative layout
# as the checkout. Test infrastructure and maintainer records stay in the source tree.
install(DIRECTORY include/ DESTINATION share/mcubsp/include
    FILES_MATCHING PATTERN "*.hpp")
install(FILES cmake/Firmware.cmake cmake/FirmwareArtifact.cmake
    cmake/board-toolchain.cmake
    cmake/toolchain-arm-cortex-m4.cmake cmake/toolchain-arm-cortex-m7.cmake
    cmake/toolchain-esp32p4.cmake cmake/toolchain-macos-arm64.cmake
    cmake/toolchain-linux-x86_64.cmake
    DESTINATION share/mcubsp/cmake)
install(DIRECTORY cmake/arch cmake/esp-idf DESTINATION share/mcubsp/cmake
    FILES_MATCHING PATTERN "*.cmake")

file(GLOB board_entries CONFIGURE_DEPENDS "${MCUBSP_ROOT}/src/board/*/*/board.cmake")
foreach(entry IN LISTS board_entries)
    get_filename_component(board_directory "${entry}" DIRECTORY)
    file(RELATIVE_PATH board_id "${MCUBSP_ROOT}/src/board" "${board_directory}")
    set(board_destination "share/mcubsp/src/board/${board_id}")
    install(FILES "${entry}" DESTINATION "${board_destination}")
    if(EXISTS "${board_directory}/linker.ld")
        install(FILES "${board_directory}/linker.ld" DESTINATION "${board_destination}")
    endif()
    install(DIRECTORY "${board_directory}/startup" DESTINATION "${board_destination}"
        PATTERN build EXCLUDE PATTERN ".DS_Store" EXCLUDE)
    if(EXISTS "${board_directory}/debug")
        install(DIRECTORY "${board_directory}/debug" DESTINATION "${board_destination}")
    endif()
endforeach()

install(FILES tool/firmware-inputs.py tool/hardware.py tool/uf2.py tool/RunnerInfo.cmake
    DESTINATION share/mcubsp/tool)
install(DIRECTORY example DESTINATION share/mcubsp
    PATTERN output EXCLUDE PATTERN build EXCLUDE PATTERN ".DS_Store" EXCLUDE)
install(FILES doc/package.md DESTINATION share/mcubsp/doc)
install(FILES doc/package.md DESTINATION share/mcubsp RENAME README.md)

# Retain the Nordic BSD notice with redistributed derived register facts.
install(FILES doc/reference/nrf52840/NORDIC-LICENSE.txt
    DESTINATION share/mcubsp/doc/licenses)
