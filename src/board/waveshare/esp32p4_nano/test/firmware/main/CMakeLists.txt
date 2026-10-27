get_filename_component(MCUBSP_ROOT "${CMAKE_CURRENT_LIST_DIR}/../../../../../../.." ABSOLUTE)
set(MCUBSP_ESP32P4_TEST_DIR "${MCUBSP_ROOT}/src/board/waveshare/esp32p4_nano/test")
set(MCUBSP_MCUTEST_SOURCE_DIR "${MCUBSP_ROOT}/../mcutest" CACHE PATH
    "mcutest checkout for the physical SDK test image")
set(MCUBSP_MCUTEST_INCLUDE_DIR "${MCUBSP_MCUTEST_SOURCE_DIR}/include")
if(NOT EXISTS "${MCUBSP_MCUTEST_INCLUDE_DIR}/mcutest/mcutest.hpp")
    message(FATAL_ERROR
        "mcutest headers not found at ${MCUBSP_MCUTEST_INCLUDE_DIR}")
endif()

idf_component_register(
    SRCS
        "app_start.cpp"
        "${MCUBSP_ESP32P4_TEST_DIR}/support.cpp"
    INCLUDE_DIRS "${MCUBSP_ROOT}/include"
    PRIV_INCLUDE_DIRS
        "${MCUBSP_ESP32P4_TEST_DIR}"
        "${MCUBSP_MCUTEST_INCLUDE_DIR}"
    REQUIRES esp_hw_support esp_mm esp_rom esp_system
)

if(NOT mcucov_DIR)
    file(GLOB MCUBSP_MCUCOV_CONFIGS
        "${MCUBSP_ROOT}/../mcucov/output/install/*/lib/cmake/mcucov/mcucov-config.cmake")
    list(LENGTH MCUBSP_MCUCOV_CONFIGS MCUBSP_MCUCOV_CONFIG_COUNT)
    if(NOT MCUBSP_MCUCOV_CONFIG_COUNT EQUAL 1)
        message(FATAL_ERROR "Select the host mcucov package explicitly with mcucov_DIR")
    endif()
    list(GET MCUBSP_MCUCOV_CONFIGS 0 MCUBSP_MCUCOV_CONFIG)
    get_filename_component(mcucov_DIR "${MCUBSP_MCUCOV_CONFIG}" DIRECTORY)
endif()
find_package(mcucov CONFIG REQUIRED)

# ESP-IDF adds GCC-only flags which the MCUCOV Clang analysis frontend does
# not accept for this RISC-V target. Filter them only for instrumentation.
get_target_property(MCUBSP_MCUCOV_INSTRUMENTER
    mcucov::mcucov-instrument IMPORTED_LOCATION_RELEASE)
set(MCUBSP_MCUCOV_INSTRUMENTER_ADAPTER
    "${CMAKE_BINARY_DIR}/mcucov/mcucov-instrument-esp32p4")
configure_file(
    "${CMAKE_CURRENT_LIST_DIR}/mcucov_instrument_esp32p4.sh.in"
    "${MCUBSP_MCUCOV_INSTRUMENTER_ADAPTER}"
    @ONLY)
file(CHMOD "${MCUBSP_MCUCOV_INSTRUMENTER_ADAPTER}"
    PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
set_target_properties(mcucov::mcucov-instrument PROPERTIES
    IMPORTED_LOCATION_RELEASE "${MCUBSP_MCUCOV_INSTRUMENTER_ADAPTER}")

add_library(waveshare.esp32p4_nano.test.board_support OBJECT
    "${MCUBSP_ESP32P4_TEST_DIR}/gpio.cpp"
    "${MCUBSP_ESP32P4_TEST_DIR}/i2c.cpp"
    "${MCUBSP_ESP32P4_TEST_DIR}/audio.cpp"
    "${MCUBSP_ESP32P4_TEST_DIR}/ethernet.cpp"
    "${MCUBSP_ESP32P4_TEST_DIR}/sdmmc.cpp"
    "${MCUBSP_ESP32P4_TEST_DIR}/wifi_sdio.cpp"
    "${MCUBSP_ESP32P4_TEST_DIR}/spi.cpp"
    "${MCUBSP_ESP32P4_TEST_DIR}/uart.cpp")
target_include_directories(waveshare.esp32p4_nano.test.board_support PRIVATE
    "${MCUBSP_ROOT}/include"
    "${MCUBSP_ESP32P4_TEST_DIR}"
    "${MCUBSP_MCUTEST_INCLUDE_DIR}")
idf_component_get_property(MCUBSP_ESP_MM_LIB esp_mm COMPONENT_LIB)
target_link_libraries(waveshare.esp32p4_nano.test.board_support PRIVATE ${MCUBSP_ESP_MM_LIB})

foreach(target ${COMPONENT_LIB} waveshare.esp32p4_nano.test.board_support)
    target_compile_features(${target} PRIVATE cxx_std_23)
    target_compile_options(${target} PRIVATE
        -Wall
        -Wextra
        -Werror
        -fno-exceptions
        -fno-rtti
        -fno-threadsafe-statics
        -fno-use-cxa-atexit)
    target_compile_definitions(${target} PRIVATE
        MCUTEST_MAX_TESTS=8
        MCUCOV_BYTE_PER_PROBE=1
        MCUCOV_COMPACT_RUNTIME=1)
endforeach()
target_compile_options(waveshare.esp32p4_nano.test.board_support PRIVATE
    -include "${MCUBSP_ROOT}/test/support/mcucov_compat.hpp")

mcucov_instrument_target(
    waveshare.esp32p4_nano.test.board_support
    PROFILE FULL
    BUILD_ID 0x4d48500d
    SOURCE_ROOT "${MCUBSP_ROOT}"
    OUTPUT_DIR "${CMAKE_BINARY_DIR}/mcucov/esp32p4_native_smoke"
)
target_sources(${COMPONENT_LIB} PRIVATE
    $<TARGET_OBJECTS:waveshare.esp32p4_nano.test.board_support>)
target_link_libraries(${COMPONENT_LIB} PRIVATE mcucov::runtime)
mcucov_configure_target(${COMPONENT_LIB} BUILD_ID 0x4d48500d)

# Let ESP-IDF perform its ordinary image/cache/libc/constructor startup, then
# take control at the boundary immediately before it creates the main FreeRTOS
# task and starts the scheduler.
target_link_options(${COMPONENT_LIB} INTERFACE
    "-Wl,--wrap=esp_startup_start_app")
target_linker_script(${COMPONENT_LIB} INTERFACE
    "${CMAKE_CURRENT_LIST_DIR}/mcucov_sections.ld")
