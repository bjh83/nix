CC=      arm-linux-gnueabi-gcc
AS=      arm-linux-gnueabi-as
LD=      arm-linux-gnueabi-ld
OBJCOPY= arm-linux-gnueabi-objcopy
LDFLAGS= -nostdlib -nostartfiles -pie
CFLAGS= -I. -nostdlib -nostartfiles -ffreestanding -fno-common -static -fPIC -march=armv7-a -marm -std=c99

ENTRY_OBJS= boot/entry-asm.o boot/entry.o

OBJS= boot/kernel-start-asm.o boot/kernel-start.o \
			boot/early_printk.o \
			utils/base.o utils/base-asm.o \
#			memory/mmu.o memory/mmu-asm.o memory/mmap.o

sys.bin: entry.bin kernel.bin
	cat entry.bin > sys.bin
	dd if=/dev/zero bs=1 count=$(shell expr 4096 - $(shell stat -c %s entry.bin)) >> sys.bin
	cat kernel.bin >> sys.bin

entry.bin: $(ENTRY_OBJS)
	$(LD) $(LDFLAGS) -T entry.lds $^ -o entry.elf
	$(OBJCOPY) entry.elf -O binary entry.bin

kernel.bin: $(OBJS)
	$(LD) $(LDFLAGS) -T kernel.lds $^ -o kernel.elf
	$(OBJCOPY) kernel.elf -O binary kernel.bin

.PHONY: clean

clean:
	rm -rf sys.bin kernel.elf kernel.bin entry.elf entry.bin $(OBJS) $(ENTRY_OBJS)
