include_guard(GLOBAL)
include(ExternalProject)
include("${CMAKE_CURRENT_LIST_DIR}/Cache.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/EspIdfEnvironment.cmake")

# ESP-IDF owns its platform image, cache/libc initialization and bootloader build.
# Keep its build explicit and isolated, with no test/coverage dependencies.
# @req REQ-0049
function(mcuhw_add_idf_firmware target)
    mcuhw_idf_environment(sdk_environment)
    set(sdk_source "${CMAKE_CURRENT_BINARY_DIR}/${target}-idf-project")
    set(sdk_binary "${CMAKE_CURRENT_BINARY_DIR}/esp-idf-build")
    file(MAKE_DIRECTORY "${sdk_source}/main")
    file(MAKE_DIRECTORY "${sdk_source}/bootloader_components/main")
    # Track the custom policy's original files so direct cmake --build refreshes
    # the SDK copy whenever the shared boot integration changes.
    foreach(boot_file IN ITEMS CMakeLists.txt bootloader_start.cpp)
        configure_file("${MCUHW_BOARD_IDF_BOOT}/main/${boot_file}"
            "${sdk_source}/bootloader_components/main/${boot_file}" COPYONLY)
    endforeach()
    configure_file("${MCUHW_BOARD_IDF_DEFAULTS}" "${sdk_source}/sdkconfig.defaults" COPYONLY)
    set(sources "")
    foreach(source IN LISTS ARGN)
        get_filename_component(source "${source}" ABSOLUTE BASE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
        string(APPEND sources "\n    [==[${source}]==]")
    endforeach()
    file(CONFIGURE OUTPUT "${sdk_source}/CMakeLists.txt" CONTENT
        "cmake_minimum_required(VERSION 3.24)\ninclude([==[${MCUHW_IDF_PATH}/tools/cmake/project.cmake]==])\nidf_build_set_property(MINIMAL_BUILD ON)\nproject(${target})\n" @ONLY)
    file(CONFIGURE OUTPUT "${sdk_source}/main/CMakeLists.txt" CONTENT
        "idf_component_register(SRCS ${sources}\n    [==[${MCUHW_BOARD_IDF_START}]==]\n    INCLUDE_DIRS [==[${MCUHW_ROOT}/include]==]\n    REQUIRES esp_hw_support esp_mm esp_rom esp_system)\ntarget_compile_features(\${COMPONENT_LIB} PRIVATE cxx_std_23)\ntarget_compile_options(\${COMPONENT_LIB} PRIVATE -Os -fno-exceptions -fno-rtti -fno-threadsafe-statics -fno-use-cxa-atexit)\ntarget_compile_definitions(\${COMPONENT_LIB} PRIVATE main=mcuhw_application_main)\ntarget_link_options(\${COMPONENT_LIB} INTERFACE -Wl,--wrap=esp_startup_start_app)\n" @ONLY)
    find_program(MCUHW_IDF_NINJA ninja REQUIRED)
    mcuhw_prepare_sdk_flags(forward "${sdk_source}/parent-flags.cmake")
    list(APPEND forward CMAKE_BUILD_TYPE CMAKE_EXPORT_COMPILE_COMMANDS)
    set(seed "${sdk_source}/compile-cache.cmake")
    mcuhw_write_initial_cache("${seed}" ${forward})
    ExternalProject_Add(${target}
        SOURCE_DIR "${sdk_source}" BINARY_DIR "${sdk_binary}"
        DOWNLOAD_COMMAND "" UPDATE_COMMAND "" PATCH_COMMAND ""
        CONFIGURE_COMMAND ${sdk_environment}
            "${CMAKE_COMMAND}" -S "${sdk_source}" -B "${sdk_binary}" -G Ninja -C "${seed}"
            "-DIDF_TARGET=${MCUHW_IDF_TARGET}" "-DPYTHON=${PYTHON}"
            "-DCMAKE_MAKE_PROGRAM=${MCUHW_IDF_NINJA}"
        BUILD_COMMAND ${sdk_environment}
            "${CMAKE_COMMAND}" --build "${sdk_binary}"
        BUILD_ALWAYS TRUE
        BUILD_BYPRODUCTS "${sdk_binary}/${target}.elf" "${sdk_binary}/${target}.bin"
        INSTALL_COMMAND "${CMAKE_COMMAND}" -E copy_if_different
            "${sdk_binary}/${target}.elf" "${sdk_binary}/${target}.bin" "${CMAKE_CURRENT_BINARY_DIR}"
        USES_TERMINAL_CONFIGURE TRUE USES_TERMINAL_BUILD TRUE)
    ExternalProject_Add_StepDependencies(${target} configure "${seed}" "${sdk_source}/CMakeLists.txt" "${CMAKE_PROJECT_INCLUDE}")
    set_target_properties(${target} PROPERTIES FOLDER "Application")
    find_package(Python3 3.10 REQUIRED COMPONENTS Interpreter)
    mcuhw_firmware_artifact_command(artifact_command
        "${CMAKE_CURRENT_BINARY_DIR}/${target}.elf" supplied SDK_BUILD "${sdk_binary}"
        OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${target}.build.json")
    ExternalProject_Add_Step(${target} identity
        DEPENDEES install ALWAYS TRUE
        COMMAND "${Python3_EXECUTABLE}" "${MCUHW_ROOT}/tool/mcuhwlib/provenance.py"
            --root "${MCUHW_ROOT}" --build "${sdk_binary}" --target "${target}.elf"
            --ninja "${MCUHW_IDF_NINJA}"
            --extra "${MCUHW_BOARD_ENTRY}"
            --extra "${sdk_source}/sdkconfig"
            --output "${CMAKE_CURRENT_BINARY_DIR}/${target}.identity.json"
        COMMAND "${Python3_EXECUTABLE}" "${MCUHW_ROOT}/tool/mcuhwlib/provenance.py"
            --root "${MCUHW_ROOT}" --build "${sdk_binary}/bootloader" --target bootloader.elf
            --ninja "${MCUHW_IDF_NINJA}"
            --extra "${MCUHW_BOARD_ENTRY}" --extra "${sdk_source}/sdkconfig"
            --output "${CMAKE_CURRENT_BINARY_DIR}/${target}.bootloader.identity.json"
        COMMAND "${Python3_EXECUTABLE}" "${MCUHW_ROOT}/tool/mcuhwlib/provenance.py"
            --root "${MCUHW_ROOT}" --build "${sdk_binary}"
            --flash-manifest "${sdk_binary}/flasher_args.json"
            --output "${CMAKE_CURRENT_BINARY_DIR}/${target}.flash.identity.json"
        COMMAND ${artifact_command})
endfunction()
