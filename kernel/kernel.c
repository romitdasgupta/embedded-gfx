#include <stdint.h>
#include "gfx.h"

void kernel_main() {
    fb_init(800, 600, 32, (void *)0x10020000);
    fb_clear(0xFF202020);

    draw_rect(100, 100, 200, 150, 0xFFFF0000);
    draw_line(100, 100, 300, 250, 0xFF00FF00);
    draw_circle(400, 300, 80, 0xFF0000FF);

    while (1);
}
