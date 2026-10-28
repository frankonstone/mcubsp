    /* @req-file REQ-0053 REQ-0057 REQ-0073 */
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
    .word NMI_Handler /* NMI */
    .word HardFault_Handler /* HardFault */
    .word MemManage_Handler /* MemManage */
    .word BusFault_Handler /* BusFault */
    .word UsageFault_Handler /* UsageFault */
    .word 0, 0, 0, 0
    .word SVC_Handler /* SVCall */
    .word DebugMon_Handler /* DebugMon */
    .word 0
    .word PendSV_Handler /* PendSV */
    .word SysTick_Handler /* SysTick */
    /* All 48 nRF52840 external vector positions, including reserved slots.
       Startup enables no peripheral source; explicit runtime subscriptions own IRQs. */
    .rept 48
    .word mcubsp_promicro_irq_trampoline
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

    .align 2
    .thumb_func
mcubsp_promicro_irq_trampoline:
    push {r4, lr}
    mrs r0, ipsr
    subs r0, #16
    bl mcubsp_promicro_dispatch_irq
    pop {r4, pc}

    .align 2
    .thumb_func
mcubsp_promicro_fault_trampoline:
    mrs r0, ipsr
    b mcubsp_promicro_record_fault
    .weak NMI_Handler
    .thumb_set NMI_Handler, mcubsp_promicro_fault_trampoline
    .weak HardFault_Handler
    .thumb_set HardFault_Handler, mcubsp_promicro_fault_trampoline
    .weak MemManage_Handler
    .thumb_set MemManage_Handler, mcubsp_promicro_fault_trampoline
    .weak BusFault_Handler
    .thumb_set BusFault_Handler, mcubsp_promicro_fault_trampoline
    .weak UsageFault_Handler
    .thumb_set UsageFault_Handler, mcubsp_promicro_fault_trampoline
    .weak SVC_Handler
    .thumb_set SVC_Handler, Default_Handler
    .weak DebugMon_Handler
    .thumb_set DebugMon_Handler, Default_Handler
    .weak PendSV_Handler
    .thumb_set PendSV_Handler, Default_Handler
    .weak SysTick_Handler
    .thumb_set SysTick_Handler, Default_Handler
