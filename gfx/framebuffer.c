// gfx/framebuffer.c
#include "gfx.h"

static uint32_t* fb;
static int width, height, pitch, bpp;

void fb_init(int w, int h, int depth, void* base) {
    width = w; height = h; bpp = depth;
    pitch = w * (depth / 8);
    fb = (uint32_t*)base;
}

void fb_clear(uint32_t color) {
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            fb[y * (pitch / 4) + x] = color;
        }
    }
}

void fb_putpixel(int x, int y, uint32_t color) {
    if (x >= 0 && x < width && y >= 0 && y < height) {
        fb[y * (pitch / 4) + x] = color;
    }
}
