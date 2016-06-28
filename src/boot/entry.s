@ .data
@ .globl __device_tree_blob
@ __device_tree_blob: .word 0

.text
.globl _start
_start:
  mrc p15, 0, r0, c1, c0, 0 @ Read SCTLR into r0.
  orr r0, r0, #0x0 @ Set bit[0] corresponding to MMU disable.
  mcr p15, 0, r0, c1, c0, 0 @ Write r0 into SCTLR.
  ldr sp, =__stack_bottom
  @ ldr r0, =__device_tree_blob
  @ str r2, [r0]
  bl main
  b .
