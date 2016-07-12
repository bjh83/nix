.text

.globl __kernel_start
__kernel_start:
  ldr sp, =__kernel_start @ Set up stack.
  blx kernel_start
  b .                     @ kernel_start should never return.
