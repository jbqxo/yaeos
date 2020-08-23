# TODO: Tests coverage
CPPFLAGS += -D__i686__ -DUNITY_INCLUDE_PRINT_FORMATTED

CC := clang
LD := clang
AS := gcc -xassembler-with-cpp -c
AR := ar

ARCH := i686
