    .syntax unified
    .cpu cortex-m7
    .fpu fpv5-d16
    .thumb

    .section .vectors, "a", %progbits
    .align 10
    .global __isr_vector
__isr_vector:
    .word __stack
    .word Reset_Handler
    .rept 14
    .word Default_Handler
    .endr
    /* STM32H753 has external interrupt positions 0 through 149. */
    .rept 150
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

    /* Own the vector table from the first instruction after reset. */
    ldr r0, =0xE000ED08
    ldr r1, =__isr_vector
    str r1, [r0]
    dsb sy
    isb sy

    /* RM0433 6.4.1/6.8.4: select the board's LDO supply and wait for
       ACTVOSRDY before the first AXI SRAM write. */
    ldr r0, =0x5802480C
    movs r1, #6
    str r1, [r0]
    ldr r0, =0x58024804
    ldr r2, =1000000
1:
    ldr r1, [r0]
    tst r1, #0x2000
    bne 2f
    subs r2, r2, #1
    bne 1b
    b Default_Handler
2:
    ldr r0, =__data_load
    ldr r1, =__data_start
    ldr r2, =__data_end
3:
    cmp r1, r2
    bhs 4f
    ldr r3, [r0], #4
    str r3, [r1], #4
    b 3b
4:
    movs r0, #0
    ldr r1, =__bss_start
    ldr r2, =__bss_end
5:
    cmp r1, r2
    bhs 6f
    str r0, [r1], #4
    b 5b
6:
    bl mcuhw_nucleo_early_init

    ldr r4, =__init_array_start
    ldr r5, =__init_array_end
7:
    cmp r4, r5
    bhs 8f
    ldr r0, [r4], #4
    blx r0
    b 7b
8:
    movs r0, #0
    movs r1, #0
    bl main
9:
    b 9b

    .align 2
    .thumb_func
    .global Default_Handler
Default_Handler:
    bkpt #0
    b .
