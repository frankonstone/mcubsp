# @req REQ-0049
set(MCUHW_BOARD_SDK "esp-idf")
# This development board has no ordinary user LED. Wire an external LED to GPIO2.
set(MCUHW_BOARD_LED "board_profile::pins_type::gpio2")
set(MCUHW_BOARD_CLOCK_CHECK "true")
set(MCUHW_BOARD_IDF_DEFAULTS "${CMAKE_CURRENT_LIST_DIR}/bsp/sdkconfig.defaults")
set(MCUHW_BOARD_IDF_BOOT "${CMAKE_CURRENT_LIST_DIR}/bsp/bootloader_components")
set(MCUHW_BOARD_IDF_START "${CMAKE_CURRENT_LIST_DIR}/bsp/app_start.cpp")
