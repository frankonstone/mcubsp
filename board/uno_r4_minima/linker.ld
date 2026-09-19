/* Bare-metal UNO R4 Minima image that preserves Arduino's DFU bootloader.
 * ArduinoCore-renesas 1.6.0 links applications at 0x4000; the shipped
 * bootloader image occupies 0x0000..0x3087. */
ENTRY(Reset_Handler)

MEMORY
{
    FLASH (rx)  : ORIGIN = 0x00004000, LENGTH = 0x0003C000
    RAM   (rwx) : ORIGIN = 0x20000000, LENGTH = 0x00008000
}

__stack = ORIGIN(RAM) + LENGTH(RAM);
__stack_limit = __stack - 0x1000;

SECTIONS
{
    .vectors : ALIGN(256)
    {
        KEEP(*(.vectors))
    } > FLASH

    .text :
    {
        *(.text*)
        *(.rodata*)
        KEEP(*(.init))
        KEEP(*(.fini))
    } > FLASH

    .ARM.extab : { *(.ARM.extab*) } > FLASH
    __exidx_start = .;
    .ARM.exidx : { *(.ARM.exidx*) } > FLASH
    __exidx_end = .;

    .init_array (READONLY) :
    {
        . = ALIGN(4);
        __init_array_start = .;
        KEEP(*(SORT(.init_array.*)))
        KEEP(*(.init_array))
        __init_array_end = .;
    } > FLASH

    mcucov_modules :
    {
        . = ALIGN(4);
        __start_mcucov_modules = .;
        KEEP(*(mcucov_modules))
        __stop_mcucov_modules = .;
    } > FLASH

    .data :
    {
        . = ALIGN(4);
        __data_start = .;
        *(.data*)
        . = ALIGN(4);
        __data_end = .;
    } > RAM AT > FLASH
    __data_load = LOADADDR(.data);

    .bss (NOLOAD) :
    {
        . = ALIGN(8);
        __bss_start = .;
        *(.bss*)
        *(COMMON)
        . = ALIGN(8);
        __bss_end = .;
    } > RAM

    .noinit (NOLOAD) :
    {
        . = ALIGN(8);
        *(.noinit*)
        . = ALIGN(8);
        __image_ram_end = .;
        end = .;
        _end = .;
    } > RAM

    /DISCARD/ : { *(.comment*) *(.note*) }
}

ASSERT(__image_ram_end <= __stack_limit, "UNO R4 Minima RAM overlaps 4 KiB stack")
