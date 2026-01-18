/*
 * Raspberry Pi 3B Kernel
 * Graphics demo with UART debug output
 */

#include "gfx.h"

// Raspberry Pi 3B peripheral base
#define MMIO_BASE       0x3F000000

// PL011 UART registers (this is what QEMU uses for serial)
#define UART0_BASE      (MMIO_BASE + 0x00201000)
#define UART0_DR        (*(volatile unsigned int*)(UART0_BASE + 0x00))
#define UART0_FR        (*(volatile unsigned int*)(UART0_BASE + 0x18))
#define UART0_IBRD      (*(volatile unsigned int*)(UART0_BASE + 0x24))
#define UART0_FBRD      (*(volatile unsigned int*)(UART0_BASE + 0x28))
#define UART0_LCRH      (*(volatile unsigned int*)(UART0_BASE + 0x2C))
#define UART0_CR        (*(volatile unsigned int*)(UART0_BASE + 0x30))
#define UART0_ICR       (*(volatile unsigned int*)(UART0_BASE + 0x44))

#define UART0_FR_TXFF   (1 << 5)  // Transmit FIFO full

void uart_init(void) {
    // Disable UART
    UART0_CR = 0;
    
    // Clear pending interrupts
    UART0_ICR = 0x7FF;
    
    // Set baud rate (115200 at 48MHz clock = IBRD=26, FBRD=3)
    // For QEMU, these values don't matter much
    UART0_IBRD = 26;
    UART0_FBRD = 3;
    
    // 8 bits, no parity, 1 stop bit, enable FIFOs
    UART0_LCRH = (1 << 4) | (1 << 5) | (1 << 6);  // FIFO enable, 8-bit
    
    // Enable UART, TX and RX
    UART0_CR = (1 << 0) | (1 << 8) | (1 << 9);  // UARTEN, TXE, RXE
}

void uart_putc(char c) {
    // Wait for transmit FIFO to have space
    while (UART0_FR & UART0_FR_TXFF) {
        asm volatile("nop");
    }
    UART0_DR = c;
}

void uart_puts(const char* str) {
    while (*str) {
        if (*str == '\n') {
            uart_putc('\r');
        }
        uart_putc(*str++);
    }
}

void kernel_main(void) {
    uart_init();
    uart_puts("\n=== Raspberry Pi 3B Graphics Demo ===\n");
    uart_puts("Kernel started\n");
    
    // Initialize framebuffer: 640x480 @ 32bpp (better support on RPi)
    fb_init(640, 480, 32, 0);
    uart_puts("Framebuffer initialized\n");
    
    // Clear screen with dark blue
    fb_clear(0x00102030);
    uart_puts("Screen cleared\n");

    // Draw some graphics - using 32-bit BGR colors (RPi framebuffer is BGR)
    draw_rect(50, 50, 150, 100, 0x000000FF);     // Red rectangle (BGR: 0x0000FF)
    uart_puts("Drew red rectangle\n");
    
    draw_line(50, 50, 200, 150, 0x0000FF00);     // Green diagonal line (BGR: 0x00FF00)
    uart_puts("Drew green line\n");
    
    draw_circle(320, 240, 80, 0x00FF0000);       // Blue circle (BGR: 0xFF0000)
    uart_puts("Drew blue circle\n");
    
    // Draw a white border around the screen
    draw_rect(0, 0, 640, 480, 0x00FFFFFF);
    uart_puts("Drew white border\n");
    
    // Draw some additional shapes for visual interest
    draw_circle(500, 100, 40, 0x0000FFFF);       // Yellow circle (BGR: 0x00FFFF)
    draw_circle(140, 380, 50, 0x00FF00FF);       // Magenta circle (BGR: 0xFF00FF)
    draw_rect(400, 300, 100, 80, 0x00FFFF00);    // Cyan rectangle (BGR: 0xFFFF00)
    
    uart_puts("\n*** Graphics complete! Check the QEMU window. ***\n");

    // Infinite loop
    while (1) {
        asm volatile("wfe");
    }
}
