[bits 64]
%include "src/sys/except.asm"
section .data
global isrs 
isrs:
; This macro assigns the first 32 isrs 
%assign i 0
%rep 32
	dq except%+i
%assign i i+1
%endrep
%rep 224
	dq 0
%assign i i+1
%endrep
; Past this are going to the irq andfuck these fuck these  software interrupts
