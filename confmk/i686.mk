# This file is not intended to be included directly.

CPPFLAGS_COMMON += -D__i686__
LD := i686-elf-gcc
CC := clang -march=i686 --target=i686-pc-none-elf
AS := i686-elf-gcc -xassembler-with-cpp -c
AR := i686-elf-ar
CFLAGS_COMMON += -m32

ARCH := i686
