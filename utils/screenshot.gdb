# GDB script to dump framebuffer and convert to image
# Usage: 
#   Terminal 1: qemu-system-arm -M vexpress-a9 -cpu cortex-a9 -m 128M -kernel kernel8.img -s -S
#   Terminal 2 (Linux): gdb-multiarch kernel.elf -x utils/screenshot.gdb
#   Terminal 2 (macOS): arm-none-eabi-gdb kernel.elf -x utils/screenshot.gdb

target remote :1234
continue

# Wait a bit for graphics to be drawn
# Then break and dump framebuffer
break kernel_main
continue
finish

# Dump framebuffer memory (640x480x2 bytes = 614400 bytes)
dump binary memory framebuffer.raw 0x4C000000 0x4C096000

# Convert with Python script (see screenshot.py)
quit
