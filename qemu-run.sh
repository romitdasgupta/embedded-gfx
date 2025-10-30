#!/bin/bash
qemu-system-arm \
  -M vexpress-a9 \
  -cpu cortex-a9 \
  -m 128 \
  -kernel kernel8.img \
  -display sdl \
  -serial stdio

