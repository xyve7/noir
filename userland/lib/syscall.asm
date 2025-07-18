[bits 64]
section .text

make_syscall:
	syscall
	ret

global sys_open
global sys_close
global sys_read
global sys_write
global sys_exit

sys_open:
	mov rax, 0
	call make_syscall
	ret

sys_close:
	mov rax, 1
	call make_syscall
	ret

sys_read:
	mov rax, 2
	call make_syscall
	ret

sys_write:
	mov rax, 3
	call make_syscall
	ret

sys_exit:
	mov rax, 4
	call make_syscall
	ret
