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

/*
 * Flushes a page from the TLB.
 *
 * params:
 * r0 - pages address (and ASID) to flush.
 */
.globl __flush_page
__flush_page:
  stmfd	sp!, {fp, lr}
  mcr p15, 0, r0, c8, c7, 1 @ Request invalidation of virtual address in r0.
  ldmfd	sp!, {fp, pc}

