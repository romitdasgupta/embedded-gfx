# Complete Guide: Getting Visual Graphics Output

## Quick Summary

| Method | Difficulty | Visual Output | Best For |
|--------|-----------|---------------|----------|
| **Real Hardware** | Medium | ✅ Yes | Production/Final testing |
| **Raspberry Pi QEMU** | Medium | ✅ Yes | Development |
| **VNC** | Easy | ⚠️ Maybe | Quick test |
| **Memory Dump** | Easy | ✅ Yes | Debugging |

---

## Method 1: Real Hardware (Recommended for Production)

### What You Need
- ARM VExpress-A9 development board (~$200-400)
- Display with appropriate connector
- SD card or flash storage
- Serial cable for debugging

### Steps
1. Build the kernel:
   ```bash
   make clean && make
   ```

2. Copy `kernel8.img` to boot media

3. Connect display to CLCD interface

4. Power on and see graphics!

### Pros
- Real hardware performance
- Actual display output
- Production-ready testing

### Cons
- Requires hardware purchase
- Setup time

---

## Method 2: Raspberry Pi QEMU (Best for Development)

### Why This Works
QEMU's Raspberry Pi emulation has full graphics support with window output.

### Quick Start

1. **Install additional tools** (if needed):
   ```bash
   sudo apt install qemu-system-aarch64
   ```

2. **See detailed porting guide**: `RASPI_PORT.md`

3. **Key changes needed**:
   - Modify framebuffer code to use RPi mailbox protocol
   - Update linker script (load at 0x8000 instead of 0x80000000)
   - Change QEMU machine to `-M raspi2` or `-M raspi3b`

4. **Run**:
   ```bash
   qemu-system-arm -M raspi2 -kernel kernel8.img -serial stdio
   ```
   
   **A graphical window will appear!** 🎉

### Pros
- Visual output in QEMU window
- No hardware needed
- Fast iteration

### Cons
- Requires code modifications
- Different hardware platform

---

## Method 3: VNC Output (Quick Test)

### Steps

1. **Start QEMU with VNC**:
   ```bash
   ./test-vnc.sh
   ```
   
   Or manually:
   ```bash
   qemu-system-arm -M vexpress-a9 -cpu cortex-a9 -m 128M \
     -kernel kernel8.img -vnc :0 -serial stdio
   ```

2. **Connect VNC viewer**:
   ```bash
   vncviewer localhost:5900
   ```
   
   Or use any VNC client (TigerVNC, RealVNC, etc.)

### Expected Result
⚠️ **May still show blank screen** due to VExpress-A9 CLCD limitations in QEMU, but worth trying.

### Pros
- No code changes
- Quick to test

### Cons
- May not work due to QEMU limitations
- Requires VNC client

---

## Method 4: Memory Dump Screenshot (Best for Debugging)

### Steps

1. **Terminal 1 - Start QEMU with GDB**:
   ```bash
   qemu-system-arm -M vexpress-a9 -cpu cortex-a9 -m 128M \
     -kernel kernel8.img -s -S -nographic
   ```
   
   Flags:
   - `-s`: Start GDB server on port 1234
   - `-S`: Pause at startup

2. **Terminal 2 - Run GDB and dump framebuffer**:
   ```bash
   gdb-multiarch kernel.elf -x utils/screenshot.gdb
   ```
   
   Or manually:
   ```bash
   gdb-multiarch kernel.elf
   (gdb) target remote :1234
   (gdb) continue
   (gdb) # Wait for graphics to be drawn, then Ctrl+C
   (gdb) dump binary memory framebuffer.raw 0x60000000 0x60096000
   (gdb) quit
   ```

3. **Convert to PNG**:
   ```bash
   python3 utils/screenshot.py framebuffer.raw screenshot.png
   ```
   
   Requires: `pip install pillow`

4. **View the image**:
   ```bash
   xdg-open screenshot.png
   ```

### Expected Result
✅ **You will see your rendered graphics!** Red rectangle, green line, and blue circle.

### Pros
- Works with current code (no changes)
- Guaranteed to show framebuffer contents
- Great for debugging

### Cons
- Not real-time
- Requires Python + PIL
- Manual process

---

## Recommended Workflow

### For Learning/Development
1. Start with **Method 4 (Memory Dump)** to verify your code works
2. Port to **Method 2 (Raspberry Pi)** for interactive development
3. Test on **Method 1 (Real Hardware)** before deployment

### For Quick Verification
Use **Method 4** - it's the fastest way to see if your graphics code is working.

### For Production
Use **Method 1** - test on actual target hardware.

---

## Example: Using Memory Dump Method

```bash
# Terminal 1
qemu-system-arm -M vexpress-a9 -cpu cortex-a9 -m 128M \
  -kernel kernel8.img -s -S -nographic &

# Terminal 2
sleep 2  # Let QEMU start
gdb-multiarch kernel.elf << EOF
target remote :1234
continue
# Wait 1 second for graphics
shell sleep 1
dump binary memory framebuffer.raw 0x60000000 0x60096000
quit
EOF

# Convert and view
python3 utils/screenshot.py framebuffer.raw screenshot.png
xdg-open screenshot.png

# Cleanup
killall qemu-system-arm
```

This will show you exactly what's in the framebuffer!

---

## Troubleshooting

### "No graphics in VNC"
- Expected with VExpress-A9 in QEMU
- Try Method 2 (Raspberry Pi) or Method 4 (Memory Dump)

### "GDB can't connect"
- Ensure QEMU is running with `-s -S` flags
- Check port 1234 is not in use: `netstat -an | grep 1234`

### "Python script fails"
- Install PIL: `pip install pillow`
- Check framebuffer.raw exists and is ~614KB

### "Screenshot is all black"
- Code may not have run yet - add delay in GDB
- Check UART output to confirm kernel executed

---

## Next Steps

1. **Try Method 4 now** to see your graphics working
2. **Read RASPI_PORT.md** if you want interactive graphics
3. **Check TROUBLESHOOTING.md** for more details
