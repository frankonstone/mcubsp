# @req REQ-0049
set(MCUHW_BOARD_EARLY_INIT "${CMAKE_CURRENT_LIST_DIR}/bsp/early_init.cpp")
set(MCUHW_BOARD_STARTUP
    "${CMAKE_CURRENT_LIST_DIR}/bsp/startup.S"
    "${MCUHW_BOARD_EARLY_INIT}"
    "${CMAKE_CURRENT_LIST_DIR}/bsp/restricted_runtime.cpp")
set(MCUHW_BOARD_LINKER "${CMAKE_CURRENT_LIST_DIR}/bsp/linker.ld")
set(MCUHW_BOARD_LED "board_profile::pins_type::led_builtin")
set(MCUHW_BOARD_CLOCK_DECLARATION "extern \"C\" std::uint32_t mcuhw_uno_clock_status;")
set(MCUHW_BOARD_CLOCK_CHECK "mcuhw_uno_clock_status == 0U")

# Standalone facade examples compatible with this board binding.
set(MCUHW_BOARD_IO_EXAMPLES blink serial_echo spi_dma tmp102 w5100)
