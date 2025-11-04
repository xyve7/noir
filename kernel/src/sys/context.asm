[bits 64]

section .text
; As per the System V ABI, only a handful of registers need to be saved 
; rbp, rbx, r12, r13, r14, r15 
; And of course the rsp in this case 
global context_save
context_save:
	; Save the address we need to return to 
	mov r9, [rsp + 0]
	; rdi is where the structure that we populate is
	mov [rdi + 0],  r15
	mov [rdi + 8],  r14
	mov [rdi + 16], r13
	mov [rdi + 24], r12
	mov [rdi + 32], rbp
	mov [rdi + 40], rsp
	mov [rdi + 48], rbx
	mov [rdi + 56], r9

	; 0 for success
	xor rax, rax
	ret

global context_restore
context_restore:
	; rdi is where the structure that we populate is
	mov r15, [rdi + 0]
	mov r14, [rdi + 8]
	mov r13, [rdi + 16]
	mov r12, [rdi + 24]
	mov rbp, [rdi + 32]
	mov rsp, [rdi + 40]
	mov rbx, [rdi + 48]
	mov r9,  [rdi + 56]
	; We put 1 into rax so it jumps back to the IP we saved 
	; This time, instead of return 0 like we did in context_restore, we return 1 
	; This is basically like setjmp/longjmp
	mov rsi, 0
	mov rax, 1
	jmp r9
