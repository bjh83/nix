.globl _start
_start:
  // ldr sp, =stack_bottom
  bl main
  b .
