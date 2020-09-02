# TODO: Tests coverage
CPPFLAGS_COMMON += -D__i686__ -DUNITY_INCLUDE_PRINT_FORMATTED -Wno-macro-redefined

CC := clang
LD := clang
AS := gcc -xassembler-with-cpp -c
AR := ar

ARCH := i686
