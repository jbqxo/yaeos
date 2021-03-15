# TODO: Tests coverage
CPPFLAGS_COMMON += -D__tests__ -DUNITY_INCLUDE_PRINT_FORMATTED

CC := gcc
LD := $(CC)
AS := $(CC) -xassembler-with-cpp -c
AR := ar

CFLAGS_COMMON +=\
	-fsanitize=address \
	-fsanitize=pointer-compare \
	-fsanitize=pointer-subtract \
	-fsanitize=leak \
	-fsanitize=undefined \
	-fsanitize=shift \
	-fsanitize=bounds \
	-fsanitize-address-use-after-scope \
	-fno-omit-frame-pointer

ARCH := tests
