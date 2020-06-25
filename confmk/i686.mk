# This file is not intended to be included directly.

ifeq ($(TARGET), i686)
    CPPFLAGS := -D__i686__
    ifeq ($(BUILD_TEST), 1)
        CC := clang
        LD := gcc
        NASM := nasm
        AR := ar
    else
        LD := i686-elf-gcc
        CRTDIR := $(shell $(LD) -print-search-dirs | grep install\: | sed 's/install\://')
        CC := clang -march=i686 --target=i686-pc-none-elf -B$(CRTDIR)
        NASM := nasm -felf32
        AR := i686-elf-ar
        CFLAGS_COMMON += -m32
    endif
endif
