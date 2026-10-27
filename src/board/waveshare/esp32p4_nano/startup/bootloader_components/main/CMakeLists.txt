idf_component_register(
    SRCS "bootloader_start.cpp"
    REQUIRES bootloader bootloader_support
)
target_link_options(${COMPONENT_LIB} INTERFACE
    "LINKER:--dependency-file,${CMAKE_BINARY_DIR}/bootloader.elf.link.d")
