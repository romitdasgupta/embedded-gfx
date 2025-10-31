#!/bin/bash
qemu-system-arm \
  -M vexpress-a9 \
  -cpu cortex-a9 \
  -m 128M \
  -kernel kernel8.img \
  -serial stdio

