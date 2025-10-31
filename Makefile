CROSS_COMPILE = arm-none-eabi-
CC = $(CROSS_COMPILE)gcc
LD = $(CROSS_COMPILE)ld
OBJCOPY = $(CROSS_COMPILE)objcopy

CFLAGS = -Wall -O2 -mfpu=vfpv3 -mfloat-abi=hard -march=armv7-a -mtune=cortex-a9 \
         -nostdlib -ffreestanding \
         -Iinclude

LDFLAGS = -T link.ld

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
	qemu-system-arm -M vexpress-a9 -cpu cortex-a9 -m 128M \
		-kernel kernel8.img -serial stdio

.PHONY: all clean run
