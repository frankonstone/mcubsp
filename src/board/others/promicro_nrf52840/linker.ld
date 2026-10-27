/* @req REQ-0053
 * Zephyr nrf52840_partition_uf2_sdv6.dtsi: preserve SoftDevice below 0x26000,
 * storage at 0xEC000, and the bootloader at 0xF4000. Bootloader handoff only. */
ENTRY(Reset_Handler)

MEMORY
{
    FLASH (rx) : ORIGIN = 0x00026000, LENGTH = 0x000C6000
    RAM (rwx)  : ORIGIN = 0x20000000, LENGTH = 0x00040000
}

__stack = ORIGIN(RAM) + LENGTH(RAM);
__stack_limit = __stack - 0x1000;

SECTIONS
{
    .vectors : ALIGN(256) { KEEP(*(.vectors)) } > FLASH
    .text : { *(.text*) *(.rodata*) KEEP(*(.init)) KEEP(*(.fini)) } > FLASH
    .ARM.extab : { *(.ARM.extab*) } > FLASH
    __exidx_start = .;
    .ARM.exidx : { *(.ARM.exidx*) } > FLASH
    __exidx_end = .;
    .init_array (READONLY) :
    {
        . = ALIGN(4);
        __init_array_start = .;
        KEEP(*(SORT(.init_array.*))) KEEP(*(.init_array))
        __init_array_end = .;
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
        *(.bss*) *(COMMON)
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
ASSERT(ADDR(.vectors) == 0x26000, "Pro Micro UF2 vectors must begin at 0x26000")
ASSERT(SIZEOF(.vectors) == 256, "nRF52840 requires 16 core and 48 external vectors")
ASSERT(__image_ram_end <= __stack_limit, "Pro Micro RAM overlaps 4 KiB stack")
