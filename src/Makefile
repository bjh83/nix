CC=      arm-linux-gnueabi-gcc
AS=      arm-linux-gnueabi-as
LD=      arm-linux-gnueabi-ld
OBJCOPY= arm-linux-gnueabi-objcopy
LDFLAGS= -nostdlib -nostartfiles -pie
CFLAGS= -I. -nostdlib -nostartfiles -ffreestanding -fno-common -static -fPIC -march=armv7-a -marm -std=c99

OBJS= boot/init.o boot/entry.o boot/early_printk.o \
			utils/base.o utils/base-asm.o \
			memory/mmu-init.o memory/mmu-init-asm.o \
			memory/mmu.o memory/mmu-asm.o memory/mmap.o

init.bin: $(OBJS)
	$(LD) $(LDFLAGS) -T init.lds $^ -o init.elf
	$(OBJCOPY) init.elf -O binary init.bin

.PHONY: clean

clean:
	rm -rf init.elf init.bin $(OBJS)
