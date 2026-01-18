#!/bin/bash
# Universal MCU framebuffer dump script
# Works with various MCU boards and memory layouts

set -e

# Configuration (override via environment or command line)
MCU_TYPE="${MCU_TYPE:-cortex-m3}"
QEMU_MACHINE="${QEMU_MACHINE:-mps2-an385}"
KERNEL="${KERNEL:-kernel.elf}"
FB_ADDR="${FB_ADDR:-0x20000000}"  # Typical SRAM start for Cortex-M
WIDTH="${WIDTH:-640}"
HEIGHT="${HEIGHT:-480}"
BPP="${BPP:-16}"  # Bits per pixel (16 for RGB565, 32 for RGB888)
OUTPUT="${OUTPUT:-screenshot.png}"

# Calculate framebuffer size
if [ "$BPP" = "16" ]; then
    FB_SIZE=$((WIDTH * HEIGHT * 2))
elif [ "$BPP" = "32" ]; then
    FB_SIZE=$((WIDTH * HEIGHT * 4))
else
    echo "Error: Unsupported BPP: $BPP (supported: 16, 32)"
    exit 1
fi

# Calculate end address
FB_ADDR_DEC=$(printf "%d" $FB_ADDR)
FB_END=$(printf "0x%X" $((FB_ADDR_DEC + FB_SIZE)))

echo "========================================="
echo "MCU Framebuffer Dump Utility"
echo "========================================="
echo "MCU Type:    $MCU_TYPE"
echo "Machine:     $QEMU_MACHINE"
echo "Framebuffer: $FB_ADDR - $FB_END ($FB_SIZE bytes)"
echo "Resolution:  ${WIDTH}x${HEIGHT} @ ${BPP}bpp"
echo "========================================="
echo ""

# Cleanup previous runs
rm -f framebuffer.raw "$OUTPUT"

# Start QEMU in background
echo "Starting QEMU..."
qemu-system-arm \
  -M "$QEMU_MACHINE" \
  -cpu "$MCU_TYPE" \
  -kernel "$KERNEL" \
  -serial stdio \
  -s -S \
  -display none > /tmp/qemu.log 2>&1 &
QEMU_PID=$!

echo "QEMU started (PID: $QEMU_PID)"
echo "Waiting 2 seconds for initialization..."
sleep 2

# Check if QEMU is still running
if ! kill -0 $QEMU_PID 2>/dev/null; then
    echo "❌ Error: QEMU failed to start. Check /tmp/qemu.log"
    cat /tmp/qemu.log
    exit 1
fi

# Dump framebuffer via GDB
echo "Connecting GDB and dumping framebuffer..."
if arm-none-eabi-gdb -batch \
    -ex "target remote :1234" \
    -ex "dump binary memory framebuffer.raw $FB_ADDR $FB_END" \
    -ex "quit" \
    "$KERNEL" 2>/dev/null; then
    
    if [ -f framebuffer.raw ]; then
        ACTUAL_SIZE=$(stat -f%z framebuffer.raw 2>/dev/null || stat -c%s framebuffer.raw 2>/dev/null)
        if [ "$ACTUAL_SIZE" -eq "$FB_SIZE" ]; then
            echo "✅ Framebuffer dumped successfully ($ACTUAL_SIZE bytes)"
            
            # Convert to PNG
            if command -v python3 &> /dev/null; then
                echo "Converting to PNG..."
                if python3 utils/screenshot.py framebuffer.raw "$OUTPUT" "$WIDTH" "$HEIGHT"; then
                    if [ -f "$OUTPUT" ]; then
                        echo ""
                        echo "✅ SUCCESS! Screenshot saved to $OUTPUT"
                        if command -v open &> /dev/null; then
                            open "$OUTPUT"
                        elif command -v xdg-open &> /dev/null; then
                            xdg-open "$OUTPUT"
                        fi
                    else
                        echo "⚠️  Warning: Conversion completed but file not found"
                    fi
                else
                    echo "❌ Failed to convert framebuffer. Do you have PIL/Pillow installed?"
                    echo "   Install with: pip3 install pillow"
                    echo "   Raw framebuffer saved as: framebuffer.raw"
                fi
            else
                echo "⚠️  Python3 not found. Raw framebuffer saved as: framebuffer.raw"
            fi
        else
            echo "⚠️  Warning: Expected $FB_SIZE bytes, got $ACTUAL_SIZE bytes"
            echo "   Raw framebuffer saved as: framebuffer.raw"
        fi
    else
        echo "❌ Error: Failed to create framebuffer.raw"
    fi
else
    echo "❌ Error: GDB failed to connect or dump memory"
    echo "   Make sure:"
    echo "   1. QEMU is running with -s -S flags"
    echo "   2. arm-none-eabi-gdb is in your PATH"
    echo "   3. GDB can access the kernel.elf file"
fi

# Cleanup
echo ""
echo "Stopping QEMU..."
kill $QEMU_PID 2>/dev/null || true
wait $QEMU_PID 2>/dev/null || true

echo "Done."

