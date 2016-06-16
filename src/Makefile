CC=      arm-linux-gnueabi-gcc
AS=      arm-linux-gnueabi-as
LD=      arm-linux-gnueabi-ld
OBJCOPY= arm-linux-gnueabi-objcopy
CFLAGS= -I. -nostdlib -nostartfiles -ffreestanding -std=c99

init.bin: boot/init.o boot/entry.o
	$(LD) -T memmap boot/entry.o boot/init.o -o init.elf
	$(OBJCOPY) init.elf -O binary init.bin

.PHONY: clean

clean:
	rm -rf boot/init.o boot/entry.o init.elf init.bin
