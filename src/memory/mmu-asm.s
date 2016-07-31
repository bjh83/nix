.text

.globl __set_cpsr
__set_cpsr:
  stmfd	sp!, {fp, lr}
  msr cpsr, r0 
  ldmfd	sp!, {fp, pc}

.globl __get_cpsr
__get_cpsr:
  stmfd	sp!, {fp, lr}
  mrs r0, cpsr @ Put r0 into the cpsr (Current Program Status Register).
  ldmfd	sp!, {fp, pc}

.globl __set_scr
__set_scr:
  stmfd	sp!, {fp, lr}
  mcr p15, 0, r0, c1, c1, 0  @ Put r0 into the scr (Secure Configuration Register).
  ldmfd	sp!, {fp, pc}

.globl __get_scr
__get_scr:
  stmfd	sp!, {fp, lr}
  mrc p15, 0, r0, c1, c1, 0  @ Put the scr (Secure Configuration Register) into r0.
  ldmfd	sp!, {fp, pc}

.globl __set_sctlr
__set_sctlr:
  stmfd	sp!, {fp, lr}
  mcr p15, 0, r0, c1, c0, 0 @ Put r0 into sctlr (System Control Register).
  ldmfd	sp!, {fp, pc}

.globl __get_sctlr
__get_sctlr:
  stmfd	sp!, {fp, lr}
  mrc p15, 0, r0, c1, c0, 0 @ Put sctlr (System Control Register) into r0.
  ldmfd	sp!, {fp, pc}

.globl __set_actlr
__set_actlr:
  stmfd	sp!, {fp, lr}
  mcr p15, 0, r0, c1, c0, 1 @ Put r0 into actlr (Auxilary Control Register).
  ldmfd	sp!, {fp, pc}

.globl __get_actlr
__get_actlr:
  stmfd	sp!, {fp, lr}
  mrc p15, 0, r0, c1, c0, 1 @ Put actlr (Auxilary Control Register) into r0.
  ldmfd	sp!, {fp, pc}

.globl __monitor_set_l2_cache_actlr
__monitor_set_l2_cache_actlr:
  stmfd	sp!, {fp, lr}
  mov r12,      #0x100
  orr r12, r12, #0x002 @ Put 0x102 into r12
  mcr p15, 0, r1, c7, c5, #6
  dsb
  isb
  dmb
  smc #1
  ldmfd	sp!, {fp, pc}

.globl __set_l2_cache_actlr
__set_l2_cache_actlr:
  stmfd	sp!, {fp, lr}
  mcr p15, 1, r0, c9, c0, 2 @ Put r0 into L2 Cache Auxilary Control Register.
  ldmfd	sp!, {fp, pc}

.globl __get_l2_cache_actlr
__get_l2_cache_actlr:
  stmfd	sp!, {fp, lr}
  mrc p15, 1, r0, c9, c0, 2 @ Put L2 Cache Auxilary Control Register into r0.
  ldmfd	sp!, {fp, pc}

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
 * Gets value in ttbr0 (translation table register 0) register.
 *
 * returns:
 * value in ttbr0 register.
 */
.globl __get_ttbr0
__get_ttbr0:
  stmfd	sp!, {fp, lr}
  mrc p15, 0, r0, c2, c0, 0 @ Put ttbr0 in r0.
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

.globl __flush_tlb
__flush_tlb:
  stmfd	sp!, {fp, lr}
  mcr p15, 0, r1, c8, c7, 0 @ Invalidate entire TLB.
  ldmfd	sp!, {fp, pc}

.globl __disable_irqs
__disable_irqs:
  stmfd	sp!, {fp, lr}
  mrs r0, cpsr
  orr r0, r0, #0x1C0
  msr cpsr, r0
  ldmfd	sp!, {fp, pc}
