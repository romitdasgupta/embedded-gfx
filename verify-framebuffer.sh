#!/bin/bash
# Verify framebuffer rendering by capturing a screenshot
# Runs QEMU headless, waits for graphics to render, then captures to PNG

cd "$(dirname "$0")"

echo "========================================="
echo "Raspberry Pi 3B Framebuffer Verification"
echo "========================================="
echo ""

# Clean up old files
rm -f /tmp/qemu_screenshot.ppm /tmp/serial.log screenshot.png

echo "Building kernel..."
make clean && make || { echo "Build failed!"; exit 1; }

echo ""
echo "Running QEMU and capturing screenshot..."

# Run QEMU with delayed screendump
(
  sleep 4
  echo "screendump /tmp/qemu_screenshot.ppm"
  sleep 1
  echo "quit"
) | qemu-system-aarch64 -M raspi3b -kernel kernel8.img \
  -serial file:/tmp/serial.log \
  -display none \
  -monitor stdio 2>/dev/null

echo ""
echo "=== Serial Output ==="
cat /tmp/serial.log
echo ""

# Convert to PNG
if [ -f /tmp/qemu_screenshot.ppm ]; then
    if command -v python3 &> /dev/null; then
        python3 -c "
from PIL import Image
img = Image.open('/tmp/qemu_screenshot.ppm')
img.save('screenshot.png')
pixels = list(img.getdata())
non_black = sum(1 for p in pixels if any(c > 0 for c in p[:3]))
print(f'Screenshot saved: screenshot.png ({img.size[0]}x{img.size[1]})')
print(f'Non-black pixels: {non_black} / {len(pixels)}')
if non_black == len(pixels):
    print('✅ SUCCESS: All pixels rendered!')
elif non_black > 0:
    print('✅ SUCCESS: Graphics detected!')
else:
    print('❌ FAILURE: Screen is black')
"
        # Open on macOS
        if command -v open &> /dev/null; then
            open screenshot.png
        fi
    else
        echo "Python3 not found. Raw screenshot: /tmp/qemu_screenshot.ppm"
    fi
else
    echo "❌ Failed to capture screenshot"
    exit 1
fi
