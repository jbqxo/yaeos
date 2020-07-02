# This file is not intended to be included directly.

ifeq ($(TARGET), i686)
    CPPFLAGS := -D__i686__
    ifeq ($(BUILD_TEST), 1)
        CC := clang
        LD := gcc
        AS := gcc -xassembler-with-cpp -c
        AR := ar
    else
        LD := i686-elf-gcc
        CRTDIR := $(shell $(LD) -print-search-dirs | grep install\: | sed 's/install\://')
        CC := clang -march=i686 --target=i686-pc-none-elf -B$(CRTDIR)
        AS := i686-elf-gcc -xassembler-with-cpp -c
        AR := i686-elf-ar
        CFLAGS_COMMON += -m32
    endif
endif