# Per-MCU test sources stay local; shared description contracts have one suite.
foreach(pair IN ITEMS "esp32p4;espressif/esp32p4" "ra4m1;renesas/ra4m1" "stm32h753;st/stm32h753")
    list(GET pair 0 directory)
    list(GET pair 1 identifier)
    file(GLOB sources CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/${directory}/*.cpp")
    mcuhw_native_suite("mcu:${identifier}" ${sources})
endforeach()
mcuhw_native_suite(descriptions
    test_description_suite.cpp
    test_three_description_gate.cpp)
