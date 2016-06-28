.text
.globl __set_ttbr0
__set_ttbr0:
  @ args = 0, pretend = 0, frame = 8
  @ frame_needed = 1, uses_anonymous_args = 0
  @ link register save eliminated.
  str	fp, [sp, #-4]!
  add	fp, sp, #0
  sub	sp, sp, #12
  str	r0, [fp, #-8] @ Put param, new_ttbr0, in r0.
  mcr p15, 0, r0, c2, c0, 0
  add	sp, fp, #0
  ldmfd	sp!, {fp}
  bx	lr
.globl __set_ttbcr
__set_ttbcr:
  @ args = 0, pretend = 0, frame = 8
  @ frame_needed = 1, uses_anonymous_args = 0
  @ link register save eliminated.
  str	fp, [sp, #-4]!
  add	fp, sp, #0
  sub	sp, sp, #12
  str	r0, [fp, #-8] @ Put param, new_ttbcr, in r0.
  mcr p15, 0, r0, c2, c0, 2
  add	sp, fp, #0
  ldmfd	sp!, {fp}
  bx	lr
.globl __start_mmu
__start_mmu:
  @ We don't care about maintaining the stack, as starting the MMU will bork it
  @ anyway.
  @ TODO(brendan): Reset the stack in post initialization.
  mrc p15, 0, r0, c1, c0, 0 @ Read SCTLR into r0.
  orr r0, r0, #0x1 @ Set bit[0] corresponding to MMU enable.
  mcr p15, 0, r0, c1, c0, 0 @ Write r0 into SCTLR.
  b __post_start_mmu
