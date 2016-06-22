@ .data
@ .globl __device_tree_blob
@ __device_tree_blob: .word 0

.text
.globl _start
_start:
  @ ldr sp, =stack_bottom
  @ ldr r0, =__device_tree_blob
  @ str r2, [r0]
  bl main
  b .
