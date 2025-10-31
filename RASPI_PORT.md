# Porting to Raspberry Pi for Better QEMU Graphics

## Why Raspberry Pi?
QEMU's Raspberry Pi 2/3 emulation has much better graphics support and can display output in a window.

## Changes Needed

### 1. Update Makefile

Change the run target:
```makefile
run: kernel8.img
	qemu-system-aarch64 -M raspi3b -kernel kernel8.img -serial stdio -device usb-kbd
```

Or for 32-bit ARM:
```makefile
run: kernel8.img
	qemu-system-arm -M raspi2 -kernel kernel8.img -serial stdio
```

### 2. Update Framebuffer Code

Replace `gfx/framebuffer.c` with Raspberry Pi mailbox interface:

```c
// Raspberry Pi uses mailbox protocol to set up framebuffer
#define MAILBOX_BASE    0x3F00B880  // RPi 2/3
#define MAILBOX_READ    (*(volatile uint32_t*)(MAILBOX_BASE + 0x00))
#define MAILBOX_STATUS  (*(volatile uint32_t*)(MAILBOX_BASE + 0x18))
#define MAILBOX_WRITE   (*(volatile uint32_t*)(MAILBOX_BASE + 0x20))

#define MAILBOX_EMPTY   0x40000000
#define MAILBOX_FULL    0x80000000

// Framebuffer request structure (must be 16-byte aligned)
struct fb_request {
    uint32_t width;
    uint32_t height;
    uint32_t vwidth;
    uint32_t vheight;
    uint32_t pitch;
    uint32_t depth;
    uint32_t x_offset;
    uint32_t y_offset;
    uint32_t pointer;
    uint32_t size;
} __attribute__((aligned(16)));

void mailbox_write(uint32_t channel, uint32_t data) {
    while (MAILBOX_STATUS & MAILBOX_FULL);
    MAILBOX_WRITE = (data & ~0xF) | (channel & 0xF);
}

uint32_t mailbox_read(uint32_t channel) {
    while (1) {
        while (MAILBOX_STATUS & MAILBOX_EMPTY);
        uint32_t data = MAILBOX_READ;
        if ((data & 0xF) == channel) {
            return data & ~0xF;
        }
    }
}

void fb_init(int w, int h, int depth, void* base) {
    struct fb_request req = {
        .width = w,
        .height = h,
        .vwidth = w,
        .vheight = h,
        .pitch = 0,
        .depth = depth,
        .x_offset = 0,
        .y_offset = 0,
        .pointer = 0,
        .size = 0
    };
    
    // Send request via mailbox (channel 1 = framebuffer)
    mailbox_write(1, (uint32_t)&req);
    mailbox_read(1);
    
    // req.pointer now contains framebuffer address
    fb = (uint32_t*)req.pointer;
    width = w;
    height = h;
    bpp = depth;
    pitch = req.pitch;
}
```

### 3. Update Linker Script

Raspberry Pi loads at different address:
```ld
ENTRY(_start)

SECTIONS
{
    . = 0x8000;  /* RPi loads at 0x8000, not 0x80000000 */
    
    .text : {
        *(.text)
    }
    /* ... rest same ... */
}
```

### 4. Update Start Code

```asm
.section .text
.global _start

_start:
    ldr sp, =0x8000  /* Stack grows down from load address */
    
    ldr r0, =__bss_start
    ldr r1, =__bss_end
    mov r2, #0
1:  cmp r0, r1
    strlo r2, [r0], #4
    blo 1b
    
    bl kernel_main
    
hang:
    b hang
```

### 5. Run with Graphics

```bash
make clean && make
qemu-system-arm -M raspi2 -kernel kernel8.img -serial stdio
```

A graphical window should appear showing your rendered graphics!

## Alternative: Use raspi3b (64-bit)

For 64-bit ARM:
- Use `qemu-system-aarch64`
- Change compiler to `aarch64-none-elf-gcc`
- Update assembly for AArch64
- Mailbox base: `0x3F00B880` (same)
