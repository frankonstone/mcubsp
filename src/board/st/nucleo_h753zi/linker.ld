/* NUCLEO-H753ZI bare-metal image. The board boots user flash directly at
 * 0x08000000; initial data and the stack use the 512 KiB AXI SRAM. */
ENTRY(Reset_Handler)

MEMORY
{
    FLASH (rx)  : ORIGIN = 0x08000000, LENGTH = 2048K
    RAM   (rwx) : ORIGIN = 0x24000000, LENGTH = 512K
}

__stack = ORIGIN(RAM) + LENGTH(RAM);
__stack_limit = __stack - 16K;

SECTIONS
{
    .vectors : ALIGN(1024)
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

    .init_array ALIGN(4) (READONLY) :
    {
        __init_array_start = .;
        KEEP(*(SORT(.init_array.*)))
        KEEP(*(.init_array))
        __init_array_end = .;
    } > FLASH

    mcucov_modules (READONLY) :
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

ASSERT(__image_ram_end <= __stack_limit,
       "NUCLEO-H753ZI AXI SRAM overlaps 16 KiB stack")
