CPPFLAGS_COMMON += -D__i686__ -DUNITY_INCLUDE_PRINT_FORMATTED -Wno-macro-redefined
# Silence unity output
CPPFLAGS_COMMON += -DUNITY_OUTPUT_CHAR=""

TARGET_GCC := gcc

CC := clang -fsanitize=address -fno-omit-frame-pointer -fno-optimize-sibling-calls -fsanitize-memory-track-origins
LD := $(CC)
AS := $(CC) -xassembler-with-cpp -c
AR := ar

ARCH := i686
