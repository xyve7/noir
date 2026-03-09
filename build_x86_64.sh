#!/bin/sh

make TOOLCHAIN_PREFIX=x86_64-elf- ARCH=x86_64 clean
make TOOLCHAIN_PREFIX=x86_64-elf- ARCH=x86_64 run
