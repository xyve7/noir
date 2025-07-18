[bits 64]
section .text

global gdt_load_tss
gdt_load_tss:
	mov ax, 0x30 
	ltr ax
	ret
	
global gdt_get_rsp
gdt_get_rsp:
	mov rax, rsp 
	ret

global gdt_reload_seg:
gdt_reload_seg:
	push 0x08
	lea rax, [rel .gdt_reload_cs]
	push rax
	retfq
.gdt_reload_cs:
	mov   ax, 0x10
	mov   ds, ax
	mov   es, ax
	mov   fs, ax
	mov   gs, ax
	mov   ss, ax
	ret
