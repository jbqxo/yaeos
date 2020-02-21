export ROOT := $(shell pwd)

# Names of the base directories
export DIR_ISODIR ?= isodir
export DIR_KERNEL ?= kernel
export DIR_LIBC   ?= libc
export DIR_LIBK   ?= libk
export DIR_BOOT   ?= boot

# Name of the build prefixes
export PREFIX_BUILD  ?= $(shell pwd)/build
export PREFIX_ISODIR ?= $(PREFIX_BUILD)/$(DIR_ISODIR)
export PREFIX_KERNEL ?= $(PREFIX_BUILD)/$(DIR_KERNEL)
export PREFIX_LIBC   ?= $(PREFIX_BUILD)/$(DIR_LIBC)
export PREFIX_LIBK   ?= $(PREFIX_BUILD)/$(DIR_LIBK)
export PREFIX_BOOT   ?= $(PREFIX_BUILD)/$(DIR_BOOT)

# General build flags
export TARGET           ?= i686
export CFLAGS_DEBUG     := -O0 -g
export CFLAGS_RELEASE   := -O2
export CFLAGS           += -std=gnu18 -m32 \
    $(if $(findstring 1, $(NDEBUG)), $(CFLAGS_RELEASE), $(CFLAGS_DEBUG))
export CPPFLAGS_DEBUG   :=
export CPPFLAGS_RELEASE := -DNDEBUG
export CPPFLAGS         += $(if $(findstring 1, $(NDEBUG)), $(CPPFLAGS_RELEASE), $(CPPFLAGS_DEBUG))

# TODO(Maxim Lyapin): Try to move to the lld linker
export LDFLAGS_DEBUG   :=
export LDFLAGS_RELEASE :=
export LDFLAGS         += -m32 \
    $(if $(findstring 1, $(NDEBUG)), $(LDFLAGS_RELEASE), $(LDFLAGS_DEBUG))

# Compilation toolchain
ifeq ($(shell uname -s), Darwin)
    export MAKE := gmake --no-print-directory
else
    export MAKE := make --no-print-directory
endif
ifeq ($(TARGET), i686)
    export CC := clang --target=i686-pc-none-elf -march=i686
	export AS := nasm -felf32
endif
export AR := $(CONFIG_TARGET)-ar

# Basic commands
export MKDIRP := mkdir -p
export RMRF   := rm -rf
export CPRP   := cp -R -p
export FIND   := find

.PHONY: all kernel libc run-qemu grub-iso
all: kernel libc grub-iso

build_dir:
	@$(MKDIRP) $(PREFIX_BUILD)

clean:
	@$(MAKE) -C $(DIR_KERNEL) clean
	@$(MAKE) -C $(DIR_LIBC) clean
	@$(MAKE) -C $(DIR_BOOT) clean
	@$(RMRF) $(PREFIX_BUILD)
	@$(RMRF) compile_commands.json

compile_commands.json: clean
	@compiledb --full-path -o $@ make all

grub-iso: kernel libc | build_dir
	$(info [general] make grub iso)
	@$(MAKE) -C $(DIR_BOOT) $(PREFIX_BUILD)/grub.iso

kernel: libc | build_dir
	$(info [general] make kernel)
	@$(MAKE) -C $(DIR_KERNEL)

libc: | build_dir
	$(info [general] make libc)
	@$(MAKE) -C $(DIR_LIBC)

run-qemu-debug: grub-iso
	@qemu-system-i386 -s -S -curses -cdrom $(PREFIX_BUILD)/grub.iso

run-qemu: grub-iso
	@qemu-system-i386 -curses -no-reboot -cdrom $(PREFIX_BUILD)/grub.iso
