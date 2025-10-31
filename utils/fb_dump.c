// Utility to dump framebuffer contents for verification
// Add this to kernel.c to output framebuffer as PPM image via UART

#include <stdint.h>

extern void uart_putc(char c);
extern void uart_puts(const char* str);

void uart_puthex(uint32_t val) {
    const char hex[] = "0123456789ABCDEF";
    for (int i = 7; i >= 0; i--) {
        uart_putc(hex[(val >> (i * 4)) & 0xF]);
    }
}

void dump_framebuffer_ppm(uint16_t* fb, int width, int height) {
    uart_puts("P3\n");
    uart_puts("640 480\n");
    uart_puts("255\n");
    
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            uint16_t pixel = fb[y * width + x];
            // Convert RGB565 to RGB888
            uint8_t r = ((pixel >> 11) & 0x1F) << 3;
            uint8_t g = ((pixel >> 5) & 0x3F) << 2;
            uint8_t b = (pixel & 0x1F) << 3;
            
            // Output as decimal (slow but readable)
            // In practice, you'd want binary output
            uart_puthex(r);
            uart_putc(' ');
            uart_puthex(g);
            uart_putc(' ');
            uart_puthex(b);
            uart_putc('\n');
        }
    }
}
