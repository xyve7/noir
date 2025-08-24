[bits 64]
default rel
section .rodata
	message: db "Hello, World!", 10
	message.length: equ $ - message
	console: db "/device/console", 0
section .text 
global _start
_start:
	push rbp 
	mov rbp, rsp
	sub rsp, 8

	; Open the console
	mov rax, 0
	lea rdi, console
	mov rsi, 0
	lea rdx, [rbp - 8]
	syscall 

	; Write
	mov rax, 3
	mov rdi, [rbp - 8]
	lea rsi, message 
	mov rdx, message.length
	syscall 

	; Close /device/console
	mov rax, 1 
	mov rdi, [rbp - 8]
	syscall 

	pop rbp

	; Exit
	mov rax, 4 
	syscall

