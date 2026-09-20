get_filename_component(MCUHW_ROOT "${CMAKE_CURRENT_LIST_DIR}/../../../.." ABSOLUTE)
set(MCUHW_ESP32P4_TEST_DIR "${MCUHW_ROOT}/board/esp32p4_nano/tests")
set(MCUHW_MCUTEST_INCLUDE_DIR "${MCUHW_ROOT}/../mcutest/include")
if(NOT EXISTS "${MCUHW_MCUTEST_INCLUDE_DIR}/mcutest/mcutest.hpp")
    message(FATAL_ERROR
        "mcutest headers not found at ${MCUHW_MCUTEST_INCLUDE_DIR}")
endif()

idf_component_register(
    SRCS
        "app_start.cpp"
        "${MCUHW_ESP32P4_TEST_DIR}/support.cpp"
    INCLUDE_DIRS "${MCUHW_ROOT}/include"
    PRIV_INCLUDE_DIRS
        "${MCUHW_ESP32P4_TEST_DIR}"
        "${MCUHW_MCUTEST_INCLUDE_DIR}"
    REQUIRES esp_hw_support esp_mm esp_rom esp_system
)

file(GLOB MCUHW_MCUCOV_CONFIGS
    "${MCUHW_ROOT}/../mcucov/output/install/*/lib/cmake/mcucov/mcucov-config.cmake")
list(LENGTH MCUHW_MCUCOV_CONFIGS MCUHW_MCUCOV_CONFIG_COUNT)
if(NOT MCUHW_MCUCOV_CONFIG_COUNT EQUAL 1)
    message(FATAL_ERROR
        "Expected one installed mcucov package below ${MCUHW_ROOT}/../mcucov/output/install")
endif()
list(GET MCUHW_MCUCOV_CONFIGS 0 MCUHW_MCUCOV_CONFIG)
get_filename_component(mcucov_DIR "${MCUHW_MCUCOV_CONFIG}" DIRECTORY)
find_package(mcucov CONFIG REQUIRED)

# ESP-IDF adds GCC-only flags which the MCUCOV Clang analysis frontend does
# not accept for this RISC-V target. Filter them only for instrumentation.
get_target_property(MCUHW_MCUCOV_INSTRUMENTER
    mcucov::mcucov-instrument IMPORTED_LOCATION_RELEASE)
set(MCUHW_MCUCOV_INSTRUMENTER_ADAPTER
    "${CMAKE_BINARY_DIR}/mcucov/mcucov-instrument-esp32p4")
configure_file(
    "${CMAKE_CURRENT_LIST_DIR}/mcucov_instrument_esp32p4.sh.in"
    "${MCUHW_MCUCOV_INSTRUMENTER_ADAPTER}"
    @ONLY)
file(CHMOD "${MCUHW_MCUCOV_INSTRUMENTER_ADAPTER}"
    PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
set_target_properties(mcucov::mcucov-instrument PROPERTIES
    IMPORTED_LOCATION_RELEASE "${MCUHW_MCUCOV_INSTRUMENTER_ADAPTER}")

add_library(mcuhw_esp32p4_board_tests OBJECT
    "${MCUHW_ESP32P4_TEST_DIR}/gpio.cpp"
    "${MCUHW_ESP32P4_TEST_DIR}/i2c.cpp"
    "${MCUHW_ESP32P4_TEST_DIR}/audio.cpp"
    "${MCUHW_ESP32P4_TEST_DIR}/ethernet.cpp"
    "${MCUHW_ESP32P4_TEST_DIR}/sdmmc.cpp"
    "${MCUHW_ESP32P4_TEST_DIR}/wifi_sdio.cpp"
    "${MCUHW_ESP32P4_TEST_DIR}/spi.cpp"
    "${MCUHW_ESP32P4_TEST_DIR}/uart.cpp")
target_include_directories(mcuhw_esp32p4_board_tests PRIVATE
    "${MCUHW_ROOT}/include"
    "${MCUHW_ESP32P4_TEST_DIR}"
    "${MCUHW_MCUTEST_INCLUDE_DIR}")
idf_component_get_property(MCUHW_ESP_MM_LIB esp_mm COMPONENT_LIB)
target_link_libraries(mcuhw_esp32p4_board_tests PRIVATE ${MCUHW_ESP_MM_LIB})

foreach(target ${COMPONENT_LIB} mcuhw_esp32p4_board_tests)
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
target_compile_options(mcuhw_esp32p4_board_tests PRIVATE
    -include "${CMAKE_CURRENT_LIST_DIR}/mcucov_compat.hpp")

mcucov_instrument_target(
    mcuhw_esp32p4_board_tests
    PROFILE FULL
    BUILD_ID 0x4d48500d
    SOURCE_ROOT "${MCUHW_ROOT}"
    OUTPUT_DIR "${CMAKE_BINARY_DIR}/mcucov/esp32p4_native_smoke"
)
target_sources(${COMPONENT_LIB} PRIVATE
    $<TARGET_OBJECTS:mcuhw_esp32p4_board_tests>)
target_link_libraries(${COMPONENT_LIB} PRIVATE mcucov::runtime)
mcucov_configure_target(${COMPONENT_LIB} BUILD_ID 0x4d48500d)

# Let ESP-IDF perform its ordinary image/cache/libc/constructor startup, then
# take control at the boundary immediately before it creates the main FreeRTOS
# task and starts the scheduler.
target_link_options(${COMPONENT_LIB} INTERFACE
    "-Wl,--wrap=esp_startup_start_app")
target_linker_script(${COMPONENT_LIB} INTERFACE
    "${CMAKE_CURRENT_LIST_DIR}/mcucov_sections.ld")
