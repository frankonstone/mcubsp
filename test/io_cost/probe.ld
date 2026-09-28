/* Linked size probes only. No vectors or startup: do not flash these ELFs. */
ENTRY(mcuhw_io_cost_entry)
MEMORY
{
    FLASH (rx) : ORIGIN = 0x00004000, LENGTH = 240K
    RAM (rwx) : ORIGIN = 0x20000000, LENGTH = 32K
}
SECTIONS
{
    .text : { *(.text*) *(.rodata*) } > FLASH
    .ARM.extab : { *(.ARM.extab*) } > FLASH
    .ARM.exidx : { *(.ARM.exidx*) } > FLASH
    .init_array : { KEEP(*(.init_array*)) } > FLASH
    .data : { *(.data*) } > RAM AT > FLASH
    .bss (NOLOAD) : { *(.bss*) *(COMMON) } > RAM
    /DISCARD/ : { *(.comment) *(.note*) }
}
