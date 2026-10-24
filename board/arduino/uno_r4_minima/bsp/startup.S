    .syntax unified
    .cpu cortex-m4
    .fpu fpv4-sp-d16
    .thumb

    .section .vectors, "a", %progbits
    .align 8
    .global __isr_vector
__isr_vector:
    .word __stack
    .word Reset_Handler
    .word Default_Handler
    .word Default_Handler
    .word Default_Handler
    .word Default_Handler
    .word Default_Handler
    .word 0
    .word 0
    .word 0
    .word 0
    .word Default_Handler
    .word Default_Handler
    .word 0
    .word Default_Handler
    .word Default_Handler

    /* Each test image may provide strong handlers for the first four
       selectable ICU lines. The weak defaults trap unused lines. */
    .word IRQ0_Handler
    .word IRQ1_Handler
    .word IRQ2_Handler
    .word IRQ3_Handler
    .rept 28
    .word Default_Handler
    .endr

    .section .text.Reset_Handler, "ax", %progbits
    .align 2
    .thumb_func
    .global Reset_Handler
Reset_Handler:
    /* Enable CP10/CP11 before any hard-float C++ can execute. */
    ldr r0, =0xE000ED88
    ldr r1, [r0]
    ldr r2, =0x00F00000
    orrs r1, r1, r2
    str r1, [r0]
    dsb sy
    isb sy

    /* This image owns its vector table even though the DFU bootloader jumps
       to it rather than a hardware reset fetching it directly. */
    ldr r0, =0xE000ED08
    ldr r1, =__isr_vector
    str r1, [r0]
    dsb sy
    isb sy

    ldr r0, =__data_load
    ldr r1, =__data_start
    ldr r2, =__data_end
1:
    cmp r1, r2
    bhs 2f
    ldr r3, [r0], #4
    str r3, [r1], #4
    b 1b
2:
    movs r0, #0
    ldr r1, =__bss_start
    ldr r2, =__bss_end
3:
    cmp r1, r2
    bhs 4f
    str r0, [r1], #4
    b 3b
4:
    bl mcubsp_uno_early_init

    ldr r4, =__init_array_start
    ldr r5, =__init_array_end
5:
    cmp r4, r5
    bhs 6f
    ldr r0, [r4], #4
    blx r0
    b 5b
6:
    movs r0, #0
    movs r1, #0
    bl main
7:
    b 7b

    .align 2
    .thumb_func
    .global Default_Handler
Default_Handler:
    b .

    .weak IRQ0_Handler
    .thumb_set IRQ0_Handler, Default_Handler
    .weak IRQ1_Handler
    .thumb_set IRQ1_Handler, Default_Handler
    .weak IRQ2_Handler
    .thumb_set IRQ2_Handler, Default_Handler
    .weak IRQ3_Handler
    .thumb_set IRQ3_Handler, Default_Handler
