[bits 64]
section .text 

; The only thing we really need to know is that not every register is relavent 
; The calling convention for this is identical to the System V one, except rcx is unused, so r10 replaces it.
; Linux does this also, and from what I understand is that the syscall instruction will store the rip of the returning function
; So yes we have to preserve it, but we also have to ignore it.
; We also can't use r11, since rflags are stored there.
extern syscall_handler
global syscall_common 
syscall_common:
	swapgs
	mov gs:16, rsp
	mov rsp, gs:8

	push rax ; Syscall number
	push rcx ; Where to return
	push rdx ; Argument 3 
	push rsi ; Argument 2
	push rdi ; Argument 1
	push r8  ; Argument 5
	push r9  ; Argument 6
	push r10 ; Argument 4
	push r11 ; rflags
	
	mov rdi, rsp

	call syscall_handler

	pop r11 ; rflags
	pop r10 ; Argument 4
	pop r9  ; Argument 6
	pop r8  ; Argument 5
	pop rdi ; Argument 1
	pop rsi ; Argument 2
	pop rdx ; Argument 3 
	pop rcx ; Where to return
	pop rax ; Syscall number
	
	swapgs 
	mov rsp, gs:16

	o64 sysret

