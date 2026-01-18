# Embedded Graphics Project

Bare-metal graphics demo for Raspberry Pi 3B, running on QEMU.

## Features

- 640x480 32-bit framebuffer via VideoCore mailbox protocol
- Drawing primitives: lines, rectangles, circles
- PL011 UART serial output for debugging
- Works in QEMU with visual output

## Prerequisites

### macOS (Homebrew)

```bash
brew install aarch64-elf-gcc qemu
pip3 install pillow  # For screenshot verification
```

### Linux (Debian/Ubuntu)

```bash
sudo apt install gcc-aarch64-linux-gnu qemu-system-aarch64 python3-pil
```

## Building

```bash
make
```

## Running

### With Graphical Display

```bash
make run
# Or:
./qemu-run.sh
```

A window will open showing the rendered graphics:
- Red rectangle (top-left)
- Green diagonal line
- Blue circle (center)
- Yellow circle (top-right)
- Magenta circle (bottom-left)
- Cyan rectangle (bottom-right)
- White border

### Headless Verification

To verify graphics without a display window:

```bash
./verify-framebuffer.sh
```

This captures a screenshot to `screenshot.png` and verifies the framebuffer has content.

## Project Structure

```
├── kernel/
│   ├── start.S       # AArch64 boot code
│   └── kernel.c      # Main kernel with UART and graphics demo
├── gfx/
│   ├── framebuffer.c # Mailbox-based framebuffer driver
│   └── draw.c        # Drawing primitives (Bresenham algorithms)
├── include/
│   └── gfx.h         # Graphics API
├── link.ld           # Linker script (loads at 0x80000)
└── Makefile          # Build system
```

## Technical Details

- **Target**: Raspberry Pi 3B (AArch64 Cortex-A53)
- **QEMU Machine**: `raspi3b`
- **Load Address**: 0x80000
- **Framebuffer**: Allocated by GPU via mailbox protocol
- **Serial**: PL011 UART at 0x3F201000
- **Pixel Format**: 32-bit BGR (Blue-Green-Red)

## Example Output

```
=== Raspberry Pi 3B Graphics Demo ===
Kernel started
Framebuffer initialized
Screen cleared
Drew red rectangle
Drew green line
Drew blue circle
Drew white border

*** Graphics complete! Check the QEMU window. ***
```
