.text

/*
 * Turns on MMU and jumps to new address space.
 *
 * params:
 * r0 - virtual address to jump to after turning on the MMU.
 * r1 - virtual address for new stack.
 */
.globl __start_mmu
__start_mmu:
  stmfd	sp!, {fp, lr}
  mrc p15, 0, r2, c1, c0, 0 @ Read SCTLR into r0.
  orr r2, r2, #0x1          @ Set bit[0] corresponding to MMU enable.
  mcr p15, 0, r2, c1, c0, 0 @ Write r0 into SCTLR.
  ldr r2, =addr_space_jump  @ Load address to current function.
  mov r3, #0x1
  lsl r3, #31               @ Load 0x80000000 into r3.
  sub r2, r2, r3            @ Offset addr_space_jump by 0x80000000.
  blx r2                    @ Jump into new address space.
addr_space_jump:
  ldr r2, =__start_mmu
  mov r3, #0x1000
  sub r3, r3, #0x1
  neg r3, r3
  and r2, r2, r3
  mcr p15, 0, r2, c8, c7, 1 @ Request invalidation of virtual address in r2.
  ldr r2, =__text_start
  mcr p15, 0, r2, c8, c7, 1 @ Request invalidation of virtual address in r2.
  @ mcr p15, 0, r1, c8, c7, 0 @ Invalidate entire TLB.
  @ mcr p15, 0, r1, c7, c5, 0 @ Invalidate entire instruction cache.
  @ mcr p15, 0, r1, c7, c5, 6 @ Invalidate entire branch predictor.
  dsb                       @ Synchronize data.
  isb                       @ Synchronize instructions.
  mov sp, r1                @ Reset stack in new address space.
  mov lr, r1
  blx r0                    @ Branch to new address space.
