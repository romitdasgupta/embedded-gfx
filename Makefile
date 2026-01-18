# Raspberry Pi 3B (AArch64) Embedded Graphics
CROSS_COMPILE = aarch64-elf-
CC = $(CROSS_COMPILE)gcc
LD = $(CROSS_COMPILE)ld
OBJCOPY = $(CROSS_COMPILE)objcopy

CFLAGS = -Wall -O2 -ffreestanding -nostdinc -nostdlib -nostartfiles \
         -mcpu=cortex-a53 -Iinclude

LDFLAGS = -nostdlib -T link.ld

OBJS = kernel/start.o kernel/kernel.o gfx/framebuffer.o gfx/draw.o

all: kernel8.img

kernel8.img: kernel.elf
	$(OBJCOPY) -O binary kernel.elf kernel8.img

kernel.elf: $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.S
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.img *.elf $(OBJS)

run: kernel8.img
	qemu-system-aarch64 -M raspi3b -kernel kernel8.img -serial stdio

run-nographic: kernel8.img
	qemu-system-aarch64 -M raspi3b -kernel kernel8.img -serial stdio -nographic

.PHONY: all clean run run-nographic
