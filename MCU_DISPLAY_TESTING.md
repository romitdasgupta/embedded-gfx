# Best Practices: Testing MCU Display Graphics with QEMU

## Overview

Testing embedded display graphics for **Microcontroller Units (MCUs)** differs from SoCs because:
- MCUs have simpler display interfaces (SPI, I2C, parallel)
- Often use external display driver chips (ST7789, ILI9341, SSD1306)
- May not have framebuffers (direct pixel-by-pixel rendering)
- Limited memory (smaller buffers or no buffers at all)

## Method 1: Memory Dump via GDB (RECOMMENDED)

**Best for**: Any MCU, especially those without built-in framebuffers

This is your current approach and works excellently for MCUs. The key advantage is that it doesn't rely on QEMU's display emulation at all.

### How It Works
1. Your code writes to memory (framebuffer or display buffer)
2. GDB connects to QEMU and dumps that memory region
3. Convert raw memory dump to image format (PNG, PPM)

### Implementation Tips

#### For MCUs with Framebuffers
```bash
# Dump framebuffer memory (adjust address/size for your MCU)
arm-none-eabi-gdb -batch -ex "target remote :1234" \
  -ex "dump binary memory framebuffer.raw 0x20000000 0x2004B000" \
  kernel.elf
```

#### For MCUs with External Display Controllers
Many MCUs use SPI/I2C display drivers. Options:

**A. Monitor SPI/I2C Transactions**
- Use QEMU trace to capture display commands
- Parse commands to reconstruct image
- Requires display driver knowledge

**B. Use a Software Framebuffer**
- Allocate memory buffer in your code
- Render to buffer instead of directly to display
- Dump buffer via GDB
- More memory usage but easier testing

**C. Semihosting File Output**
- Write framebuffer directly to host file system
- No GDB needed
- Requires semihosting enabled

### Enhanced Dump Script Template

```bash
#!/bin/bash
# Enhanced MCU framebuffer dump script

MCU_TYPE="cortex-m3"  # or cortex-m4, cortex-m0, etc.
QEMU_MACHINE="mps2-an385"  # Adjust for your MCU board
KERNEL="kernel.elf"
FB_ADDR="0x20000000"  # Your framebuffer address
FB_SIZE=614400        # 640*480*2 for RGB565

# Calculate end address
FB_END=$(printf "0x%X" $(( $(printf "%d" $FB_ADDR) + $FB_SIZE ))")

# Start QEMU with GDB server
qemu-system-arm \
  -M $QEMU_MACHINE \
  -cpu $MCU_TYPE \
  -kernel $KERNEL \
  -s -S \
  -display none &

QEMU_PID=$!
sleep 2

# Dump via GDB
arm-none-eabi-gdb -batch \
  -ex "target remote :1234" \
  -ex "dump binary memory framebuffer.raw $FB_ADDR $FB_END" \
  -ex "quit" \
  $KERNEL

# Convert to PNG
if [ -f framebuffer.raw ]; then
    python3 utils/screenshot.py framebuffer.raw screenshot.png
fi

kill $QEMU_PID
```

## Method 2: Semihosting File Output

**Best for**: MCUs with semihosting support (Cortex-M, most ARM MCUs)

Write framebuffer directly to host filesystem without GDB.

### QEMU Setup
```bash
qemu-system-arm \
  -M mps2-an385 \
  -cpu cortex-m3 \
  -kernel kernel.elf \
  -semihosting-config enable=on,target=native
```

### Code Implementation
```c
// Write framebuffer to file via semihosting
void dump_fb_semihosting(uint16_t* fb, int width, int height) {
    FILE *f = fopen("/tmp/framebuffer.raw", "wb");
    if (f) {
        fwrite(fb, width * height * 2, 1, f);  // RGB565 = 2 bytes/pixel
        fclose(f);
    }
}
```

**Pros**: Simple, no GDB needed  
**Cons**: Requires semihosting (slightly slower), filesystem access

## Method 3: QEMU Display Devices (Limited MCU Support)

**Best for**: MCUs that QEMU can emulate with display hardware

### Supported MCU Boards with Display
QEMU support for MCU displays is limited. Check:

- **STM32 Boards**: Some STM32F boards have LCD emulation
- **nRF52**: Limited display support
- **ESP32**: Better support but more SoC-like

### Example: STM32F4 Discovery
```bash
qemu-system-arm \
  -M stm32f4-discovery \
  -cpu cortex-m4 \
  -kernel kernel.elf \
  -display sdl  # or gtk, vnc
```

