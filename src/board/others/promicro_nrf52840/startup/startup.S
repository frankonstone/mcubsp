    /* @req-file REQ-0053 */
    .syntax unified
    .cpu cortex-m4
    .fpu fpv4-sp-d16
    .thumb

    .section .vectors, "a", %progbits
    .balign 256
    .global __isr_vector
__isr_vector:
    .word __stack
    .word Reset_Handler
    .word Default_Handler /* NMI */
    .word Default_Handler /* HardFault */
    .word Default_Handler /* MemManage */
    .word Default_Handler /* BusFault */
    .word Default_Handler /* UsageFault */
    .word 0, 0, 0, 0
    .word Default_Handler /* SVCall */
    .word Default_Handler /* DebugMon */
    .word 0
    .word Default_Handler /* PendSV */
    .word Default_Handler /* SysTick */
    /* All 48 nRF52840 external vector positions, including reserved slots.
       GPIO-only startup enables no peripheral interrupt. */
    .rept 48
    .word Default_Handler
    .endr

    .section .text.Reset_Handler, "ax", %progbits
    .align 2
    .thumb_func
    .global Reset_Handler
Reset_Handler:
    cpsid i
    ldr r0, =__stack
    msr msp, r0

    /* Quiesce inherited core interrupt delivery before relocating vectors. */
    ldr r0, =0xE000E010 /* SysTick CTRL */
    movs r1, #0
    str r1, [r0]
    ldr r0, =0xE000ED04 /* ICSR: clear PendSV and pending SysTick */
    ldr r1, =0x0A000000
    str r1, [r0]
    ldr r0, =0xE000E180 /* NVIC ICER */
    ldr r1, =0xFFFFFFFF
    str r1, [r0]
    str r1, [r0, #4]
    ldr r0, =0xE000E280 /* NVIC ICPR */
    str r1, [r0]
    str r1, [r0, #4]

    /* Enable CP10/11 before hard-float C++ and own the application vectors. */
    ldr r0, =0xE000ED88
    ldr r1, [r0]
    ldr r2, =0x00F00000
    orrs r1, r1, r2
    str r1, [r0]
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
    bl mcubsp_promicro_early_init
    ldr r4, =__init_array_start
    ldr r5, =__init_array_end
5:
    cmp r4, r5
    bhs 6f
    ldr r0, [r4], #4
    blx r0
    b 5b
6:
    cpsie i
    movs r0, #0
    movs r1, #0
    bl main
    b .

    .align 2
    .thumb_func
    .global Default_Handler
Default_Handler:
    b .
