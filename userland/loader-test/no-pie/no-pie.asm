[bits 64]
section .rodata
	message: db "Hello, World!", 10
	message.length: equ $ - message
section .text 
global _start
_start:
	mov rax, 1 
	mov rdi, 1
	lea rsi, message 
	mov rdx, message.length
	syscall 

	mov rax, 67
	ret
