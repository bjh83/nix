/*
 * Unfortunately, GCC is severely mentally challenged and requires the return
 * value to be stored in r1 instead of the usual r0 for these functions; so we
 * have to implement the functions in assembly.
 *
 * If you are curious why GCC would do something so extremely stupid and bizare,
 * the reason has to do with ARM's EABI spec. It states that a single division
 * function may be implemented that returns the integer division result in r0
 * and the remainder in r1; however, this is a dumb excuse for GCC because GCC
 * expects functions to be implemented for both operations: division and
 * modulus.
 */
.globl __aeabi_uidivmod
__aeabi_uidivmod:
	@ args = 0, pretend = 0, frame = 8
	@ frame_needed = 1, uses_anonymous_args = 0
	stmfd	sp!, {fp, lr}
	add	fp, sp, #4
	sub	sp, sp, #8
	str	r0, [fp, #-8]
	str	r1, [fp, #-12]
	ldr	r0, [fp, #-8]
	ldr	r1, [fp, #-12]
	bl	__aeabi_uidiv
	mov	r3, r0
	ldr	r2, [fp, #-12]
	mul	r3, r2, r3
	ldr	r2, [fp, #-8]
	rsb	r3, r3, r2
	mov	r1, r3
	sub	sp, fp, #4
	ldmfd	sp!, {fp, pc}

/*
 * Same story as above: GCC is severely brain damaged.
 */
.globl __aeabi_idivmod
__aeabi_idivmod:
	@ args = 0, pretend = 0, frame = 8
	@ frame_needed = 1, uses_anonymous_args = 0
	stmfd	sp!, {fp, lr}
	add	fp, sp, #4
	sub	sp, sp, #8
	str	r0, [fp, #-8]
	str	r1, [fp, #-12]
	ldr	r0, [fp, #-8]
	ldr	r1, [fp, #-12]
	bl	__aeabi_idiv
	mov	r3, r0
	ldr	r2, [fp, #-12]
	mul	r3, r2, r3
	ldr	r2, [fp, #-8]
	rsb	r3, r3, r2
	mov	r1, r3
	sub	sp, fp, #4
	ldmfd	sp!, {fp, pc}
