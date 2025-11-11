#!/bin/sh

make TOOLCHAIN_PREFIX=aarch64-elf- ARCH=aarch64 clean
make TOOLCHAIN_PREFIX=aarch64-elf- ARCH=aarch64 run
