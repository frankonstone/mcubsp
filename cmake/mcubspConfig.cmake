# Source-tree package for examples and external projects using a checkout.
get_filename_component(MCUBSP_ROOT "${CMAKE_CURRENT_LIST_DIR}/.." ABSOLUTE)
if(NOT TARGET mcubsp::mcubsp)
    add_library(mcubsp::mcubsp INTERFACE IMPORTED)
    set_target_properties(mcubsp::mcubsp PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${MCUBSP_ROOT}/include"
        INTERFACE_COMPILE_FEATURES cxx_std_23)
    add_library(mcubsp::io INTERFACE IMPORTED)
    set_target_properties(mcubsp::io PROPERTIES INTERFACE_LINK_LIBRARIES mcubsp::mcubsp)
    add_library(mcubsp::device INTERFACE IMPORTED)
    set_target_properties(mcubsp::device PROPERTIES INTERFACE_LINK_LIBRARIES mcubsp::mcubsp)
endif()
