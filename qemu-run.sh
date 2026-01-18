#!/bin/bash
# Run the Raspberry Pi 3B graphics demo in QEMU
# This will open a graphical window showing the rendered shapes

cd "$(dirname "$0")"

echo "Starting Raspberry Pi 3B graphics demo..."
echo "A window should open showing colorful shapes."
echo "Press Ctrl+C to exit."
echo ""

qemu-system-aarch64 \
  -M raspi3b \
  -kernel kernel8.img \
  -serial stdio \
  -display cocoa
