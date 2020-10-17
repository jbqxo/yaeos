CFLAGS_COMMON += -fsanitize=undefined -fno-omit-frame-pointer
LDFLAGS_COMMON += -fsanitize=undefined
CPPFLAGS_COMMON += -D__i686__ -DUNITY_INCLUDE_PRINT_FORMATTED -Wno-macro-redefined
# Silence unity output
CPPFLAGS_COMMON += -DUNITY_OUTPUT_CHAR=""

CC := clang
HOST_CC := $(CC)
LD := clang
AS := gcc -xassembler-with-cpp -c
AR := ar

ARCH := i686
