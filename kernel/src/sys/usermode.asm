[bits 64]
section .text 

global usermode_enter
usermode_enter:
	; We have to fake an interrupt return to be able to jump to usermode. 
	; We have to push these values in this order.
    ; ss | 0x3
    ; rsp (usermode stack)
    ; rflags
    ; cs | 0x3
	; rip (where in userspace)

	; ss
	mov rax, 0x20
	or rax, 3 

	; cs
	mov rcx, 0x18 
	or rcx, 3

	; rdi has the usermode stack (rsp)
	; rsi is where return (rip) 

	push rax 
	push rdi 
	pushfq
	push rcx 
	push rsi

	iretq
