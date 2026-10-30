#
# SPDX-License-Identifier: GPL-3.0-or-later
#
# Copyright (c) 2026 sulfurLabs
#
# PROJECT: sDesktop
# FILE: Makefile
#

CC     := x86_64-elf-gcc
AR     := x86_64-elf-ar
RANLIB := x86_64-elf-ranlib
LD     := x86_64-elf-ld

LIBC   ?= ../include/libc

CFLAGS := -ffreestanding -nostdlib -fno-builtin -fno-stack-protector \
          -fno-PIE -fno-pic -m64 -march=x86-64 -mno-sse -mno-sse2    \
          -mno-mmx -mno-red-zone -Wall -Wextra -std=gnu11            \
          -I$(LIBC)/include -Ilibinclude -Isrc

SRCS := src/registry.c     \
        src/ui/value.c     \
        src/ui/builtin.c   \
        src/ui/draw.c      \
        src/ui/parse.c

OBJS := $(SRCS:src/%.c=build/%.o)

all: build/libpui-parser.a

build/%.o: src/%.c | build
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

build/libpui-parser.a: $(OBJS) | build
	$(AR) rc $@ $(OBJS)
	$(RANLIB) $@

build:
	mkdir -p build

clean:
	rm -rf build

.PHONY: all clean
