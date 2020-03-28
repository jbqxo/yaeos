ifeq ($(ROOT),)
    $(error The ROOT variable must be set)
endif

# Build Status
# We need to export it in order to save the status for embedded(?) make calls.

ifeq ($(findstring test,$(MAKECMDGOALS)),)
    export BUILD_TEST ?= 0
else
    export BUILD_TEST ?= 1
endif

ifeq ($(findstring 1,$(NDEBUG)),1)
    export BUILD_DEBUG ?= 0
else
    export BUILD_DEBUG ?= 1
endif

# Names of the base directories
DIR_ISODIR ?= isodir
DIR_KERNEL ?= kernel
DIR_LIBC   ?= libc
DIR_LIBK   ?= libk
DIR_BOOT   ?= boot
DIR_DEPS   ?= deps

# Name of the build prefixes
BUILDDIR_BUILD  ?= $(ROOT)/build
BUILDDIR_ISODIR ?= $(BUILDDIR_BUILD)/$(DIR_ISODIR)
BUILDDIR_KERNEL ?= $(BUILDDIR_BUILD)/$(DIR_KERNEL)
BUILDDIR_LIBC   ?= $(BUILDDIR_BUILD)/$(DIR_LIBC)
BUILDDIR_LIBK   ?= $(BUILDDIR_BUILD)/$(DIR_LIBK)
BUILDDIR_BOOT   ?= $(BUILDDIR_BUILD)/$(DIR_BOOT)
BUILDDIR_TESTS  ?= $(BUILDDIR_BUILD)/tests
BUILDDIR_DEPS   ?= $(BUILDDIR_BUILD)/$(DIR_DEPS)

# General build flags

CFLAGS_DEBUG     := -O0 -g
CFLAGS_RELEASE   := -O2
CFLAGS_COMMON    := -std=gnu18

CPPFLAGS_DEBUG   :=
CPPFLAGS_RELEASE := -DNDEBUG

LDFLAGS_DEBUG   :=
LDFLAGS_RELEASE :=

TARGET           ?= i686
# Compilation toolchain
ifeq ($(shell uname -s), Darwin)
    MAKE := gmake --no-print-directory
else
    MAKE := make --no-print-directory
endif

ifeq ($(TARGET), i686)
    CPPFLAGS := -D__i686__
    ifeq ($(BUILD_TEST), 1)
        CC := gcc
        CXX := g++
        NASM := nasm
        AR := ar
    else
        CC := i686-elf-gcc
        CXX := i686-elf-g++
        NASM := nasm -felf32
        AR := i686-elf-ar
        CFLAGS_COMMON += -m32
    endif
endif

# Compiler diagnostics
# All of these diagnostics there for a reason right?
CFLAGS_COMMON += -Wall

# Basic commands
MKDIRP := mkdir -p
RMRF   := rm -rf
CPRP   := cp -R -p
FIND   := find
