/*
 * Raspberry Pi 3B boot code (AArch64)
 * Entry point for the kernel
 */

.section ".text.boot"

.global _start

_start:
    // Read CPU ID from MPIDR_EL1
    mrs     x1, mpidr_el1
    and     x1, x1, #3
    cbz     x1, primary_cpu

    // Secondary CPUs: hang in low-power loop
secondary_hang:
    wfe
    b       secondary_hang

primary_cpu:
    // Set up stack pointer (stack grows down from 0x80000)
    ldr     x1, =_start
    mov     sp, x1

    // Clear BSS section
    ldr     x1, =__bss_start
    ldr     x2, =__bss_end
clear_bss:
    cmp     x1, x2
    b.ge    bss_done
    str     xzr, [x1], #8
    b       clear_bss

bss_done:
    // Jump to C kernel
    bl      kernel_main

    // If kernel_main returns, hang
hang:
    wfe
    b       hang
