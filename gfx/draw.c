/*
 * Drawing primitives
 * Uses Bresenham algorithms for lines and circles
 */

#include "gfx.h"

static inline int abs(int a) {
    return a > 0 ? a : -a;
}

void draw_line(int x0, int y0, int x1, int y1, unsigned int color) {
    int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int dy = abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
    int err = dx - dy, e2;

    while (1) {
        fb_putpixel(x0, y0, color);
        if (x0 == x1 && y0 == y1) break;
        e2 = 2 * err;
        if (e2 > -dy) { err -= dy; x0 += sx; }
        if (e2 < dx) { err += dx; y0 += sy; }
    }
}

void draw_rect(int x, int y, int w, int h, unsigned int color) {
    // Top and bottom edges
    for (int i = 0; i < w; i++) {
        fb_putpixel(x + i, y, color);
        if (h > 1) {
            fb_putpixel(x + i, y + h - 1, color);
        }
    }
    // Left and right edges
    if (h > 2) {
        for (int i = 1; i < h - 1; i++) {
            fb_putpixel(x, y + i, color);
            fb_putpixel(x + w - 1, y + i, color);
        }
    }
}

void draw_circle(int cx, int cy, int r, unsigned int color) {
    int x = -r, y = 0, err = 2 - 2 * r;
    do {
        fb_putpixel(cx - x, cy + y, color);
        fb_putpixel(cx - y, cy - x, color);
        fb_putpixel(cx + x, cy - y, color);
        fb_putpixel(cx + y, cy + x, color);
        int e2 = err;
        if (e2 <= y) err += ++y * 2 + 1;
        if (e2 > x || err > y) err += ++x * 2 + 1;
    } while (x < 0);
}
