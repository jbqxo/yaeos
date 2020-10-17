# This file is not intended to be included directly.

CPPFLAGS_COMMON += -D__i686__
HOST_CC := clang
CC := clang -march=i686 --target=i686-pc-none-elf
AS := $(CC) -xassembler-with-cpp -c
LD := $(CC)
AR := i686-elf-ar
CFLAGS_COMMON += -m32

ARCH := i686
