CC = gcc 

CFLAGS = -Wall -Wextra -Werror -O2 -nostdlib -ffreestanding -no-pie -std=c17 -Isrc/include -m64
CFILES = $(shell find src -type f -name '*.c')
COBJS = $(patsubst src/%,%,$(CFILES:.c=.c.o))

ASMFLAGS = -masm=intel 
ASMFILES = $(shell find src -type f -name '*.S')
ASMOBJS = $(patsubst src/%,%,$(ASMFILES:.S=.S.o))

OUT = libc.o 

.PHONY: all clean

bin/$(OUT): obj/$(COBJS) obj/$(ASMOBJS)
	mkdir -p "$$(dirname $@)"
	$(CC) -r obj/$(COBJS) obj/$(ASMOBJS) -o $@
	
obj/%.c.o: src/%.c
	mkdir -p "$$(dirname $@)"
	$(CC) $(CFLAGS) -c $< -o $@

obj/%.S.o: src/%.S
	mkdir -p "$$(dirname $@)"
	$(CC) $(ASMFLAGS) -c $< -o $@

clean:
	rm -rf obj bin
