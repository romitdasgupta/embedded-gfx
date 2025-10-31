# Embedded Graphics Project

This project contains embedded graphics code for ARM-based systems.

## Prerequisites

### Toolchain Installation

#### Linux (Debian/Ubuntu)

Install the required ARM toolchain and development tools:

```bash
sudo apt install gcc-arm-none-eabi binutils-arm-none-eabi qemu-system-arm gdb-multiarch make
```

#### macOS (using Homebrew)

Install the required ARM toolchain and development tools:

```bash
brew install arm-none-eabi-gcc qemu gdb make
```

**Note**: On macOS, `gdb` is installed instead of `gdb-multiarch`. You may need to codesign gdb for it to work:
```bash
# If gdb fails to run, you may need to codesign it:
sudo codesign --entitlements - --force --sign - $(which arm-none-eabi-gdb)
```

#### Package Details

- **gcc-arm-none-eabi** / **arm-none-eabi-gcc**: ARM cross-compiler for bare-metal targets
- **binutils-arm-none-eabi**: Binary utilities (assembler, linker, etc.) for ARM
- **qemu-system-arm**: ARM system emulator for testing without hardware
- **gdb-multiarch** (Linux) / **arm-none-eabi-gdb** (macOS): Debugger with support for multiple architectures including ARM
- **make**: Build automation tool

## Getting Started

After installing the toolchain, you can build and run the project using the provided Makefile.

```bash
make
```

## Running

To run with QEMU:

```bash
make run
```

Or use the provided script:

```bash
./qemu-run.sh
```

## Important Note: Graphics Display

**QEMU's VExpress-A9 emulation has limited graphics support.** While the CLCD controller is emulated, QEMU may not display a graphical window by default. The code is working correctly (verified via UART output), but you may see a blank screen or no window at all.

### Verification
The kernel outputs debug messages to the serial console:
- "Kernel started"
- "Framebuffer initialized"
- "Screen cleared"
- "Graphics drawn - check display"

If you see these messages, the graphics code is executing correctly.

### Alternatives for Visual Output
1. **Use real hardware**: VersatilePB or VExpress-A9 boards
2. **Try different QEMU machine**: `-M raspi2` or `-M raspi3b` (requires code changes)
3. **Use VNC**: Add `-vnc :0` to QEMU command and connect with VNC viewer
4. **Framebuffer dump**: Add code to dump framebuffer contents to a file

## Development

Use QEMU to test your code without physical hardware, and gdb-multiarch (Linux) or arm-none-eabi-gdb (macOS) for debugging ARM binaries.
