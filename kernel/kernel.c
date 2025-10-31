#include <stdint.h>
#include "gfx.h"

// UART for debugging (VExpress-A9 UART0)
#define UART0_BASE 0x10009000
#define UART0_DR   (*(volatile uint32_t*)(UART0_BASE + 0x00))

void uart_putc(char c) {
    UART0_DR = c;
}

void uart_puts(const char* str) {
    while (*str) {
        uart_putc(*str++);
    }
}

void kernel_main() {
    uart_puts("Kernel started\n");
    
    // Initialize framebuffer: 640x480 @ 16bpp, auto-select VRAM
    fb_init(640, 480, 16, (void *)0x60000000);
    uart_puts("Framebuffer initialized\n");
    
    fb_clear(0x0020);  // Dark gray in 16-bit color
    uart_puts("Screen cleared\n");

    draw_rect(50, 50, 150, 100, 0xF800);    // Red
    draw_line(50, 50, 200, 150, 0x07E0);    // Green
    draw_circle(320, 240, 60, 0x001F);      // Blue
    
    uart_puts("Graphics drawn - check display\n");

    while (1);
}
