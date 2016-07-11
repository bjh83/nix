.text

.globl __kernel_start
__kernel_start:
  @ ldr sp, =__stack_bottom @ Set up stack.
  @ mov lr, sp
  blx kernel_start
  b .                     @ kernel_start should never return.
