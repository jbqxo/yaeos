export ROOT := $(shell pwd)

# Names of the base directories
export DIR_ISODIR ?= isodir
export DIR_KERNEL ?= kernel
export DIR_LIBC   ?= libc
export DIR_LIBK   ?= libk
export DIR_BOOT   ?= boot
export DIR_DEPS   ?= deps

# Name of the build prefixes
export PREFIX_BUILD  ?= $(shell pwd)/build
export PREFIX_ISODIR ?= $(PREFIX_BUILD)/$(DIR_ISODIR)
export PREFIX_KERNEL ?= $(PREFIX_BUILD)/$(DIR_KERNEL)
export PREFIX_LIBC   ?= $(PREFIX_BUILD)/$(DIR_LIBC)
export PREFIX_LIBK   ?= $(PREFIX_BUILD)/$(DIR_LIBK)
export PREFIX_BOOT   ?= $(PREFIX_BUILD)/$(DIR_BOOT)
export PREFIX_TESTS  ?= $(PREFIX_BUILD)/tests
export PREFIX_DEPS   ?= $(PREFIX_BUILD)/$(DIR_DEPS)

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
# Default tools
	export CC := gcc
	export AS := nasm
	exprot AR := ar
	export CPPFLAGS := -D__i686__

	ifneq ($(MAKECMDGOALS),test)
		export CC := i686-elf-gcc
		export AS := nasm -felf32
		export AR := i686-elf-ar
	endif
endif

# Thanks to Apple for deprecating i386.
# If we are running tests right now, do not compile for i386.
ifeq ($(MAKECMDGOALS),test)
	export CFLAGS := $(filter-out -m32,$(CFLAGS))
	export LDFLAGS := $(filter-out -m32,$(LDFLAGS))
endif

# Compiler diagnostics
# All of these diagnostics there for a reason right?
export CFLAGS += -Wall

# Basic commands
export MKDIRP := mkdir -p
export RMRF   := rm -rf
export CPRP   := cp -R -p
export FIND   := find

.PHONY: all kernel libc run-qemu grub-iso deps test
all: kernel libc grub-iso

build_dir:
	@$(MKDIRP) $(PREFIX_BUILD)

clean:
	@$(MAKE) -C $(DIR_KERNEL) clean
	@$(MAKE) -C $(DIR_LIBC) clean
	@$(MAKE) -C $(DIR_BOOT) clean
	@$(MAKE) -C $(DIR_DEPS) clean
	@$(RMRF) $(PREFIX_BUILD)

compile_commands.json: clean
	@compiledb --command-style -o $@ make all

grub-iso: kernel libc | build_dir
	$(info [general] make grub iso)
	@$(MAKE) -C $(DIR_BOOT) $(PREFIX_BUILD)/grub.iso

kernel: libc | build_dir
	$(info [general] make kernel)
	@$(MAKE) -C $(DIR_KERNEL)

libc: | build_dir
	$(info [general] make libc)
	@$(MAKE) -C $(DIR_LIBC)

deps: | build_dir
	$(info [general] make deps)
	@$(MAKE) -C $(DIR_DEPS)

test: CPPFLAGS += -I$(ROOT)/$(DIR_DEPS)/unity/src
test: deps | build_dir
	$(info [general] make test)
	@$(MAKE) -C $(DIR_LIBC) test
	@$(MAKE) -C $(DIR_KERNEL) test

run-qemu-debug: grub-iso
	@qemu-system-i386 -s -S -cdrom $(PREFIX_BUILD)/grub.iso

run-qemu: grub-iso
	@qemu-system-i386 -no-reboot -cdrom $(PREFIX_BUILD)/grub.iso
