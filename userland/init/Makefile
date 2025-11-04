CC = gcc 

LIBC = ../libc/bin/libc.o

CFLAGS = -Wall -Wextra -Werror -O2 -nostdlib -ffreestanding -fno-stack-protector -no-pie -std=c17 -Isrc/include -I../libc/src/include -m64 -Wl,-e_start
CFILES = $(shell find src -type f -name '*.c')
COBJS = $(patsubst src/%,%,$(CFILES:.c=.c.o))

OUT = init 

.PHONY: all clean

bin/$(OUT): obj/$(COBJS)
	mkdir -p "$$(dirname $@)"
	$(CC) $(CFLAGS) obj/$(COBJS) $(LIBC) -o $@
	
obj/%.c.o: src/%.c
	mkdir -p "$$(dirname $@)"
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf obj bin
