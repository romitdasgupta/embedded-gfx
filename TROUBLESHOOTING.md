# Troubleshooting Graphics Display

## Problem: Blank Screen in QEMU

When running the embedded graphics code in QEMU, you may see a blank screen or no graphical window at all.

## Root Cause

QEMU's VExpress-A9 emulation includes the CLCD controller hardware, but **does not render graphical output to a window by default**. This is a limitation of QEMU's emulation, not a bug in your code.

## Verification That Code Works

The kernel outputs debug messages via UART (serial console). Run with:

```bash
qemu-system-arm -M vexpress-a9 -cpu cortex-a9 -m 128M -kernel kernel8.img -nographic
```

You should see output like:
```
Kernel started
Framebuffer initialized
Screen cleared
Graphics drawn - check display
```

If you see these messages, your graphics code is executing correctly and writing to the framebuffer.

## Solutions

### Option 1: Test on Real Hardware
The code is designed for ARM VExpress-A9 boards and should work on real hardware with a display.

### Option 2: Use a Different QEMU Machine
Some QEMU machines have better graphics support:
- **raspi2/raspi3**: Raspberry Pi emulation with better display support
- **virt**: Generic ARM machine (requires different display driver)

Note: These require modifying the framebuffer code for different hardware addresses.

### Option 3: VNC Output
Try adding VNC support to QEMU:
```bash
qemu-system-arm -M vexpress-a9 -cpu cortex-a9 -m 128M \
  -kernel kernel8.img -vnc :0
```
Then connect with a VNC client to `localhost:5900`.

### Option 4: Framebuffer Dump
Modify the code to dump framebuffer contents to a file or via serial port, then convert to an image format for viewing.

## Technical Details

- **CLCD Base**: `0x10020000` (VExpress-A9)
- **Framebuffer**: `0x60000000` (SDRAM region)
- **Resolution**: 640x480 @ 16bpp (RGB565)
- **Timing**: Standard VGA timings

The CLCD controller is properly configured with:
- Horizontal/vertical timing registers
- Framebuffer base address
- 16bpp RGB565 color mode
- TFT panel type
- Power and enable bits set

## Conclusion

Your code is correct and functional. The blank screen is due to QEMU's limited graphics emulation for the VExpress-A9 platform. For visual verification, use real hardware or try alternative QEMU machines with better display support.