**Note**: Most MCU boards don't have full display emulation in QEMU.

## Method 4: Peripheral Emulation for External Displays

**Best for**: MCUs using SPI/I2C display drivers (ST7789, ILI9341, SSD1306)

### Approach
1. **Monitor SPI/I2C Transactions**: Use QEMU tracing
   ```bash
   qemu-system-arm -M mps2-an385 \
     -kernel kernel.elf \
     -trace spi*,i2c*
   ```

2. **Parse Display Commands**: Write parser to reconstruct image from commands
   - Requires understanding of display driver protocol
   - Complex but most accurate

3. **Software Buffer Approach** (Easier):
   ```c
   // Instead of direct SPI write
   void lcd_write_pixel(int x, int y, uint16_t color) {
       // Write to hardware (SPI)
       spi_write_color(color);
       
       // ALSO write to software buffer for testing
       #ifdef TESTING
       test_framebuffer[y * WIDTH + x] = color;
       #endif
   }
   ```

## Method 5: Unit Testing with Framebuffer Verification

**Best for**: Continuous testing, regression testing

Test graphics functions independently:

```c
// test_graphics.c
void test_draw_rect(void) {
    uint16_t fb[640*480] = {0};
    
    // Initialize test framebuffer
    test_fb_init(fb, 640, 480);
    
    // Call your drawing function
    draw_rect(10, 10, 100, 50, 0xF800);  // Red
    
    // Verify pixels
    assert(fb[10 * 640 + 10] == 0xF800);  // Top-left
    assert(fb[60 * 640 + 110] == 0xF800); // Bottom-right
    
    // Or dump and compare with reference image
    dump_framebuffer(fb, "test_rect.raw");
}
```

## Recommended Workflow for MCU Graphics Testing

### Development Phase
1. **Use Software Framebuffer**:**
   ```c
   #ifdef QEMU_TEST
   static uint16_t test_fb[640*480];  // Test framebuffer
   static uint16_t* active_fb = test_fb;
   #else
   static uint16_t* active_fb = (uint16_t*)0x20000000;  // Hardware
   #endif
   
   void fb_putpixel(int x, int y, uint16_t color) {
       active_fb[y * width + x] = color;
       #ifndef QEMU_TEST
       // Send to actual display hardware
       spi_send_pixel(x, y, color);
       #endif
   }
   ```

2. **Automated Dump Script**: Run after each test
   ```bash
   # Makefile target
   test-graphics:
       make
       ./dump-framebuffer.sh
       python3 compare-with-reference.py screenshot.png
   ```

3. **Visual Regression Testing**: Compare with reference images
   ```python
   from PIL import Image
   import numpy as np
   
   def compare_images(img1_path, img2_path, threshold=0.95):
       img1 = Image.open(img1_path)
       img2 = Image.open(img2_path)
       return np.array_equal(np.array(img1), np.array(img2))
   ```

### Production Build
- Remove test framebuffer
- Direct hardware writes only
- Smaller binary size

## QEMU Machine Selection for MCUs

| MCU Family | QEMU Machine | Display Support |
|------------|--------------|-----------------|
| Cortex-M3 | `mps2-an385` | ❌ None |
| Cortex-M4 | `mps2-an505` | ❌ None |
| STM32F4 | `stm32f4-discovery` | ⚠️ Limited |
| nRF52 | `nrf52` | ❌ None |
| ESP32 | `esp32` | ✅ Good |

**Recommendation**: Use memory dump approach (Method 1) for most MCUs since QEMU display emulation is limited.

## Comparison Table

| Method | Complexity | Accuracy | Speed | MCU Support |
|--------|-----------|----------|-------|-------------|
| **Memory Dump (GDB)** | Medium | ✅ High | Fast | ✅ All |
| **Semihosting File** | Low | ✅ High | Medium | ✅ Most ARM |
| **QEMU Display** | Low | ⚠️ Medium | Fast | ❌ Limited |
| **SPI/I2C Tracing** | High | ✅ High | Slow | ✅ External displays |
| **Unit Tests** | Medium | ⚠️ Partial | Very Fast | ✅ All |

## Best Practice Summary

1. **For Development**: Use memory dump approach (current method) - most reliable
2. **For Continuous Testing**: Add semihosting file output for automation
3. **For External Displays**: Use software framebuffer + memory dump
4. **For Final Verification**: Test on real hardware

Your current `verify-framebuffer.sh` approach is excellent and applies well to MCUs. Consider adding:
- Support for multiple MCU board types
- Automatic test comparison
- CI/CD integration
- Support for non-framebuffer displays (software buffer)

