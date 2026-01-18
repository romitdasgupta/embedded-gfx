#!/usr/bin/env python3
"""Convert raw framebuffer dump to PNG image"""

import struct
from PIL import Image

def rgb565_to_rgb888(pixel):
    """Convert 16-bit RGB565 to 24-bit RGB888"""
    r = ((pixel >> 11) & 0x1F) << 3
    g = ((pixel >> 5) & 0x3F) << 2
    b = (pixel & 0x1F) << 3
    return (r, g, b)

def convert_framebuffer(input_file, output_file, width=640, height=480):
    """Convert raw RGB565 framebuffer to PNG"""
    
    with open(input_file, 'rb') as f:
        data = f.read()
    
    # Create image
    img = Image.new('RGB', (width, height))
    pixels = img.load()
    
    # Read 16-bit pixels
    for y in range(height):
        for x in range(width):
            offset = (y * width + x) * 2
            if offset + 1 < len(data):
                # Little-endian 16-bit value
                pixel = struct.unpack('<H', data[offset:offset+2])[0]
                pixels[x, y] = rgb565_to_rgb888(pixel)
    
    img.save(output_file)
    print(f"Saved screenshot to {output_file}")

if __name__ == '__main__':
    import sys
    
    input_file = sys.argv[1] if len(sys.argv) > 1 else 'framebuffer.raw'
    output_file = sys.argv[2] if len(sys.argv) > 2 else 'screenshot.png'
    width = int(sys.argv[3]) if len(sys.argv) > 3 else 640
    height = int(sys.argv[4]) if len(sys.argv) > 4 else 480
    
    convert_framebuffer(input_file, output_file, width, height)
