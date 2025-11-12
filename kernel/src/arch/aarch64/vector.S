// External handlers depending on the kind of exception

#include "except.S"

.extern aarch64_sync_entry

.section .text
.global aarch64_vector_table

// aarch64 Vector Table 
// We skip the first 0x200 bytes, since these are when the exception is handled at EL0 (which is never)
.balign 2048
aarch64_vector_table:
// Alignment at 128 is necessary
.balign 128 
	b .
.balign 128
	b .
.balign 128
	b .
.balign 128
	b .

.balign 128 
	b aarch64_sync_entry
.balign 128
	b . 
.balign 128
	b . 
.balign 128
	b . 
