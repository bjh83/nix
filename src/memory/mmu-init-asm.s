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
  @ mcr p15, 0, r1, c8, c7, 0 @ Invalidate entire TLB.
  @ mcr p15, 0, r1, c7, c5, 0 @ Invalidate entire instruction cache.
  @ mcr p15, 0, r1, c7, c5, 6 @ Invalidate entire branch predictor.
  dsb                       @ Synchronize data.
  isb                       @ Synchronize instructions.
  mov sp, r1                @ Reset stack in new address space.
  mov lr, r1
  blx r0                    @ Branch to new address space.
