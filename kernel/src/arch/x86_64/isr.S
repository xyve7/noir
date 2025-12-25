.intel_syntax noprefix

#include "except.S"

.section .data 

.global x86_64_isrs
x86_64_isrs:
.altmacro

.macro isr num
    .quad x86_64_isr\num
.endm

.set i, 0
.rept 256
    isr %i
    .set i, i + 1
.endr

.noaltmacro
