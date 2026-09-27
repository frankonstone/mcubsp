# Per-MCU sources are manifest-owned; shared description contracts stay local.
mcuhw_component_suites(mcu mcu)
mcuhw_native_suite(descriptions
    test_description_suite.cpp
    test_three_description_gate.cpp)
