file kernel/bin-x86_64/kernel
source scripts/gdb/noir-printers.py
break kmain
target remote localhost:1234
set disassembly-flavor intel
continue
