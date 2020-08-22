# TODO: Tests coverage
CPPFLAGS += -D__i686__

CC := clang
LD := clang
AS := gcc -xassembler-with-cpp -c
AR := ar

ARCH := i686
