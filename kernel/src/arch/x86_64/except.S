.intel_syntax noprefix
.section .text 

// This is a beauty in NASM, but this mess in GNU

.macro generate_stub i 
.if (\i == 8 || (\i > 9 && \i < 15) || \i == 17 || \i == 21 || \i == 29 || \i == 30)
	x86_64_isr\i:
		push \i
		jmp x86_64_except_common
.else
	x86_64_isr\i:
		push 0
		push \i
		jmp x86_64_except_common
.endif
.endm 

.altmacro
.set i, 0
.rept 256
    generate_stub %i
    .set i, i + 1
.endr
.noaltmacro

.macro save_state 
	push rax
	push rbx
	push rcx
	push rdx
	push rsi
	push rdi
	push rbp
	push r8
	push r9
	push r10
	push r11
	push r12
	push r13
	push r14
	push r15
.endm

.macro restore_state 
	push r15
	push r14
	push r13
	push r12
	push r11
	push r10
	push r9
	push r8
	push rbp
	push rdi
	push rsi
	push rdx
	push rcx
	push rbx
	push rax
.endm

.extern x86_64_except_handler

x86_64_except_common:
	save_state

	mov rdi, rsp 
	call x86_64_except_handler
	// Skip over the error code and interrupt number
	restore_state 
	add rsp, 16
	iretq
