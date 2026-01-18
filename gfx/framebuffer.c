/*
 * Raspberry Pi 3B Framebuffer via Mailbox
 * Uses the VideoCore mailbox interface to allocate framebuffer
 */

#include "gfx.h"

// Raspberry Pi 3B peripheral base
#define MMIO_BASE       0x3F000000

// Mailbox registers
#define MBOX_BASE       (MMIO_BASE + 0x0000B880)
#define MBOX_READ       (*(volatile unsigned int*)(MBOX_BASE + 0x00))
#define MBOX_STATUS     (*(volatile unsigned int*)(MBOX_BASE + 0x18))
#define MBOX_WRITE      (*(volatile unsigned int*)(MBOX_BASE + 0x20))

#define MBOX_FULL       0x80000000
#define MBOX_EMPTY      0x40000000
#define MBOX_RESPONSE   0x80000000
#define MBOX_CH_PROP    8  // Property channel

// Property tags for framebuffer setup
#define TAG_SET_PHYS_WH     0x48003
#define TAG_SET_VIRT_WH     0x48004
#define TAG_SET_DEPTH       0x48005
#define TAG_SET_PIXEL_ORDER 0x48006
#define TAG_GET_FRAMEBUFFER 0x40001
#define TAG_GET_PITCH       0x40008
#define TAG_END             0

// Framebuffer state
static volatile unsigned char* fb_ptr;
static unsigned int fb_width;
static unsigned int fb_height;
static unsigned int fb_pitch;
static unsigned int fb_bpp;

// Mailbox message buffer - must be 16-byte aligned
volatile unsigned int __attribute__((aligned(16))) mbox[36];

// Send message to mailbox
static int mbox_call(unsigned char channel) {
    unsigned int r = ((unsigned int)((unsigned long)&mbox) & ~0xF) | (channel & 0xF);
    
    // Wait until we can write
    while (MBOX_STATUS & MBOX_FULL) {
        asm volatile("nop");
    }
    
    // Write address + channel
    MBOX_WRITE = r;
    
    // Wait for response
    while (1) {
        while (MBOX_STATUS & MBOX_EMPTY) {
            asm volatile("nop");
        }
        
        if (MBOX_READ == r) {
            return mbox[1] == MBOX_RESPONSE;
        }
    }
}

void fb_init(int w, int h, int depth, void* base) {
    (void)base;  // Unused - GPU allocates framebuffer
    
    fb_width = w;
    fb_height = h;
    fb_bpp = depth;
    
    // Build the mailbox message
    mbox[0] = 35 * 4;           // Message size
    mbox[1] = 0;                // Request code
    
    // Set physical display size
    mbox[2] = TAG_SET_PHYS_WH;
    mbox[3] = 8;                // Value buffer size
    mbox[4] = 8;                // Request size
    mbox[5] = w;                // Width
    mbox[6] = h;                // Height
    
    // Set virtual display size
    mbox[7] = TAG_SET_VIRT_WH;
    mbox[8] = 8;
    mbox[9] = 8;
    mbox[10] = w;
    mbox[11] = h;
    
    // Set depth (bits per pixel)
    mbox[12] = TAG_SET_DEPTH;
    mbox[13] = 4;
    mbox[14] = 4;
    mbox[15] = depth;
    
    // Set pixel order (0 = BGR, 1 = RGB)
    mbox[16] = TAG_SET_PIXEL_ORDER;
    mbox[17] = 4;
    mbox[18] = 4;
    mbox[19] = 1;               // RGB
    
    // Allocate framebuffer
    mbox[20] = TAG_GET_FRAMEBUFFER;
    mbox[21] = 8;
    mbox[22] = 8;
    mbox[23] = 4096;            // Alignment
    mbox[24] = 0;               // Size (will be filled by GPU)
    
    // Get pitch (bytes per row)
    mbox[25] = TAG_GET_PITCH;
    mbox[26] = 4;
    mbox[27] = 4;
    mbox[28] = 0;               // Pitch (will be filled by GPU)
    
    mbox[29] = TAG_END;
    
    // Make the call
    if (mbox_call(MBOX_CH_PROP) && mbox[20] == TAG_GET_FRAMEBUFFER && mbox[24] != 0) {
        // Convert GPU address to ARM address
        // GPU returns bus address, need to mask off upper bits
        fb_ptr = (volatile unsigned char*)(unsigned long)(mbox[23] & 0x3FFFFFFF);
        fb_pitch = mbox[28];
    } else {
        // Fallback - should not happen on QEMU
        fb_ptr = (volatile unsigned char*)0x3C100000;
        fb_pitch = w * (depth / 8);
    }
}

void fb_clear(unsigned int color) {
    if (fb_bpp == 16) {
        volatile unsigned short* fb16 = (volatile unsigned short*)fb_ptr;
        unsigned short color16 = (unsigned short)color;
        unsigned int pixels_per_row = fb_pitch / 2;
        
        for (unsigned int y = 0; y < fb_height; y++) {
            for (unsigned int x = 0; x < fb_width; x++) {
                fb16[y * pixels_per_row + x] = color16;
            }
        }
    } else if (fb_bpp == 32) {
        volatile unsigned int* fb32 = (volatile unsigned int*)fb_ptr;
        unsigned int pixels_per_row = fb_pitch / 4;
        
        for (unsigned int y = 0; y < fb_height; y++) {
            for (unsigned int x = 0; x < fb_width; x++) {
                fb32[y * pixels_per_row + x] = color;
            }
        }
    }
}

void fb_putpixel(int x, int y, unsigned int color) {
    if (x < 0 || x >= (int)fb_width || y < 0 || y >= (int)fb_height) {
        return;
    }
    
    if (fb_bpp == 16) {
        volatile unsigned short* fb16 = (volatile unsigned short*)fb_ptr;
        unsigned int pixels_per_row = fb_pitch / 2;
        fb16[y * pixels_per_row + x] = (unsigned short)color;
    } else if (fb_bpp == 32) {
        volatile unsigned int* fb32 = (volatile unsigned int*)fb_ptr;
        unsigned int pixels_per_row = fb_pitch / 4;
        fb32[y * pixels_per_row + x] = color;
    }
}
