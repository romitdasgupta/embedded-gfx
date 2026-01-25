# CLAUDE.md - AI Assistant Guide for embedded-gfx

This document provides essential context for AI assistants working with this bare-metal graphics project.

## Project Overview

This is a **bare-metal graphics demonstration** for Raspberry Pi 3B, running under QEMU emulation. It implements a 640x480 32-bit framebuffer with drawing primitives, communicating with hardware via the VideoCore mailbox protocol.

**Key Characteristics:**
- No operating system or standard library
- Direct hardware register access
- AArch64 (ARM64) Cortex-A53 target
- QEMU-based development and testing

## Repository Structure

```
embedded-gfx/
├── kernel/
│   ├── start.S          # AArch64 assembly boot code (entry point)
│   └── kernel.c         # Main kernel with UART and graphics demo
├── gfx/
│   ├── framebuffer.c    # Mailbox-based framebuffer driver
│   └── draw.c           # Drawing primitives (Bresenham algorithms)
├── include/
│   └── gfx.h            # Public graphics API header
├── utils/
│   ├── fb_dump.c        # Framebuffer dump utility
│   ├── screenshot.py    # Raw to PNG converter
│   └── screenshot.gdb   # GDB script for memory dumping
├── Makefile             # Build system
├── link.ld              # Linker script (loads at 0x80000)
├── qemu-run.sh          # QEMU runner scripts
└── verify-framebuffer.sh # Screenshot verification
```

## Build Commands

```bash
# Build the kernel image
make

# Run with QEMU graphical display
make run

# Run headless (UART output only)
make run-nographic

# Clean build artifacts
make clean

# Capture and verify framebuffer output
./verify-framebuffer.sh
```

## Development Workflow

1. **Edit** source files in `kernel/` or `gfx/`
2. **Build** with `make`
3. **Run** with `make run` (graphical) or `make run-nographic` (headless)
4. **Debug** via UART output printed to terminal
5. **Verify** graphics with `./verify-framebuffer.sh`

## Technical Specifications

| Specification | Value |
|---------------|-------|
| CPU | ARM Cortex-A53 (AArch64) |
| Platform | Raspberry Pi 3B |
| Load Address | 0x80000 |
| Display Resolution | 640 x 480 |
| Color Depth | 32-bit BGR |
| UART Base | 0x3F201000 (PL011) |
| Baud Rate | 115200 |
| GPU Interface | VideoCore mailbox (channel 8) |

## Code Architecture

### Boot Sequence (kernel/start.S)
1. Read CPU ID from MPIDR_EL1
2. Park secondary CPUs (wait-for-event loop)
3. Set up stack pointer (below 0x80000)
4. Clear BSS section
5. Jump to `kernel_main()`

### Main Kernel (kernel/kernel.c)
- `uart_init()` - Initialize PL011 UART
- `uart_putc()` / `uart_puts()` - Serial output for debugging
- `kernel_main()` - Entry point, orchestrates demo

### Framebuffer Driver (gfx/framebuffer.c)
- `fb_init()` - Allocate framebuffer via GPU mailbox
- `fb_clear()` - Fill screen with color
- `fb_putpixel()` - Write single pixel with bounds checking

### Drawing Primitives (gfx/draw.c)
- `draw_line()` - Bresenham line algorithm
- `draw_rect()` - Rectangle outline
- `draw_circle()` - Bresenham circle algorithm

## Coding Conventions

### Hardware Access
```c
// Use volatile for hardware registers
#define REG (*(volatile unsigned int *)0xADDRESS)

// Polling pattern for status flags
while (UART0_FR & UART0_FR_TXFF) { asm volatile("nop"); }
```

### Naming
- **Hardware registers:** `DEVICE_REGISTER` (uppercase)
- **Functions:** `module_function()` (lowercase with underscores)
- **Constants:** `DEFINE_NAME` (uppercase)

### Safety
- Always check bounds before pixel writes
- Use `volatile` for memory-mapped I/O
- Document color format (BGR vs RGB)

## Cross-Compiler Toolchain

**Required tools:**
- `aarch64-elf-gcc` - ARM64 cross-compiler
- `aarch64-elf-ld` - Linker
- `aarch64-elf-objcopy` - Binary converter
- `qemu-system-aarch64` - System emulator

**Installation:**
```bash
# macOS
brew install aarch64-elf-gcc qemu

# Linux (Debian/Ubuntu)
sudo apt install gcc-aarch64-linux-gnu qemu-system-aarch64 python3-pil
```

## Common Tasks for AI Assistants

### Adding a New Drawing Primitive
1. Add function declaration to `include/gfx.h`
2. Implement in `gfx/draw.c` using `fb_putpixel()`
3. Call from `kernel_main()` in `kernel/kernel.c`
4. Build and verify with `./verify-framebuffer.sh`

### Modifying Display Settings
- Resolution: Change `fb_init()` parameters in `kernel.c` and mailbox message in `framebuffer.c`
- Color depth: Modify depth property (16 or 32) in mailbox message

### Debugging Graphics Issues
1. Check UART output for initialization messages
2. Use `./verify-framebuffer.sh` to capture framebuffer
3. For deep debugging, use GDB with `utils/screenshot.gdb`

### Adding UART Debug Output
```c
uart_puts("Debug message\n");
```

## Key Files to Understand

| File | Purpose |
|------|---------|
| `kernel/start.S` | Assembly boot code - CPU setup |
| `kernel/kernel.c` | Main logic - modify for new features |
| `gfx/framebuffer.c` | GPU mailbox protocol - framebuffer allocation |
| `gfx/draw.c` | Drawing algorithms - add primitives here |
| `Makefile` | Build configuration |
| `link.ld` | Memory layout - rarely needs changes |

## Important Constraints

1. **No standard library** - Cannot use `printf`, `malloc`, etc.
2. **No floating point** - Use integer-only algorithms
3. **Fixed memory layout** - Kernel loads at 0x80000
4. **BGR color order** - Hardware expects Blue-Green-Red byte order
5. **16-byte alignment** - Mailbox messages must be aligned

## Testing Graphics Output

**Method 1: QEMU Screenshot (Recommended)**
```bash
./verify-framebuffer.sh
```

**Method 2: GDB Memory Dump**
```bash
./test-framebuffer-dump.sh
```

**Method 3: VNC Connection**
```bash
./test-vnc.sh
# Connect VNC client to localhost:5900
```

## Expected Demo Output

The graphics demo draws:
- Dark blue background (0x00102030)
- Red rectangle at (50, 50) - 150x100px
- Green diagonal line from (50,50) to (200,150)
- Blue circle at center (320,240) - radius 80
- White border around entire screen
- Yellow circle at (500,100) - radius 40
- Magenta circle at (140,380) - radius 50
- Cyan rectangle at (400,300) - 100x80px

## Documentation References

- `README.md` - Quick start guide
- `GRAPHICS_OUTPUT_GUIDE.md` - Graphics verification methods
- `FLOW_DIAGRAM.md` - Execution flow diagrams
- `MCU_DISPLAY_TESTING.md` - Advanced testing guide
- `TROUBLESHOOTING.md` - Common problems and solutions

## Git Workflow

- Main development branch: `main`
- Feature branches: `claude/` prefix for AI-generated changes
- Commit messages: Descriptive, imperative mood

## Quick Reference

```bash
# Full build and run cycle
make clean && make && make run

# Headless test with framebuffer capture
make && ./verify-framebuffer.sh

# View serial output only
make run-nographic
```
