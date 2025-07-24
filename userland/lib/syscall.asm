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
global sys_version

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

sys_version:
	mov rax, 5
	call make_syscall
	ret
