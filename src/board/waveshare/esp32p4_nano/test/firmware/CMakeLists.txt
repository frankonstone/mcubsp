cmake_minimum_required(VERSION 3.22)
set(MCUBSP_APPLICATION_VERSION "0.1.0" CACHE STRING
    "Explicit test application version, independent of repository revision")
set(PROJECT_VER "${MCUBSP_APPLICATION_VERSION}")

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
get_filename_component(mcubsp_boot_components
    "${CMAKE_CURRENT_LIST_DIR}/../../startup/bootloader_components" ABSOLUTE)
idf_build_set_property(BOOTLOADER_EXTRA_COMPONENT_DIRS "${mcubsp_boot_components}")
idf_build_set_property(MINIMAL_BUILD ON)
project(mcubsp_esp32p4_baremetal_smoke)
target_link_options(${CMAKE_PROJECT_NAME}.elf PRIVATE
    "LINKER:--dependency-file,${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}.elf.link.d")

# Produce a record only after every image in the SDK flash bundle is complete.
add_custom_target(mcubsp-test-artifact ALL
    COMMAND "${CMAKE_COMMAND}"
        "-DFIRMWARE=${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}.elf"
        "-Dsdk_build=${CMAKE_BINARY_DIR}" "-Dboard=waveshare/esp32p4_nano"
        "-Drevision=development-v1.0" "-Dstartup=supplied"
        "-Dcompiler=${CMAKE_CXX_COMPILER}"
        "-Dgenerator=${CMAKE_GENERATOR}" "-Dbuild_root=${CMAKE_BINARY_DIR}"
        "-Dsource_root=${CMAKE_SOURCE_DIR}"
        "-Dframework_root=${CMAKE_CURRENT_SOURCE_DIR}/../../../../../.."
        "-Dninja=${CMAKE_MAKE_PROGRAM}" "-Dpython=${PYTHON}"
        -P "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../cmake/FirmwareArtifact.cmake"
    DEPENDS app bootloader partition_table_bin VERBATIM)
