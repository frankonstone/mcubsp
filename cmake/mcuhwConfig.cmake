# Source-tree package for examples and external projects using a checkout.
get_filename_component(MCUHW_ROOT "${CMAKE_CURRENT_LIST_DIR}/.." ABSOLUTE)
if(NOT TARGET mcuhw::mcuhw)
    add_library(mcuhw::mcuhw INTERFACE IMPORTED)
    set_target_properties(mcuhw::mcuhw PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${MCUHW_ROOT}/include"
        INTERFACE_COMPILE_FEATURES cxx_std_23)
    add_library(mcuhw::io INTERFACE IMPORTED)
    set_target_properties(mcuhw::io PROPERTIES INTERFACE_LINK_LIBRARIES mcuhw::mcuhw)
    add_library(mcuhw::device INTERFACE IMPORTED)
    set_target_properties(mcuhw::device PROPERTIES INTERFACE_LINK_LIBRARIES mcuhw::mcuhw)
endif()
include("${MCUHW_ROOT}/cmake/McuhwBoard.cmake")
