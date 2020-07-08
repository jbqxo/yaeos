ifeq ($(ROOT),)
    $(error The ROOT variable must be set)
endif

# Build Status
# We need to export it in order to save the status for embedded(?) make calls.

ifeq ($(findstring test,$(MAKECMDGOALS)),)
    export BUILD_TEST ?=
else
    export BUILD_TEST ?= 1
endif

ifeq ($(findstring 1,$(NDEBUG)),1)
    export BUILD_DEBUG ?=
else
    export BUILD_DEBUG ?= 1
endif

TARGET_ARCH           ?= i686
include $(ROOT)/confmk/$(TARGET_ARCH).mk

# Names of the base directories
DIR_ISODIR ?= isodir
DIR_KERNEL ?= kernel
DIR_LIBC   ?= libc
DIR_LIBK   ?= libk
DIR_BOOT   ?= boot
DIR_DEPS   ?= deps

# Name of the build prefixes
BUILDDIR_BUILD  ?= $(ROOT)/build/$(TARGET_ARCH)
BUILDDIR_ISODIR ?= $(BUILDDIR_BUILD)/$(DIR_ISODIR)
BUILDDIR_KERNEL ?= $(BUILDDIR_BUILD)/$(DIR_KERNEL)
BUILDDIR_LIBC   ?= $(BUILDDIR_BUILD)/$(DIR_LIBC)
BUILDDIR_LIBK   ?= $(BUILDDIR_BUILD)/$(DIR_LIBK)
BUILDDIR_BOOT   ?= $(BUILDDIR_BUILD)/$(DIR_BOOT)
BUILDDIR_TESTS  ?= $(BUILDDIR_BUILD)/tests
BUILDDIR_DEPS   ?= $(BUILDDIR_BUILD)/$(DIR_DEPS)

# General build flags

# TODO: Make -mgeneral-regs-only enabled only for the arch-specific code, when
# floating point, sse, mmx, sse2, 3dnow and avx instructions will be available.
CFLAGS_DEBUG     := -O0 -g
CFLAGS_RELEASE   := -O3
CFLAGS_COMMON    := -std=gnu18 -mgeneral-regs-only

CPPFLAGS_DEBUG   :=
CPPFLAGS_RELEASE := -DNDEBUG

LDFLAGS_COMMON  := -lgcc
LDFLAGS_DEBUG   := -fno-lto
LDFLAGS_RELEASE := -flto


# Compilation toolchain
ifeq ($(shell uname -s), Darwin)
    MAKE := gmake --no-print-directory
else
    MAKE := make --no-print-directory
endif

# Compiler diagnostics
# All of these diagnostics there for a reason right?
CFLAGS_COMMON += -Wall
CFLAGS_COMMON += -Wno-unused-command-line-argument

# Basic commands
MKDIRP := mkdir -p
RMRF   := rm -rf
CPRP   := cp -R -p
FIND   := find
COMPILEDB := compiledb
GREP   := grep

# Do not remove intermediate files
.SECONDARY:
