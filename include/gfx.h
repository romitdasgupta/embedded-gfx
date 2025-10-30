#ifndef GFX_H
#define GFX_H

#include <stdint.h>

void fb_init(int w, int h, int bpp, void* base);
void fb_clear(uint32_t color);
void fb_putpixel(int x, int y, uint32_t color);

void draw_line(int x0, int y0, int x1, int y1, uint32_t color);
void draw_rect(int x, int y, int w, int h, uint32_t color);
void draw_circle(int cx, int cy, int r, uint32_t color);

#endif
