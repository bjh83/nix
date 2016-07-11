@ .data
@ .globl __device_tree_blob
@ __device_tree_blob: .word 0

.text

.globl _start
_start:
  mrc p15, 0, r0, c1, c0, 0 @ Read SCTLR into r0.
  orr r0, r0, #0x0 @ Set bit[0] corresponding to MMU disable.
  mcr p15, 0, r0, c1, c0, 0 @ Write r0 into SCTLR.
  @ ldr sp, =__stack_bottom
  @ ldr r0, =__device_tree_blob
  @ str r2, [r0]
  blx init_mmu
  b .

/*
 * Sets ttbr0 (translation table register 0) register.
 *
 * params:
 * r0 - new ttbr0 value.
 */
.globl __premmu_set_ttbr0
__premmu_set_ttbr0:
  stmfd	sp!, {fp, lr}
  mcr p15, 0, r0, c2, c0, 0 @ Put param, new_ttbr0, in ttbr0.
  ldmfd	sp!, {fp, pc}

/*
 * Sets ttbcr (translation table control register) register.
 *
 * params:
 * r0 - new ttbcr value.
 */
.globl __premmu_set_ttbcr
__premmu_set_ttbcr:
  stmfd	sp!, {fp, lr}
  mcr p15, 0, r0, c2, c0, 2 @ Put param, new_ttbcr, in ttbcr.
  ldmfd	sp!, {fp, pc}

/*
 * Sets dacr (domain access register) register.
 *
 * params:
 * r0 - new dacr value.
 */
.globl __premmu_set_dacr
__premmu_set_dacr:
  stmfd	sp!, {fp, lr}
  mcr p15, 0, r0, c3, c0, 0 @ Put param, new_dacr, in dacr.
  ldmfd	sp!, {fp, pc}
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
  @ ldr r2, =__start_mmu
  @ mov r3, #0x1000
  @ sub r3, r3, #0x1
  @ neg r3, r3
  @ and r2, r2, r3
  @ mcr p15, 0, r2, c8, c7, 1 @ Request invalidation of virtual address in r2.
  @ ldr r2, =__text_start
  @ mcr p15, 0, r2, c8, c7, 1 @ Request invalidation of virtual address in r2.
  @ mcr p15, 0, r1, c8, c7, 0 @ Invalidate entire TLB.
  @ mcr p15, 0, r1, c7, c5, 0 @ Invalidate entire instruction cache.
  @ mcr p15, 0, r1, c7, c5, 6 @ Invalidate entire branch predictor.
  dsb                       @ Synchronize data.
  isb                       @ Synchronize instructions.
  @ ldr sp, =__stack_bottom
  @ mov lr, sp
  blx __kernel_start          @ Branch to new address space.
