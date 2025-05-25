[bits 64]
section .text
; If you don't wanna spend the next 10 years of your life
; googling how these stupid macros work.
; 1. Go to Compiler Explorer (https://godbolt.org).
; 2. Select Assembly (It will be in NASM automatically).
; 3. Test out the macros until you get your desired results.
%assign i 0
%rep 32
; Make sure the excepts that don't push an error code, push a dummy value
%if (i < 10) || ((i > 14) && (i < 17)) || ((i > 17) && (i < 21))
except%+i:
    push 0
    push i
	jmp except_common
%else
except%+i:
    push i
	jmp except_common
%endif
%assign i i+1
%endrep

%macro pushaq 0
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
%endmacro
%macro popaq 0
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
%endmacro

extern except_handler
except_common:
	; Push all of the registers on the stack
	pushaq 
	mov rdi, rsp
	call except_handler
	popaq 
	; The rationale for this is explained everywhere, 
	; but I didn't find any proof of it for a while.
	; However, I found it in Intel SDM 3.7.13
	;	Note that the error code is not popped when these
	;	IRET instruction is executed to return from an exception handler,
	;	so the handler must remove the error code before executing a return.
	; So, adding 16 to the stack pointer moves them out of the way. 
	add rsp, 16
	iretq

