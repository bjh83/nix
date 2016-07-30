# Nix
Nix is a toy operating system that serves only to teach me and some of my
friends about operating systems.

Nix targets the AM3358 Sitara implementation of the Cortex-A8 processor
currently and is too early in development to really start thinking about other
platforms. Nevertheless, code should be written in such a way that porting to
other platforms is *feasible*.

## Evironment Setup
TODO(diego): discuss how to set up build environment.

## Building and Installing
From within the `src` directory:

```
$ make
```

This builds several things, but most importantly `sys.bin` which has the kernel
image in it.

You will need an empty, fat32 formatted micro SD card with the following files
copied to it:

 - http://downloads.angstrom-distribution.org/demo/beagleboard/MLO
 - http://downloads.angstrom-distribution.org/demo/beagleboard/u-boot.img
 - http://downloads.angstrom-distribution.org/demo/beagleboard/uImage-beagleboard.bin

Now copy `uEnv.txt` and `sys.bin` to the SD card.

And that's it!

## Boot Image Format
The boot image `sys.bin` can be broken into two parts: the entry section, and
the kernel section. The raw entry section image is contained in `entry.bin` and
the kernel section is contained in `kernel.bin`. Each section is compiled as
free standing (no standard library linked in), statically compiled ELF file
which is then used to generate a binary image (it is copied exactly in that
format to memory when loaded). The binary files are then copied to the same
single file with an appropriate amount of padding in between.

### Entry Section
The entry section is compiled as a separate binary from the rest of the kernel
because it is responsible for turning on the MMU. Thus, it starts off in
physical addressing mode and ends in virtual addressing mode; this corrupts any
pointers in existence prior to the MMU being turned on. The entry section
overcomes the addressing change issue by simply not sharing any linkage with the
rest of the kernel. This is most easily achieved by compiling it as a completely
separate binary. The entry section's behavior is further described in the
**Boot Sequence** section below.

### Kernel Section
This is where all the magic happens, or will happen once it does something
interesting.

## Boot Sequence
Execution begins in `_start` in `boot/entry-asm.s`.
Currently, all it does is ensures that the MMU is disabled and jumps into
`init_mmu` in `boot/entry.c`.

`init_mmu`:
 1. Sets up the memory translation table
 2. Creates a flat mapping for the code that turns on the MMU
 3. Sets up the `ttbr0`, `ttbcr`, and `dacr` registers
 4. Jumps to `__start_mmu` in `boot/entry-asm.s`

`__start_mmu`:
 1. Turns on the MMU.
 2. Jumps to itself in the new virtual address space.
 3. **NOT IMPLEMENTED:** Invalidates the TLB.
 4. Jumps to the kernel, `__kernel_start` in `boot/kernel-start-asm.s`.

`__kernel_start`:
 1. Sets up the stack.
 2. Jumps to `kernel_start` in `boot/kernel-start.c`

`kernel_start`:
 1. Prints `Starting kernel ...`
 2. Clears the BSS.
 3. Finishes MMU initialization.

