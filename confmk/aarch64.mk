CC := aarch64-elf-gcc
AS := $(CC) -xassembler-with-cpp -c
LD := aarch64-elf-gcc
AR := aarch64-elf-ar
OBJCOPY := aarch64-elf-objcopy

CPPFLAGS_COMMON += -D__aarch64__
CFLAGS_COMMON += -ffreestanding
LDFLAGS_COMMON +=

ARCH := aarch64
