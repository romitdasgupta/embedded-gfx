// gfx/framebuffer.c
#include "gfx.h"

// CLCD Controller registers for VExpress-A9
#define CLCD_BASE       0x10020000
#define CLCD_TIM0       (*(volatile uint32_t*)(CLCD_BASE + 0x00))
#define CLCD_TIM1       (*(volatile uint32_t*)(CLCD_BASE + 0x04))
#define CLCD_TIM2       (*(volatile uint32_t*)(CLCD_BASE + 0x08))
#define CLCD_TIM3       (*(volatile uint32_t*)(CLCD_BASE + 0x0C))
#define CLCD_UPBASE     (*(volatile uint32_t*)(CLCD_BASE + 0x10))
#define CLCD_LPBASE     (*(volatile uint32_t*)(CLCD_BASE + 0x14))
#define CLCD_CONTROL    (*(volatile uint32_t*)(CLCD_BASE + 0x18))
#define CLCD_IMSC       (*(volatile uint32_t*)(CLCD_BASE + 0x1C))

// Use VRAM at a safe location in SDRAM
#define VRAM_BASE       0x60000000

static uint32_t* fb;
static int width, height, pitch, bpp;

void fb_init(int w, int h, int depth, void* base) {
    width = w; height = h; bpp = depth;
    pitch = w * (depth / 8);
    
    // Use VRAM_BASE if no base provided
    if (base == 0) {
        base = (void*)VRAM_BASE;
    }
    fb = (uint32_t*)base;

    // Configure CLCD for 640x480 @ 16bpp (more compatible)
    // TIM0: Horizontal timing (HBP=40, HFP=24, HSW=96, PPL=640)
    CLCD_TIM0 = ((640/16 - 1) << 2) | ((24 - 1) << 8) | ((40 - 1) << 16) | ((96 - 1) << 24);

    // TIM1: Vertical timing (VBP=13, VFP=9, VSW=2, LPP=480)
    CLCD_TIM1 = (480 - 1) | ((9 - 1) << 10) | ((13 - 1) << 16) | ((2 - 1) << 24);

    // TIM2: Clock and signal polarities
    CLCD_TIM2 = ((640 - 1) << 16) | (1 << 26) | (1 << 27);

    // TIM3: Line end delay
    CLCD_TIM3 = 0;

    // Disable interrupts
    CLCD_IMSC = 0;

    // Set framebuffer address
    CLCD_UPBASE = (uint32_t)base;
    CLCD_LPBASE = (uint32_t)base;

    // Control: Enable, 16bpp (565), TFT, power on
    CLCD_CONTROL = (1 << 0)  |  // LCD enable
                   (1 << 11) |  // LCD power enable
                   (4 << 1)  |  // 16bpp 565 mode
                   (1 << 5);    // TFT panel type
}

void fb_clear(uint32_t color) {
    if (bpp == 16) {
        uint16_t* fb16 = (uint16_t*)fb;
        uint16_t color16 = (uint16_t)color;
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                fb16[y * width + x] = color16;
            }
        }
    } else {
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                fb[y * (pitch / 4) + x] = color;
            }
        }
    }
}

void fb_putpixel(int x, int y, uint32_t color) {
    if (x >= 0 && x < width && y >= 0 && y < height) {
        if (bpp == 16) {
            uint16_t* fb16 = (uint16_t*)fb;
            fb16[y * width + x] = (uint16_t)color;
        } else {
            fb[y * (pitch / 4) + x] = color;
        }
    }
}
