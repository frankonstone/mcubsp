include_guard(GLOBAL)
include(ExternalProject)
include("${CMAKE_CURRENT_LIST_DIR}/EspIdfEnvironment.cmake")

# ESP-IDF owns its platform image, cache/libc initialization and bootloader build.
# Keep its build explicit and isolated, with no test/coverage dependencies.
# @req REQ-0049
function(mcubsp_add_idf_firmware target)
    mcubsp_idf_environment(sdk_environment)
    set(MCUBSP_APPLICATION_VERSION "0.1.0" CACHE STRING
        "Explicit SDK application version, independent of repository revision")
    set(sdk_source "${CMAKE_CURRENT_BINARY_DIR}/${target}-idf-project")
    set(sdk_binary "${CMAKE_CURRENT_BINARY_DIR}/${target}-idf-build")
    file(MAKE_DIRECTORY "${sdk_source}/main")
    file(MAKE_DIRECTORY "${sdk_source}/bootloader_components/main")
    # Track the custom policy's original files so direct cmake --build refreshes
    # the SDK copy whenever the shared boot integration changes.
    foreach(boot_file IN ITEMS CMakeLists.txt bootloader_start.cpp)
        configure_file("${MCUBSP_BOARD_IDF_BOOT}/main/${boot_file}"
            "${sdk_source}/bootloader_components/main/${boot_file}" COPYONLY)
    endforeach()
    configure_file("${MCUBSP_BOARD_IDF_DEFAULTS}" "${sdk_source}/sdkconfig.defaults" COPYONLY)
    set(sources "")
    foreach(source IN LISTS ARGN)
        get_filename_component(source "${source}" ABSOLUTE BASE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
        string(APPEND sources "\n    [==[${source}]==]")
    endforeach()
    file(CONFIGURE OUTPUT "${sdk_source}/CMakeLists.txt" CONTENT
        "cmake_minimum_required(VERSION 3.24)\nset(PROJECT_VER [==[${MCUBSP_APPLICATION_VERSION}]==])\ninclude([==[${MCUBSP_IDF_PATH}/tools/cmake/project.cmake]==])\nidf_build_set_property(MINIMAL_BUILD ON)\nproject(${target})\ntarget_link_options(${target}.elf PRIVATE [==[LINKER:--dependency-file,${sdk_binary}/${target}.elf.link.d]==])\n" @ONLY)
    file(CONFIGURE OUTPUT "${sdk_source}/main/CMakeLists.txt" CONTENT
        "idf_component_register(SRCS ${sources}\n    [==[${MCUBSP_BOARD_IDF_START}]==]\n    INCLUDE_DIRS [==[${MCUBSP_ROOT}/include]==]\n    REQUIRES esp_hw_support esp_mm esp_rom esp_system)\ntarget_compile_features(\${COMPONENT_LIB} PRIVATE cxx_std_23)\ntarget_compile_options(\${COMPONENT_LIB} PRIVATE -Os -fno-exceptions -fno-rtti -fno-threadsafe-statics -fno-use-cxa-atexit)\ntarget_compile_definitions(\${COMPONENT_LIB} PRIVATE main=mcubsp_application_main)\ntarget_link_options(\${COMPONENT_LIB} INTERFACE -Wl,--wrap=esp_startup_start_app)\n" @ONLY)
    find_program(MCUBSP_IDF_NINJA ninja REQUIRED)
    set(seed "${sdk_source}/compile-cache.cmake")
    file(WRITE "${seed}" "set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE BOOL \"\")\n")
    ExternalProject_Add(${target}
        SOURCE_DIR "${sdk_source}" BINARY_DIR "${sdk_binary}"
        DOWNLOAD_COMMAND "" UPDATE_COMMAND "" PATCH_COMMAND ""
        CONFIGURE_COMMAND ${sdk_environment}
            "${CMAKE_COMMAND}" -S "${sdk_source}" -B "${sdk_binary}" -G Ninja -C "${seed}"
            "-DIDF_TARGET=${MCUBSP_IDF_TARGET}" "-DPYTHON=${PYTHON}"
            "-DCMAKE_MAKE_PROGRAM=${MCUBSP_IDF_NINJA}"
        BUILD_COMMAND ${sdk_environment}
            "${CMAKE_COMMAND}" --build "${sdk_binary}"
        BUILD_ALWAYS TRUE
        BUILD_BYPRODUCTS "${sdk_binary}/${target}.elf" "${sdk_binary}/${target}.bin"
        INSTALL_COMMAND "${CMAKE_COMMAND}" -E copy_if_different
            "${sdk_binary}/${target}.elf" "${sdk_binary}/${target}.bin" "${CMAKE_CURRENT_BINARY_DIR}"
        USES_TERMINAL_CONFIGURE TRUE USES_TERMINAL_BUILD TRUE)
    ExternalProject_Add_Step(${target} artifact DEPENDEES install ALWAYS TRUE
        COMMAND "${CMAKE_COMMAND}"
            "-DFIRMWARE=${CMAKE_CURRENT_BINARY_DIR}/${target}.elf" "-Dsdk_build=${sdk_binary}" "-Dboard=${ARG_BOARD}"
            "-Drevision=${ARG_REVISION}" "-Dstartup=supplied" "-Dcompiler=${CMAKE_CXX_COMPILER}"
            "-Dgenerator=Ninja" "-Dbuild_root=${sdk_binary}" "-Dsource_root=${CMAKE_SOURCE_DIR}"
            "-Dframework_root=${MCUBSP_ROOT}" "-Dninja=${MCUBSP_IDF_NINJA}" "-Dpython=${PYTHON}"
            -P "${MCUBSP_ROOT}/cmake/FirmwareArtifact.cmake")
    set_target_properties(${target} PROPERTIES FOLDER "Application")
endfunction()
