# TODO: Tests coverage
CPPFLAGS_COMMON += -D__i686__ -DUNITY_INCLUDE_PRINT_FORMATTED -Wno-macro-redefined

TARGET_GCC := gcc

CC := clang
LD := $(CC)
AS := $(CC) -xassembler-with-cpp -c
AR := ar

ARCH := i686
