# This file is not intended to be included directly.

CC := i686-elf-gcc
AS := $(CC) -xassembler-with-cpp -c
LD := i686-elf-gcc
AR := i686-elf-ar
OBJCOPY := i686-elf-objcopy


CPPFLAGS_COMMON += -D__i686__
CFLAGS_COMMON += -m32 -ffreestanding
LDFLAGS_COMMON += 

ARCH := i686
