#ifndef GFX_H
#define GFX_H

/* Framebuffer functions */
void fb_init(int w, int h, int bpp, void* base);
void fb_clear(unsigned int color);
void fb_putpixel(int x, int y, unsigned int color);

/* Drawing primitives */
void draw_line(int x0, int y0, int x1, int y1, unsigned int color);
void draw_rect(int x, int y, int w, int h, unsigned int color);
void draw_circle(int cx, int cy, int r, unsigned int color);

#endif
