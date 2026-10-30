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

PARSER_SRCS := parser/src/registry.c     \
        parser/src/ui/value.c     \
        parser/src/ui/builtin.c   \
        parser/src/ui/draw.c      \
        parser/src/ui/parse.c

OBJS := $(PARSER_SRCS:parser/src/%.c=build/parse/%.o)

all: build/libpui-parser.a

build/parse/%.o: parser/src/%.c | build
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
