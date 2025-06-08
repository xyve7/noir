[bits 64]

%include "src/sys/except.asm"
%include "src/sys/irq.asm"

section .data
global isrs

isrs:
; Assign the first 32 ISRs from the exception handlers
%assign i 0
%rep 32
	dq except%+i
	%assign i i+1
%endrep
; Fill the remaining 224 entries with zero
%rep 224
	dq irq%+i
	%assign i i+1
%endrep
; Past this are going to the IRQ
