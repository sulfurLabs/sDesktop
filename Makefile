#
# SPDX-License-Identifier: GPL-3.0-or-later
#
# Copyright (c) 2026 sulfurLabs
#
# PROJECT: sDesktop
# FILE: Makefile
#

CC   := x86_64-elf-gcc
LD   := x86_64-elf-ld
LIBC ?= include/libc

OS_PATH     ?= ~/osdev/sulfurLabs/sulfurOSgcc/
OS_DSK_PATH ?= $(OS_PATH)dsk/
ROOTFS_PATH ?= $(OS_DSK_PATH)rdh/user_id/applications/
OS_LIBS     ?= $(OS_PATH)user/libs/

CFLAGS := -ffreestanding -nostdlib -fno-builtin -fno-stack-protector         \
          -fno-PIE -fno-pic -m64 -march=x86-64                               \
          -mno-mmx -mno-red-zone -Wall -Wextra -std=gnu11 -D__sulfur__ -O2   \
          -I$(LIBC)/include

LDFLAGS := -nostdlib -static -no-pie -T user.ld

LIBC_DIR := include/libc

OBJS := build/main.o \
        build/log.o \
        build/display/comp.o \
        build/display/fb_sulfur.o \
        build/input/mouse.o \
        build/input/keyboard.o \
        build/window/window.o \
        build/wm/wm.o

all: clean $(LIBC)/build/crt0.o dirs build/sDesktop.elf run

fetchDeps:
	mkdir -p include
	if [ ! -d "$(LIBC_DIR)" ]; then \
		git clone --depth=1 https://github.com/sulfurLabs/sulfurOS.git include/sulfurOS; \
		mv include/sulfurOS/user/libc $(LIBC_DIR); \
		rm -rf include/sulfurOS; \
	fi

dirs:
	mkdir -p build
	mkdir -p build/display
	mkdir -p build/input
	mkdir -p build/window
	mkdir -p build/wm

run:
	@echo "running OS..."
	cd $(OS_PATH) && make run

build/sDesktop.elf: dirs $(OBJS) $(LIBC)/build/crt0.o $(LIBC)/build/libc.a
	@echo "Building sDesktop now..."
	$(LD) $(LDFLAGS) $(LIBC)/build/crt0.o $(OBJS)  \
		$(LIBC)/build/libc.a                       \
		-o $@

	@echo "sDesktop was succesfully built!"

	@echo "building sDesktop resources now..."
	@chmod +x tools/buildimage.sh
	@./tools/buildimage.sh

	@rm -f $(ROOTFS_PATH)/sDesktop.elf
	@cp build/sDesktop.elf $(ROOTFS_PATH)sDesktop.elf
	@cp build/sDesktop.cpio $(OS_DSK_PATH)rdh/user_id/extend/sDesktop.cpio


build/main.o:                     sDesktop/main.c                 ; $(CC) $(CFLAGS) -c $< -o $@
build/log.o:                      sDesktop/log.c                  ; $(CC) $(CFLAGS) -c $< -o $@
build/display/comp.o:             sDesktop/display/comp.c         ; $(CC) $(CFLAGS) -c $< -o $@
build/display/fb_sulfur.o:        sDesktop/display/fb_sulfur.c    ; $(CC) $(CFLAGS) -c $< -o $@
build/input/mouse.o:              sDesktop/input/mouse.c          ; $(CC) $(CFLAGS) -c $< -o $@
build/input/keyboard.o:           sDesktop/input/keyboard.c       ; $(CC) $(CFLAGS) -c $< -o $@
build/window/window.o:             sDesktop/window/window.c        ; $(CC) $(CFLAGS) -c $< -o $@
build/wm/wm.o:                     sDesktop/wm/wm.c                ; $(CC) $(CFLAGS) -c $< -o $@

$(LIBC)/build/crt0.o $(LIBC)/build/libc.a:
	$(MAKE) -C $(LIBC)

clean:
	rm -f build/*.o build/display/*.o build/input/*.o build/window/*.o build/wm/*.o build/sDesktop.elf
	@if [ -d "$(LIBC)" ]; then $(MAKE) -C $(LIBC) clean; fi

.PHONY: all clean install run
