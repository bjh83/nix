CC=      arm-linux-gnueabi-gcc
AS=      arm-linux-gnueabi-as
LD=      arm-linux-gnueabi-ld
OBJCOPY= arm-linux-gnueabi-objcopy
LDFLAGS= -nostdlib -nostartfiles -pie
CFLAGS= -I. -nostdlib -nostartfiles -ffreestanding -fno-common -static -fPIC -march=armv7-a -marm -std=c99

init.bin: boot/init.o boot/entry.o boot/early_printk.o utils/base.o utils/base-asm.o memory/mmu.o memory/mmu-asm.o
	$(LD) $(LDFLAGS) -T memmap $^ -o init.elf
	$(OBJCOPY) init.elf -O binary init.bin

.PHONY: clean

clean:
	rm -rf init.elf init.bin boot/init.o boot/entry.o boot/early_printk.o utils/base.o utils/base-asm.o memory/mmu.o memory/mmu-asm.o
