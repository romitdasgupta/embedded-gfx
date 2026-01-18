#!/bin/bash
# Test graphics output via VNC

echo "Starting QEMU with VNC on port 5900..."
echo "Connect with: vncviewer localhost:5900"
echo "Press Ctrl+C to stop"
echo ""

qemu-system-arm \
  -M vexpress-a9 \
  -cpu cortex-a9 \
  -m 128M \
  -kernel kernel8.img \
  -serial stdio \
  -vnc :0

# Note: VNC may still show blank screen due to QEMU's limited
# VExpress-A9 CLCD emulation, but worth trying
