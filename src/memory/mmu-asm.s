.text
/*
 * Sets ttbr0 (translation table register 0) register.
 *
 * params:
 * r0 - new ttbr0 value.
 */
.globl __set_ttbr0
__set_ttbr0:
  stmfd	sp!, {fp, lr}
  mcr p15, 0, r0, c2, c0, 0 @ Put param, new_ttbr0, in ttbr0.
  ldmfd	sp!, {fp, pc}

/*
 * Sets ttbcr (translation table control register) register.
 *
 * params:
 * r0 - new ttbcr value.
 */
.globl __set_ttbcr
__set_ttbcr:
  stmfd	sp!, {fp, lr}
  mcr p15, 0, r0, c2, c0, 2 @ Put param, new_ttbcr, in ttbcr.
  ldmfd	sp!, {fp, pc}

/*
 * Sets dacr (domain access register) register.
 *
 * params:
 * r0 - new dacr value.
 */
.globl __set_dacr
__set_dacr:
  stmfd	sp!, {fp, lr}
  mcr p15, 0, r0, c3, c0, 0 @ Put param, new_dacr, in dacr.
  ldmfd	sp!, {fp, pc}

/*
 * Uses the MMU to translate a virtual address to a physical one.
 *
 * params:
 * r0 - virtual address to translate.
 */
.globl __translate_addr
__translate_addr:
  stmfd	sp!, {fp, lr}
  mcr p15, 0, r0, c7, c8, 3 @ Request translation of address in r0.
  mrc p15, 0, r0, c7, c4, 0 @ Read value out of PA register into r0.
  ldmfd	sp!, {fp, pc}

.globl __flush_page
__flush_page:
  stmfd	sp!, {fp, lr}
  mcr p15, 0, r0, c8, c7, 1 @ Request invalidation of virtual address in r0.
  ldmfd	sp!, {fp, pc}

/*
 * Turns on MMU and jumps to new address space.
 *
 * params:
 * r0 - virtual address to jump to after turning on the MMU.
 */
.globl __start_mmu
__start_mmu:
  stmfd	sp!, {fp, lr}
  mrc p15, 0, r1, c1, c0, 0 @ Read SCTLR into r0.
  orr r1, r1, #0x1          @ Set bit[0] corresponding to MMU enable.
  mcr p15, 0, r1, c1, c0, 0 @ Write r0 into SCTLR.
  @ mcr p15, 0, r1, c8, c7, 0 @ Invalidate entire TLB.
  @ mcr p15, 0, r1, c7, c5, 0 @ Invalidate entire instruction cache.
  @ mcr p15, 0, r1, c7, c5, 6 @ Invalidate entire branch predictor.
  dsb                       @ Synchronize data.
  isb                       @ Synchronize instructions.
  ldr sp, =__stack_bottom   @ Reset the stack.
  blx r0                    @ Branch to new address space.