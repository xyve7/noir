.section .text 

// Yes, I am saving ALL of the registers 
// Do I need to? No.
.macro save_state
	stp x0,  x1,  [sp, #0]
	stp x2,  x3,  [sp, #16]
	stp x4,  x5,  [sp, #32]
	stp x6,  x7,  [sp, #48]
	stp x8,  x9,  [sp, #64]
	stp x10, x11, [sp, #80]
	stp x12, x13, [sp, #96]
	stp x14, x15, [sp, #112]
	stp x16, x17, [sp, #128]
	stp x18, x19, [sp, #144]
	stp x20, x21, [sp, #160]
	stp x22, x23, [sp, #176]
	stp x24, x25, [sp, #192]
	stp x26, x27, [sp, #208]
	stp x28, x29, [sp, #224]
	str x30,      [sp, #240]  
.endm

.macro restore_state
	ldp x0,  x1,  [sp, #0]
	ldp x2,  x3,  [sp, #16]
	ldp x4,  x5,  [sp, #32]
	ldp x6,  x7,  [sp, #48]
	ldp x8,  x9,  [sp, #64]
	ldp x10, x11, [sp, #80]
	ldp x12, x13, [sp, #96]
	ldp x14, x15, [sp, #112]
	ldp x16, x17, [sp, #128]
	ldp x18, x19, [sp, #144]
	ldp x20, x21, [sp, #160]
	ldp x22, x23, [sp, #176]
	ldp x24, x25, [sp, #192]
	ldp x26, x27, [sp, #208]
	ldp x28, x29, [sp, #224]
	ldr x30,      [sp, #240]  
.endm

.global aarch64_sync_handler

// All of the different kinds of exceptions 
aarch64_sync_entry:
	sub sp, sp, #280
	save_state

	// Push the state onto the stack
	mrs x0, SPSR_EL1
	mrs x1, ELR_EL1
	mrs x2, ESR_EL1
	mrs x3, FAR_EL1

	stp x0, x1, [sp, #248]
	stp x2, x3, [sp, #264]

	mov x0, sp
	
	adrp x16, aarch64_sync_handler
	add x16, x16, :lo12:aarch64_sync_handler
	blr x16	
	
	restore_state
	add sp, sp, #280 
	eret
	
