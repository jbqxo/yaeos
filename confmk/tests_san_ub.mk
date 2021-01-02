CPPFLAGS_COMMON += -D__i686__ -DUNITY_INCLUDE_PRINT_FORMATTED -Wno-macro-redefined
# Silence unity output
CPPFLAGS_COMMON += -DUNITY_OUTPUT_CHAR=""

TARGET_GCC := gcc

CC := clang -fsanitize=undefined -fno-omit-frame-pointer
LD := $(CC)
AS := $(CC) -xassembler-with-cpp -c
AR := ar

ARCH := i686
