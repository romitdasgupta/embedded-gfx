#!/bin/bash
# Test framebuffer by dumping memory and converting to image
# This verifies the code works even if QEMU display is garbled

echo "Starting QEMU with GDB server..."
echo "In another terminal, run:"
echo "  arm-none-eabi-gdb kernel.elf -x utils/screenshot.gdb"
echo ""
echo "Then run: python3 utils/screenshot.py framebuffer.raw screenshot.png"
echo "Press Ctrl+C to stop QEMU"
echo ""

qemu-system-arm \
  -M vexpress-a9 \
  -cpu cortex-a9 \
  -m 128M \
  -kernel kernel8.img \
  -serial stdio \
  -s -S \
  -display none

