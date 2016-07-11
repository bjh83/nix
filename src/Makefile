CC=      arm-linux-gnueabi-gcc
AS=      arm-linux-gnueabi-as
LD=      arm-linux-gnueabi-ld
OBJCOPY= arm-linux-gnueabi-objcopy
LDFLAGS= -nostdlib -nostartfiles -pie
CFLAGS= -I. -nostdlib -nostartfiles -ffreestanding -fno-common -static -fPIC -march=armv7-a -marm -std=c99

INIT_OBJS= boot/entry.o boot/init.o

OBJS= boot/kernel-start.o \
			boot/early_printk.o \
			utils/base.o utils/base-asm.o \
#			memory/mmu.o memory/mmu-asm.o memory/mmap.o

sys.bin: init.bin kernel.bin
	cat init.bin > sys.bin
	dd if=/dev/zero bs=1 count=$(shell expr 4096 - $(shell stat -c %s init.bin)) >> sys.bin
	cat kernel.bin >> sys.bin

init.bin: $(INIT_OBJS)
	$(LD) $(LDFLAGS) -T init.lds $^ -o init.elf
	$(OBJCOPY) init.elf -O binary init.bin

kernel.bin: $(OBJS)
	$(LD) $(LDFLAGS) -T kernel.lds $^ -o kernel.elf
	$(OBJCOPY) kernel.elf -O binary kernel.bin

.PHONY: clean

clean:
	rm -rf sys.bin kernel.elf kernel.bin init.elf init.bin $(OBJS) $(INIT_OBJS)
