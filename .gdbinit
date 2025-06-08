file kernel/bin-x86_64/kernel
break kmain
target remote localhost:1234
set disassembly-flavor intel
continue
